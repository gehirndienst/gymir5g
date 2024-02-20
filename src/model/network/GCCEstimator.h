/*
 * GCCEstimator.h
 *
 * a Google Congestion Control (GCC) estimator of the bandwidth.
 * The code is based on: https://github.com/thegreatwb/HRCC/blob/main/BandwidthEstimator_gcc.py
 *
 *  Created on: Oct 9, 2023
 *      Author: HRCC paper authors, Nikita Smirnov,
 */

#include <algorithm>
#include <cmath>
#include <deque>
#include <iostream>
#include <numeric>
#include <vector>

constexpr int kMinNumDeltas = 60;
constexpr double thresholdGain = 4.0;
constexpr double kBurstIntervalMs = 5.0;
constexpr int kTrendlineWindowSize = 20;
constexpr double kTrendlineSmoothingCoeff = 0.9;
constexpr int kOverUsingTimeThreshold = 10;
constexpr double kMaxAdaptOffsetMs = 15.0;
constexpr double eta = 1.08;
constexpr double alpha = 0.85;
constexpr double kUp = 0.0087;
constexpr double kDown = 0.039;
constexpr double maxTimeDeltaMs = 200;

struct RtpPacket {
    static const int headerSize = 12;

    int sequenceNumber;
    double sendTimestamp;
    double receiveTimestamp;
    int payloadSize;
    int bandwidthPrediction;
};

class PacketGroup {
public:
    std::vector<RtpPacket> pkts;
    std::vector<int> arrivalTimeList;
    std::vector<int> sendTimeList;
    int pktGroupSize;
    int pktInGroup;
    int completeTime;
    int transferDuration;

    PacketGroup(const std::vector<RtpPacket>& packets)
        : pkts(packets)
        , pktGroupSize(0)
        , pktInGroup(0)
        , completeTime(0)
        , transferDuration(0) {

        for (const auto& pkt : packets) {
            sendTimeList.push_back(pkt.sendTimestamp);
            arrivalTimeList.push_back(pkt.receiveTimestamp);
            pktGroupSize += pkt.payloadSize;
        }
        pktInGroup = packets.size();
        completeTime = arrivalTimeList.back();
        transferDuration = arrivalTimeList.back() - arrivalTimeList.front();
    }
};

class DeltaGroup {
public:
    std::vector<double> sendTimeDeltaList;
    std::vector<double> delayGradientList;

    ~DeltaGroup() {
        sendTimeDeltaList.clear();
        delayGradientList.clear();
    }

    void addDelta(double sendTimeDelta, double delayGradient) {
        sendTimeDeltaList.push_back(sendTimeDelta);
        delayGradientList.push_back(delayGradient);
    }
};

class GCCEstimator {
public:
    std::vector<RtpPacket> packetsList;
    std::vector<PacketGroup> packetGroup;
    double firstGroupCompleteTime;

    double accDelay;
    double smooothedDelay;
    std::deque<double> accDelayList;
    std::deque<double> smoothedDelayList;

    std::string state;
    int lastBandwidthEstimation;
    int avgMaxBitrateKbps;
    double varMaxBitrateKbps;
    std::string rateControlRegion;
    int timeLastBitrateChange;

    double gamma1;
    int numOfDeltas;
    int timeOverUsing;
    double prevTrend;
    int overuseCounter;
    std::string overuseFlag;

    double lastUpdateMs;
    double lastUpdateThresholdMs;
    double nowMs;

    bool isResetPacketsList;

    int minBandwidth;
    int maxBandwidth;

    GCCEstimator(int minBandwidth = 400000, int maxBandwidth = 10000000)
        : minBandwidth(minBandwidth)
        , maxBandwidth(maxBandwidth) {
        reset();
    }

    void reset() {
        packetsList.clear();
        packetGroup.clear();
        firstGroupCompleteTime = -1.0;

        accDelay = 0;
        smooothedDelay = 0;
        accDelayList.clear();
        smoothedDelayList.clear();

        state = "Hold";
        lastBandwidthEstimation = 400000;
        avgMaxBitrateKbps = -1;
        varMaxBitrateKbps = -1;
        rateControlRegion = "kRcMaxUnknown";
        timeLastBitrateChange = -1;

        gamma1 = 12.5;
        numOfDeltas = 0;
        timeOverUsing = -1;
        prevTrend = 0.0;
        overuseCounter = 0;
        overuseFlag = "NORMAL";
        lastUpdateMs = -1.0;
        lastUpdateThresholdMs = -1.0;
        nowMs = -1.0;

        isResetPacketsList = false;
    }

    void updateWithRtpPacket(int sequenceNumber, double sendTimestamp, double receiveTimestamp, int payloadSize) {
        // first main method, call for each received RTP packet
        RtpPacket pkt;
        pkt.sequenceNumber = sequenceNumber;
        pkt.sendTimestamp = sendTimestamp;
        pkt.receiveTimestamp = receiveTimestamp;
        pkt.payloadSize = payloadSize;
        pkt.bandwidthPrediction = lastBandwidthEstimation;
        packetsList.push_back(pkt);
        nowMs = pkt.receiveTimestamp;
    }

    int estimateBandwidth() {
        // second main method, call once within an estimation period
        int estimatedBandwidthByDelay = getEstimatedBandwidthByDelay();
        int estimatedBandwidthByLoss = getEstimatedBandwidthByLoss();
        int estimatedBandwidth = std::min(estimatedBandwidthByDelay, estimatedBandwidthByLoss);
        if (minBandwidth != 0 || maxBandwidth != 0) {
            estimatedBandwidth = std::clamp(estimatedBandwidth, minBandwidth, maxBandwidth);
        }
        if (isResetPacketsList) {
            packetsList.clear();
            isResetPacketsList = false;
        }
        lastBandwidthEstimation = estimatedBandwidth;
        return lastBandwidthEstimation;
    }

    // for external optimizers or predictors
    void setEstimatedBandwidth(int bandwidthPrediction) {
        lastBandwidthEstimation = bandwidthPrediction;
    }

    int getEstimatedBandwidthByDelay() {
        if (packetsList.size() == 0) {
            return lastBandwidthEstimation;
        }

        // divide packets into groups
        std::vector<PacketGroup> pktGroupList = dividePacketGroup();
        if (pktGroupList.size() < 2) {
            return lastBandwidthEstimation;
        }

        // calculate the packets deltas
        DeltaGroup deltaGroup = computeDeltasForPacketGroup(pktGroupList);

        // calculate the trendline
        double trendline = trendlineFilter(deltaGroup.delayGradientList, pktGroupList);
        if (trendline == -1.0) {
            return lastBandwidthEstimation;
        }

        // report the current network status
        double sumSendTimeDeltas = std::accumulate(deltaGroup.sendTimeDeltaList.begin(), deltaGroup.sendTimeDeltaList.end(),
                                   0.0);
        overuseDetector(trendline, sumSendTimeDeltas);

        // determine direction to bandwidth adjustment and set the current state
        updateState();

        // adjust estimated bandwidth
        int bandwidth = rateAdaptationByDelay();

        // reset packets if we come here
        isResetPacketsList = true;

        return bandwidth;
    }

    int getEstimatedBandwidthByLoss() {
        double lossRate = calculateLossRate();
        if (lossRate == -1.0) {
            return lastBandwidthEstimation;
        }

        int bandwidth = rateAdaptationByLoss(lossRate);
        return bandwidth;
    }

    double calculateLossRate() {
        bool first = false;
        int validPkts = 0;
        int minSequenceNumber = 0, maxSequenceNumber = 0;
        if (packetsList.size() == 0) {
            return -1.0;
        }
        for (size_t i = 0; i < packetsList.size(); ++i) {
            if (!first) {
                minSequenceNumber = packetsList[i].sequenceNumber;
                maxSequenceNumber = packetsList[i].sequenceNumber;
                first = true;
            }
            validPkts++;
            minSequenceNumber = std::min(minSequenceNumber, packetsList[i].sequenceNumber);
            maxSequenceNumber = std::max(maxSequenceNumber, packetsList[i].sequenceNumber);
        }
        if ((maxSequenceNumber - minSequenceNumber) == 0) {
            return -1.0;
        }
        double rxRate = static_cast<double>(validPkts) / (maxSequenceNumber - minSequenceNumber);
        double lossRate = 1 - rxRate;
        return lossRate;
    }

    int rateAdaptationByLoss(double lossRate) {
        int bandwidth = lastBandwidthEstimation;
        if (lossRate > 0.1) {
            bandwidth = static_cast<int>(lastBandwidthEstimation * (1 - 0.5 * lossRate));
        } else if (lossRate < 0.02) {
            bandwidth = static_cast<int>(1.05 * lastBandwidthEstimation);
        }
        return bandwidth;
    }

    std::vector<PacketGroup> dividePacketGroup() {
        std::vector<PacketGroup> pktGroupList;
        std::vector<RtpPacket> packets;
        double firstSendtimeInGroup = packetsList[0].sendTimestamp;
        packets.push_back(packetsList[0]);

        for (size_t i = 1; i < packetsList.size(); ++i) {
            if (packetsList[i].sendTimestamp - firstSendtimeInGroup <= kBurstIntervalMs) {
                packets.push_back(packetsList[i]);
            } else {
                pktGroupList.push_back(PacketGroup(packets));
                if (firstGroupCompleteTime == -1.0) {
                    firstGroupCompleteTime = packets.back().receiveTimestamp;
                }
                firstSendtimeInGroup = packetsList[i].sendTimestamp;
                packets.clear();
                packets.push_back(packetsList[i]);
            }
        }
        return pktGroupList;
    }

    DeltaGroup computeDeltasForPacketGroup(const std::vector<PacketGroup>& pktGroupList) {
        DeltaGroup deltaGroup;
        for (size_t idx = 1; idx < pktGroupList.size(); ++idx) {
            double sendTimeDelta = pktGroupList[idx].sendTimeList.back() - pktGroupList[idx - 1].sendTimeList.back();
            double arrivalTimeDelta =
                pktGroupList[idx].arrivalTimeList.back() - pktGroupList[idx - 1].arrivalTimeList.back();
            //double groupsizeDelta = pktGroupList[idx].pktGroupSize - pktGroupList[idx - 1].pktGroupSize;
            double delay = arrivalTimeDelta - sendTimeDelta;
            numOfDeltas++;
            deltaGroup.addDelta(sendTimeDelta, delay);
        }
        return deltaGroup;
    }

    double trendlineFilter(const std::vector<double>& delayGradientList,
                           const std::vector<PacketGroup>& pktGroupList) {
        for (size_t i = 0; i < delayGradientList.size(); ++i) {
            double estimatedAcumulatedDelay = accDelay + delayGradientList[i];
            double estimatedSmooothedDelay = kTrendlineSmoothingCoeff * smooothedDelay +
                                             (1 - kTrendlineSmoothingCoeff) * estimatedAcumulatedDelay;

            accDelay = estimatedAcumulatedDelay;
            smooothedDelay = estimatedSmooothedDelay;

            double arrivalTime = pktGroupList[i + 1].completeTime;
            accDelayList.push_back(arrivalTime - firstGroupCompleteTime);

            smoothedDelayList.push_back(smooothedDelay);
            if (accDelayList.size() > kTrendlineWindowSize) {
                accDelayList.pop_front();
                smoothedDelayList.pop_front();
            }
        }
        if (accDelayList.size() == kTrendlineWindowSize) {
            double avgAccDelay = 0.0;
            double avgSmoothedDelay = 0.0;
            for (int i = 0; i < kTrendlineWindowSize; ++i) {
                avgAccDelay += accDelayList[i];
                avgSmoothedDelay += smoothedDelayList[i];
            }
            avgAccDelay /= kTrendlineWindowSize;
            avgSmoothedDelay /= kTrendlineWindowSize;

            double numerator = 0.0;
            double denominator = 0.0;
            for (int i = 0; i < kTrendlineWindowSize; ++i) {
                numerator += (accDelayList[i] - avgAccDelay) * (smoothedDelayList[i] - avgSmoothedDelay);
                denominator += (accDelayList[i] - avgAccDelay) * (accDelayList[i] - avgAccDelay);
            }

            double trendline = numerator / (denominator + 1e-05);
            return trendline;
        } else {
            accDelayList.clear();
            smoothedDelayList.clear();
            accDelay = 0.0;
            smooothedDelay = 0.0;
            return -1.0;
        }
    }

    void overuseDetector(double trendline, int tsDelta) {
        if (numOfDeltas < 2) {
            return;
        }

        double modifiedTrend = trendline * std::min(numOfDeltas, kMinNumDeltas) * thresholdGain;

        if (modifiedTrend > gamma1) {
            if (timeOverUsing == -1) {
                timeOverUsing = tsDelta / 2;
            } else {
                timeOverUsing += tsDelta;
            }
            overuseCounter++;
            if (timeOverUsing > kOverUsingTimeThreshold && overuseCounter > 1) {
                if (trendline > prevTrend) {
                    timeOverUsing = 0;
                    overuseCounter = 0;
                    overuseFlag = "OVERUSE";
                }
            }
        } else if (modifiedTrend < -gamma1) {
            timeOverUsing = -1;
            overuseCounter = 0;
            overuseFlag = "UNDERUSE";
        } else {
            timeOverUsing = -1;
            overuseCounter = 0;
            overuseFlag = "NORMAL";
        }

        prevTrend = trendline;
        updateThreshold(modifiedTrend);
    }

    void updateThreshold(double modifiedTrend) {
        if (lastUpdateThresholdMs == -1.0) {
            lastUpdateThresholdMs = nowMs;
        }
        if (std::abs(modifiedTrend) > gamma1 + kMaxAdaptOffsetMs) {
            lastUpdateThresholdMs = nowMs;
            return;
        }
        double k = 0.0;
        if (std::abs(modifiedTrend) < gamma1) {
            k = kDown;
        } else {
            k = kUp;
        }
        double kMaxTimeDeltaMs = 100.0;
        double timeDeltaMs = std::min(nowMs - lastUpdateThresholdMs, kMaxTimeDeltaMs);
        gamma1 += k * (std::abs(modifiedTrend) - gamma1) * timeDeltaMs;
        if (gamma1 < 6.0) {
            gamma1 = 6.0;
        } else if (gamma1 > 600.0) {
            gamma1 = 600.0;
        }
        lastUpdateThresholdMs = nowMs;
    }

    void updateState() {
        if (overuseFlag == "NORMAL") {
            if (state == "Hold") {
                state = "Increase";
            }
        } else if (overuseFlag == "OVERUSE") {
            if (state != "Decrease") {
                state = "Decrease";
            }
        } else if (overuseFlag == "UNDERUSE") {
            state = "Hold";
        }
    }

    int rateAdaptationByDelay() {
        int bytesReceived = 0;
        int estimatedThroughputBps = 0;
        for (const RtpPacket& pkt : packetsList) {
            bytesReceived += pkt.payloadSize + pkt.headerSize;
        }
        if (packetsList.size() != 0) {
            double time = std::max(nowMs - packetsList[0].receiveTimestamp, maxTimeDeltaMs);
            estimatedThroughputBps = 1000 * 8 * bytesReceived / time;
        }
        double estimatedThroughputKbps = estimatedThroughputBps / 1000;

        double throughputBasedLimit = 3 * estimatedThroughputBps + 10;

        // calculate the standard deviation of the maximum throughput
        updateMaxThroughputEstimate(estimatedThroughputKbps);
        double stdMaxBitrate = pow(varMaxBitrateKbps * avgMaxBitrateKbps, 0.5);

        if (state == "Increase") {
            if (avgMaxBitrateKbps >= 0 && estimatedThroughputKbps > avgMaxBitrateKbps + 3 * stdMaxBitrate) {
                avgMaxBitrateKbps = -1;
                rateControlRegion = "kRcMaxUnknown";
            }

            if (rateControlRegion == "kRcNearMax") {
                // already close to maximum, additivity increase
                double additiveIncreaseBps = additiveRateIncrease(nowMs, timeLastBitrateChange);
                int bandwidth = lastBandwidthEstimation + additiveIncreaseBps;
                bandwidth = std::min(bandwidth, static_cast<int>(throughputBasedLimit));
                timeLastBitrateChange = nowMs;
                return bandwidth;
            } else if (rateControlRegion == "kRcMaxUnknown") {
                // maximum value unknown, multiplicative increase
                double multiplicativeChangeBps = multiplicativeRateIncrease(nowMs, timeLastBitrateChange);
                int bandwidth = lastBandwidthEstimation + multiplicativeChangeBps;
                timeLastBitrateChange = nowMs;
                return bandwidth;
            }
        } else if (state == "Decrease") {
            double beta = 0.85;
            int bandwidth = beta * estimatedThroughputBps + 0.5;
            if (bandwidth > lastBandwidthEstimation) {
                if (rateControlRegion != "kRcMaxUnknown") {
                    bandwidth = static_cast<int>(beta * avgMaxBitrateKbps * 1000 + 0.5);
                }
                bandwidth = std::min(bandwidth, lastBandwidthEstimation);
            }
            rateControlRegion = "kRcNearMax";

            if (estimatedThroughputKbps < avgMaxBitrateKbps - 3 * stdMaxBitrate) {
                avgMaxBitrateKbps = -1;
            }
            updateMaxThroughputEstimate(estimatedThroughputKbps);

            state = "Hold";
            timeLastBitrateChange = nowMs;
            return bandwidth;
        } else {
            timeLastBitrateChange = nowMs;
            return lastBandwidthEstimation;
        }
        return lastBandwidthEstimation;
    }

    void updateMaxThroughputEstimate(double estimatedThroughputKbps) {
        if (avgMaxBitrateKbps == -1) {
            avgMaxBitrateKbps = estimatedThroughputKbps;
            varMaxBitrateKbps = 100.0;
        } else {
            double k = 0.05;
            varMaxBitrateKbps = (1 - k) * varMaxBitrateKbps + k * (avgMaxBitrateKbps - estimatedThroughputKbps) *
                                (avgMaxBitrateKbps - estimatedThroughputKbps);
            avgMaxBitrateKbps = (1 - k) * avgMaxBitrateKbps + k * estimatedThroughputKbps;
        }
    }

    double additiveRateIncrease(int nowMs, int lastMs) {
        double additiveIncreaseBps = (nowMs - lastMs) * 1e6 * alpha;
        return additiveIncreaseBps;
    }

    double multiplicativeRateIncrease(int nowMs, int lastMs) {
        double multiplicativeChangeBps = (nowMs - lastMs) * 1e6 * eta;
        return multiplicativeChangeBps;
    }
};
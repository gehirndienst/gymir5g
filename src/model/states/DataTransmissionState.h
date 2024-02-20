/*
 * DataTransmissionState.h
 *
 * a class that holds and updates a data transmission state both for rx and tx flows at the application level.
 *
 * estimates the following KPIs:
 *      general packets number and payload sum in bytes for rx and tx sides
 *      session duration (s)
 *      packet loss ratio at the application level of a receiver
 *      out-of-order percentage -||-
 *      current, mean and min/max of 1-way delay (ms)
 *      current, mean and min/max of rtt (ms)
 *      mean and min/max jitter (consecutive delay and rtt variance) (ms)
 *      mean and min/max goodput for rx and tx sides (mbit/s)
 *
 * Created on: Dec 1, 2022
 *      Author: Nikita Smirnov
 */

#ifndef DATATRANSMISSIONSTATE_H
#define DATATRANSMISSIONSTATE_H

#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>

#include <inet/common/packet/chunk/Chunk.h>
#include <nlohmann/json.hpp>
#include <omnetpp.h>

#include "domain/Statistics.h"
#include "model/common/PacketType.h"

// we do not directly need these, it is just for syntax simplicity
#include "apps/ship/messages/ShipPacket_m.h"
#include "apps/shore/messages/AckPacket_m.h"
#include "apps/shore/messages/ReceiverReportPacket_m.h"
#include "apps/shore/messages/TransportFeedbackPacket_m.h"

class DataTransmissionState {
    inline static const double RTT_ALPHA = 0.125;
public:
    DataTransmissionState(int verbose = 0);
    virtual ~DataTransmissionState() = default;

    void updateWithPacket(const inet::Ptr<const inet::Chunk>& packet, PacketType packetType, int twNumber);

    void checkNetworkInstability();

    nlohmann::json toJson();

    void print(std::ostream& os);
    void print();

private:
    void onNewAckPacket(const inet::Ptr<const inet::Chunk>& packet);
    void onNewNackPacket(const inet::Ptr<const inet::Chunk>& packet);
    void onNewDataPacket(const inet::Ptr<const inet::Chunk>& packet);
    void onNewFecPacket(const inet::Ptr<const inet::Chunk>& packet);
    void onNewReceiverReportPacket(const inet::Ptr<const inet::Chunk>& packet, int twNumber);
    void onNewTransportFeedbackPacket(const inet::Ptr<const inet::Chunk>& packet, int twNumber);
public:
    // general params
    // max time in ms for rt no reply after which the network is considered instable
    inline static const int instableAfterNoReplyMs = 625;
    // 2 print on each step, < 2 do not print
    int verbose;
public:
    /* PACKETS INFO */
    int numRxPackets = 0;
    int numTxPackets = 0;
    long numRxBytes = 0;
    long numTxBytes = 0;

    /* data packets */
    // "good" packets:
    int lastOkPacketId = 0;
    int numOutOfOrderPackets = 0;
    double outOfOrderRate = 0.0;
    // "bad" packets: plr = numLostPackets / (numRxPackets + numLostPackets)
    int numLostPackets = 0;
    double lossRate = 0.0;
    // RFC-defined bw two conseecutive PacketType::RECEIVERREPORT
    double fractionLossRate = 0.0;
    // rtc specific
    int numPlayedPackets = 0;
    double playRate = 0.0;
    int numRepairedPackets = 0;
    double repairRate = 0.0;
    int numRetransmittedPackets = 0;
    double retransmissionRate = 0.0;
    double successNackRate = 0.0;
    int numRepairedAndRetransmittedPackets = 0;

    // report packets
    int numAckPackets = 0;
    int lastAckPacketId = 0;
    int numNackPackets = 0;
    int numPliPackets = 0;
    int numFecPackets = 0;
    long numFecBytes = 0;
    int numReceiverReportPackets = 0;
    int numTransportFeedbacks = 0;
    // delivered as transport-wide numbers
    int lastReceiverReportId = 0;
    int lastTransportFeedbackId = 0;

    // delay + jitter in milliseconds
    double delay = 0.0;
    double delaySum = 0.0;
    double avDelay = 0.0;
    double minDelay = 0.0;
    double maxDelay = 0.0;
    double delayJitter = 0.0;
    double delayJitterSum = 0.0;
    double avDelayJitter = 0.0;
    // RFC-defined with a special discount factor, delivered only by PacketType::RECEIVERREPORT
    double interarrivalJitter = 0.0;
    double interarrivalDelaySum = 0.0;
    double avInterarrivalDelay = 0.0;
    double fractionAvInterarrivalDelay = 0.0;
    double fractionAvTransmissionDelay = 0.0;
    double fractionAvRetransmissionDelay = 0.0;
    double fractionAvPlayoutDelay = 0.0;
    double fractionStallingRate = 0.0;
    std::vector<std::pair<int, double>> deltas;
    std::vector<std::pair<int, int>> packets;

    // rtt + rtt jitter in milliseconds for ACKs or RRS
    double rtt = 0.0;
    double rttSum = 0.0;
    double avRtt = 0.0;
    double minRtt = std::numeric_limits<double>::max();
    double maxRtt = std::numeric_limits<double>::min();
    double rttJitter = 0.0;
    double rttJitterSum = 0.0;
    double avRttJitter = 0.0;

    // RX goodput in mbps
    int bandwidth = 0; // in bps, comes from GCCEstimator
    double rxGoodput = 0.0;
    double avRxGoodput = 0.0;
    double minRxGoodput = std::numeric_limits<double>::max();
    double maxRxGoodput = std::numeric_limits<double>::min();
    double rxFecGoodput = 0.0;
    double avRxFecGoodput = 0.0;
    double minRxFecGoodput = std::numeric_limits<double>::max();
    double maxRxFecGoodput = std::numeric_limits<double>::min();

    // TX goodput in mbps
    double avTxGoodput = 0.0;
    double minTxGoodput = std::numeric_limits<double>::max();
    double maxTxGoodput = std::numeric_limits<double>::min();

    // instability parameters: when congestion distortes realibility in state estimation
    omnetpp::simtime_t timeInstabilityStarts = 0.0;
    double instabilityDuration = 0.0;
    double instabilityRate = 0.0;
    bool isInstabilityNow = false;

    // general timestamps
    omnetpp::simtime_t timeFirstPacketArrival;
    omnetpp::simtime_t timeCurrentPacketArrival;
    omnetpp::simtime_t timeFirstPacketSent;
    omnetpp::simtime_t timeCurrentPacketSent;
    omnetpp::simtime_t timeFirstPacketAck; // FIXME:  do we need these?
    omnetpp::simtime_t timeCurrentPacketAck;
    omnetpp::simtime_t timeCurrentTransportFeedback;
    omnetpp::simtime_t timeCurrentReceiverReport;
    double rxDuration = 0.0;
    double txDuration = 0.0;
};

#endif

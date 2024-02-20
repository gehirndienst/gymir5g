/*
 * StreamReceiver.h
 *
 * a class that encapsulates all stream receiving activities: chunks reassembling, statistics estimation, etc.
 *
 *  Created on: Oct 26, 2022
 *      Author: Nikita Smirnov
 */

#ifndef STREAMRECEIVER_H
#define STREAMRECEIVER_H

#include <algorithm>
#include <chrono>
#include <mutex>
#include <string>
#include <vector>

#include "boost/circular_buffer.hpp"

#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/Chunk.h"
#include "omnetpp.h"

#include "domain/Statistics.h"
#include "model/media/lidar/PointCloudDecoder.h"
#include "model/media/video/VideoDecoder.h"
#include "model/network/GCCEstimator.h"
#include "model/network/JitterBuffer.h"
#include "model/network/LostPacket.h"
#include "model/network/PlaybackBuffer.h"
#include "model/network/RepairBuffer.h"

using namespace omnetpp;

/*
 * StreamChunk struct: assembles stream packets at the receiver side
 */

struct StreamChunk {
    int size;
    int offset;
    double arrivalTime = 0.0;
    uint8_t* data = nullptr;

    StreamChunk(int size, int offset, double arrivalTime = 0.0)
        : size(size), offset(offset), arrivalTime(arrivalTime), data(new uint8_t[size]) {}

    ~StreamChunk() {
        if (data) {
            delete data;
        }
    }

    StreamChunk(const StreamChunk& src)
        : size(src.size), offset(src.offset), arrivalTime(src.arrivalTime), data(new uint8_t[src.size]) {
        std::copy(src.data, src.data + src.size, data);
    }

    StreamChunk(StreamChunk&& src) : size(0), offset(0), arrivalTime(0.0), data(nullptr) {
        std::swap(size, src.size);
        std::swap(offset, src.offset);
        std::swap(arrivalTime, src.arrivalTime);
        std::swap(data, src.data);
    }

    StreamChunk& operator = (const StreamChunk& rhs) {
        StreamChunk tmp(rhs);
        std::swap(size, tmp.size);
        std::swap(offset, tmp.offset);
        std::swap(arrivalTime, tmp.arrivalTime);
        std::swap(data, tmp.data);
        return *this;
    }

    bool operator > (const StreamChunk& other) const {
        return offset > other.offset;
    }
};

/*
 * StreamDecoder class: decodes h264 and pcl stream packets at the receiver side
 */

class StreamDecoder {
public:
    StreamDecoder(const std::string& streamName, StreamType streamType, int verbose);
    StreamDecoder() = default;
    virtual ~StreamDecoder() = default;

    bool decode(uint8_t* buffer, int bufferSize);
    void view();
    void write();

    void clearAssemblingContainer();
private:
    std::string streamName_;
    StreamType streamType_;
    VideoDecoder vDecoder_;
    PointCloudDecoder pcDecoder_;
};

/*
 * StreamReporter class: collects statistics about incoming stream-related packets to generate RTCP-like feedbacks
 * currently estimates values needed for RTCP-like ReceiverReport and TransportFeedback packets
 * and also manages retransmission process
 */

class StreamReporter {
public:
    StreamReporter();
    virtual ~StreamReporter();

    void processRtpPacket(int sequenceNumber, int payloadSize, simtime_t senderTimestamp, simtime_t arrivalTimestamp);
    void processRetransmittedRtpPacket(int sequenceNumber, int payloadSize, simtime_t payloadTimestamp,
                                       simtime_t retranmissionTimesstamp, simtime_t arrivalTimestamp);
    void processFecPacket(int payloadSize);

    void processSenderReport(simtime_t senderTimestamp);

    void estimateFractionStats();
    void reportLostPacket();
    void reportPlayedPacket(double playoutDelay);
    void reportRepairedPacket(int sequenceNumber, int repairedAndRetransmittedPackets);

    void eraseTFVectors();
    void eraseCachedStatistic();

    std::vector<int>& getLostSequenceNumbers();

public:
    GCCEstimator gccEstimator;

    int bandwidth; // bps
    double relativeTransitTime;
    double jitter;

    int totalPacketsReceived;
    int totalPacketsLost;
    int totalPacketsOutOfOrder;
    int totalPacketsPlayed;
    int totalPacketsRepaired;
    int totalPacketsRetransmitted;
    int totalPacketsRepairedAndRetransmitted;
    long totalBytesReceived;
    int maxSequenceNumber;

    int fractionLastMaxSequenceNumber;
    int fractionPacketsReceived;
    int fractionPacketsRepaired;
    int fractionPacketsRetransmitted;
    int fractionPacketsRepairedAndRetransmitted;
    double fractionLost;
    double fractionRate;
    double fractionFecRate;
    double fractionStallingRate;

    Statistics<double> interarrivalDelay;
    Statistics<double> playoutDelay;
    Statistics<double> playbackDelay;
    double stallingSum;

    // these assume synchronized clocks between sender and receiver
    Statistics<double> transmissionDelay;
    Statistics<double> retransmissionDelay;

    simtime_t lastPacketArrivalTimestamp;
    simtime_t lastSenderReportSentTimestamp;
    simtime_t lastSenderReportArrivalTimestamp;
    simtime_t lastReceiverReportSentTimestamp;
    simtime_t lastPacketPlayTimestamp;

    std::vector<int> lostSequenceNumbers;

    std::vector<int> sequenceNumbersSinceLastRR;
    int bytesReceivedSinceLastRR;
    int bytesFecReceivedSinceLastRR;

    std::vector<int> sequenceNumbersSinceLastTF;
    std::vector<simtime_t> deltasSinceLastTF;
};
/*
 * StreamReceiver class: collects statistics for a stream at the receiver side, stores and assembles single units into frames/clouds
 */

class StreamReceiver {
public:
    StreamReceiver();
    StreamReceiver(const std::string& streamName, StreamType streamType, int verbose = 0);
    virtual ~StreamReceiver() = default;

    // assembling buffer
    void addChunk(int elemNumber, int payloadSize, int offset, double arrivalTime, inet::Packet* packetData);
    void forceDecodePreviousElements(int elemNumber);
    std::vector<StreamChunk> getChunks(int elemNumber);
    int getSizeOfAllChunks(int elemNumber);

    // lost packets (FEC + NACK) management
    LostPacketsMap& getLostPacketsMap();
    int getNextLostSequenceNumber();
    int getFirstLostSequenceNumberInRange(int left, int right);
    void putIntoLostPacketsMap(int lostSequenceNumber, simtime_t currentTime, bool isFec, bool isNack);
    void updateNextLostSequenceNumber(int sequenceNumber);
    void eraseAndUpdateNextLostSequenceNumber(int sequenceNumber);

    // playback buffer
    void pushIntoPlaybackBuffer(const inet::Packet* packet, int sequenceNumber, omnetpp::simtime_t currTime);
    std::optional<const PacketEntry> getNextFromPlaybackBuffer();
    int getMaxPushedSequenceNumber();
    int getMaxPlayedSequenceNumber();
    int getRepairedAndRetransmittedPackets();

    // FIXME: use when jitter buffer is ready
    std::pair<bool, bool> pushIntoJitterBuffer(const inet::Packet* packet, int sequenceNumber, int frameNumber,
            omnetpp::simtime_t currTime, bool isLast);
    std::vector<JitterFrame> removeJitterFrame(int frameNumber);
    omnetpp::cMessage* getPlayMsg(int frameNumber);
    int getLastPlayedFrame();

    // repair buffer
    void addSequenceNumberToRepairBuffer(int sequenceNumber);
    void addRepairSequence(int firstSequenceNumber, int lastSequenceNumber);
    RepairSequence* findRepairSequence(int sequenceNumber);
    RepairStatus getRepairStatus(int lostSequenceNumber, RepairSequence* repairSequence = nullptr);

    // decoding
    void decode(int elemNumber);
    void view();
    void write();

public:
    std::string streamName;
    StreamType streamType;
    StreamReporter streamReporter;
    int elementsDecoded;

private:
    std::map <int, std::vector<StreamChunk>> chunks_;
    LostPacketsMap lostPacketsMap_;
    JitterBuffer jitterBuffer_;
    PlaybackBuffer playbackBuffer_;
    StreamDecoder decoder_;
    RepairBuffer repairBuffer_;
};


#endif
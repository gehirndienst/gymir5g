/*
 * JitterFrame.h
 *
 * jitter frame pushed in a jitter buffer
 *
 *  Created on: Oct 10, 2023
 *      Author: Nikita Smirnov
 */

#ifndef JITTERFRAME_H
#define JITTERFRAME_H

#include "model/network/PacketEntry.h"

#include "omnetpp.h"
using namespace omnetpp;

struct JitterFrame {
    std::vector<PacketEntry> packets;
    int minSequenceNumber;
    int maxSequenceNumber;
    int frameNumber;
    bool isLastArrived;
    cMessage* playMsg;
    simtime_t timeAdded;

    JitterFrame(const std::string& streamName, int frameNumber)
        : minSequenceNumber(0)
        , maxSequenceNumber(0)
        , frameNumber(frameNumber)
        , isLastArrived(false) {

        playMsg = new cMessage("play");
        playMsg->addPar("streamName");
        playMsg->par("streamName").setStringValue(streamName.c_str());
        playMsg->addPar("frameNumber");
        playMsg->par("frameNumber").setLongValue((long)frameNumber);
        timeAdded = simTime();
    }

    JitterFrame() : minSequenceNumber(0), maxSequenceNumber(0), frameNumber(0), isLastArrived(false), playMsg(nullptr) {}

    bool addPacket(const inet::Packet* packet, int sequenceNumber, omnetpp::simtime_t currTime, bool isLast = false) {
        // return true if received all packets (i.e., no missing) and one of the packets is marked as "last"
        if (packets.empty()) {
            minSequenceNumber = sequenceNumber;
            maxSequenceNumber = sequenceNumber;
        } else {
            minSequenceNumber = std::min(minSequenceNumber, sequenceNumber);
            maxSequenceNumber = std::max(maxSequenceNumber, sequenceNumber);
        }
        packets.emplace_back(packet, sequenceNumber, currTime);
        isLastArrived = isLast;
        bool isFrameComplete = (maxSequenceNumber - minSequenceNumber + 1) == (int)packets.size() && isLastArrived;
        return isFrameComplete;
    }

    double getPlayoutDelay() const {
        return simTime().dbl() - timeAdded.dbl();
    }

    bool operator > (const JitterFrame& other) const {
        return this->frameNumber > other.frameNumber;
    }

    bool operator < (const JitterFrame& other) const {
        return this->frameNumber < other.frameNumber;
    }
};

#endif
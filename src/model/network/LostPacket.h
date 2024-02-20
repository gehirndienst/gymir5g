/*
 * LostPacket.h
 *
 * a header that unites all lost-packet related structures
 *
 *  Created on: Aug 14, 2023
 *      Author: Nikita Smirnov
 */

#ifndef NACK_H
#define NACK_H

#include <unordered_map>
#include <omnetpp.h>

using namespace omnetpp;

struct NackInfo {
    int nacksSent;
    simtime_t firstNackTimestamp;
    simtime_t lastNackTimestamp;

    NackInfo(): nacksSent(0), firstNackTimestamp(0.0), lastNackTimestamp(0.0) {}
    NackInfo(int nacks, omnetpp::simtime_t firstNack, omnetpp::simtime_t lastNack)
        : nacksSent(nacks), firstNackTimestamp(firstNack), lastNackTimestamp(lastNack) {}

};

// NOTE: scheduling and freeing cMessages* is possible only in cModule
struct LostPacket {
    cMessage* playMsg;
    cMessage* repairMsg;
    cMessage* nackMsg;
    NackInfo nackInfo;

    LostPacket(std::string& streamName, int sequenceNumber, simtime_t currentTime, bool isFec, bool isNack)
        : nackInfo(0, currentTime, currentTime) {
        playMsg = new cMessage("play");
        playMsg->addPar("streamName");
        playMsg->par("streamName").setStringValue(streamName.c_str());
        playMsg->addPar("sequenceNumber");
        playMsg->par("sequenceNumber").setLongValue((long)sequenceNumber);
        if (isFec) {
            repairMsg = new cMessage("repair");
            repairMsg->addPar("streamName");
            repairMsg->par("streamName").setStringValue(streamName.c_str());
            repairMsg->addPar("sequenceNumber");
            repairMsg->par("sequenceNumber").setLongValue((long)sequenceNumber);
        } else {
            repairMsg = nullptr;
        }
        if (isNack) {
            nackMsg = new cMessage("nack");
            nackMsg->addPar("streamName");
            nackMsg->par("streamName").setStringValue(streamName.c_str());
            nackMsg->addPar("sequenceNumber");
            nackMsg->par("sequenceNumber").setLongValue((long)sequenceNumber);
        } else {
            nackMsg = nullptr;
        }
    }

    LostPacket() : playMsg(nullptr), repairMsg(nullptr), nackMsg(nullptr), nackInfo() {}
};

typedef std::map<int, LostPacket> LostPacketsMap;

#endif
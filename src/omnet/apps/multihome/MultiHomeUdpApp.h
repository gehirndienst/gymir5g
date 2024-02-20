/*
 * MultiHomeUdpApp.h
 *
 * dummy multihome udp sender app: selects the network (lte, nr, wifi, random) and forwards to the appropriate stack
 *
 * Created on: Aug 4, 2023
 *      Author: Nikita Smirnov
 */

#ifndef MULTIHOMEUDPAPP_H
#define MULTIHOMEUDPAPP_H

#include <random>

#include "omnetpp.h"
#include "inet/common/INETDefs.h"
#include "inet/common/packet/Packet.h"

#include "model/stream/StreamManager.h"
#include "model/common/NetworkType.h"
#include "messages/MultiHomeMessage_m.h"

using namespace omnetpp;
using namespace inet;

class MultiHomeUdpApp: public cSimpleModule {
public:
    MultiHomeUdpApp();
    virtual ~MultiHomeUdpApp();

protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessage(cMessage* msg) override;

    virtual void sendData();
    virtual void receiveData(cMessage* msg);

    void findMultiHomeApps();
    virtual NetworkType getNetworkType();

protected:
    static simsignal_t ltePacketsSent;
    static simsignal_t nrPacketsSent;
    static simsignal_t wlanPacketsSent;
    static simsignal_t dontcarePacketsSent;
    int packetsSent;

protected:
    simtime_t initTime;
    double maxWaitingTimeToStart;

    cModule* cellularApp;
    cModule* wirelessApp;

    int packetSize;
    double sendingPeriod;

    cMessage* sendDataMsg;
    bool isScheduleAfterAppsAreFound;

    bool isNR;

    std::uniform_int_distribution<int> dontCareDist;
};

#endif

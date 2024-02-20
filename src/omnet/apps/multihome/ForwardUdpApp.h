/*
 * ForwardUdpApp.h
 *
 * receives packets via sendDirect and forwards them further via sockets to a given network stack type
 *
 * Created on: Aug 4, 2023
 *      Author: Nikita Smirnov
 */

#ifndef FORWARDUDPAPP_H
#define FORWARDUDPAPP_H

#include <random>

#include "omnetpp.h"
#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

#include "model/common/NetworkType.h"
#include "messages/MultiHomeMessage_m.h"
#include "messages/MultiHomePacket_m.h"

using namespace omnetpp;
using namespace inet;

class ForwardUdpApp: public cSimpleModule {
public:
    ForwardUdpApp() = default;
    virtual ~ForwardUdpApp() = default;

protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessage(cMessage* msg) override;

    virtual void initTraffic();

    virtual void forwardData(cMessage* msg);
    virtual void processData(cMessage* msg);
protected:
    const char* packetName;
    UdpSocket socket;
    int localPort;

    L3Address destAddress;
    int destPort;

    cModule* multiHomeApp;

    simtime_t initTime;
};

#endif

/*
 * ForwardShoreApp.h
 *
 * an app with inet interface to forward WLAN packets to the ShoreApp
 *
 * Created on: Nov 13, 2023
 *      Author: Nikita Smirnov
 */

#ifndef FORWARDSHOREAPP_H
#define FORWARDSHOREAPP_H

#include "inet/applications/base/ApplicationBase.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

#include "inet/applications/udpapp/UdpSink.h"

#include "apps/ship/messages/ShipPacket_m.h"
#include "domain/CSVLogger.h"
#include "model/common/NetworkType.h"
#include "model/stream/StreamReceiver.h"

using namespace inet;

class INET_API ForwardShoreApp : public UdpSink {
public :
    ForwardShoreApp();
    virtual ~ForwardShoreApp();
protected:
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage* msg) override;
    virtual void socketDataArrived(UdpSocket* socket, Packet* packet) override;
    virtual void finish() override;

    void log();
    std::pair<std::string, std::string> analyseNetworkTypes(std::string& streamName);

protected:
    UdpSocket socket;

    simtime_t initTime;
    int packetsRcvd;
    int lastSequenceNumber;

    std::string logdir;
    double logUpdatePeriod;
    cMessage* logUpdateMsg;
    bool verbose;

    std::unordered_map<std::string, StreamReceiver*> streamReceivers;
    std::unordered_map<std::string, CSVLogger*> loggers;
    std::unordered_map<std::string, std::vector<NetworkType>> networkTypes;
};

#endif



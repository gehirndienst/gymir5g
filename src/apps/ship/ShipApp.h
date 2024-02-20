/*
 * OMNeT++ application for a ship
 *
 * Created on: July 21, 2022
 *      Author: Nikita Smirnov
 */

#ifndef SHIPAPP_H
#define SHIPAPP_H

#include <cmath>
#include <iostream>
#include <string>
#include <map>
#include <stdexcept>
#include <vector>

#include <inet/common/INETDefs.h>
#include <inet/networklayer/common/L3AddressResolver.h>
#include <inet/transportlayer/contract/udp/UdpSocket.h>
#include <omnetpp.h>

#include <nlohmann/json.hpp>

#include "apps/shore/messages/AckPacket_m.h"
#include "apps/shore/messages/NackPacket_m.h"
#include "apps/shore/messages/ReceiverReportPacket_m.h"
#include "apps/shore/messages/TransportFeedbackPacket_m.h"

#include "domain/TaskPerformer.h"
#include "model/stream/StreamManager.h"
// should be after StreamManager.h because otherwise there is a template mess with VTK libraries
#include "model/common/NetworkType.h"
#include "model/mobility/Mobility.h"
#include "model/states/PhyDataState.h"
#include "omnet/stack/phy/NRPhyUeWithFeedback.h"
#include "service/GymCommunicator.h"

#include "messages/FecPacket_m.h"
#include "messages/NackDecision_m.h"
#include "messages/SenderReportPacket_m.h"
#include "messages/ShipPacket_m.h"
#include "messages/ShipOffsetMessage_m.h"

using namespace omnetpp;
using namespace inet;

class ShipApp : public cSimpleModule, public cListener {
public:
    ShipApp();
    ~ShipApp();
protected:
    // override functions from the parent omnet++ class
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage* msg) override;
    virtual void finish() override;

    virtual void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject*) override;
private:
    // initialization
    void initTraffic();
    void initScheduler();

    // mobility
    void getMobility();

    // send packets
    void sendData(const char* streamName);
    void sendSenderReport(const char* streamName);
    void sendDataWithOffsets(std::string& streamName, int offset);
    void generateAndSendPacket(std::string& streamName, int offset = 0);
    void sendViaSocket(Packet* packet, NetworkType networkType = NetworkType::DONTCARE);

    // rtc
    void receivePacket(cMessage* msg);
    bool getNackDecision();

    // net instability
    void checkNetworkInstability();

    // logging
    void log();

    // future data
    void getFutureStreamData(std::string& streamName, int futureLookup = 0);

    // multihoming
    void findMultiHomeApps();
    NetworkType getNetworkType();

    // learning stuff
    void pushStates();
    void pullActions();
    void applyActions(nlohmann::json& actions);
    void nonDrlStep();

protected:
    // general transport parameters
    int localPortData;
    int destPort;
    L3Address destAddress;

    // communication parameters
    UdpSocket socket;

    /// multihoming
    bool isMultiHome;
    cModule* cellularApp;
    cModule* wirelessApp;

    // time
    simtime_t initTime;
    simtime_t startTime;
    simtime_t finishTime;
    double logUpdatePeriod;

    // mobility
    Mobility mobility;
    cMessage* mobilityMsg;
    bool isVeinsMobility;
    double maxWaitingTimeForMobility;

    // rtc
    double senderReportPeriod;
    bool isFec;
    int fecAfterPackets;

    // simultaneous sending?
    bool isSendingSimultaneously;
    double delayDelta;

    // data objects for streams
    std::unordered_map<std::string, StreamData> dataMap;
    // future data
    bool isUsingFutureData;
    TaskPerformer taskPerformer;
    std::unordered_map<std::string, std::queue<std::future<StreamData>>> futureDataMap;

    // stream manager object
    StreamManager* streamManager;
    std::string logdir;

    // net instability msg object
    cMessage* netInstabilityMsg;

    // adaptive streaming
    AdaptiveAlgorithm algorithm;
    double stateUpdatePeriod;
    GymCommunicator* gymCommunicator;
    std::thread gymThread;
    cMessage* actionMsg;
    cMessage* stateMsg;
    bool isIpc;

    // msg containers for streams
    std::unordered_map<std::string, cMessage*> dataMsgMap;
    std::unordered_map<std::string, cMessage*> senderReportMsgMap;
};

#endif

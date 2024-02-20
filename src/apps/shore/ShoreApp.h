/*
 * OMNeT++ application for a shore-based control center
 *
 * Created on: July 27, 2022
 *      Author: Nikita Smirnov
 */

#ifndef SHOREAPP_H
#define SHOREAPP_H

#include <iostream>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>

#include <inet/common/INETDefs.h>
#include <inet/networklayer/common/L3AddressResolver.h>
#include <inet/transportlayer/contract/udp/UdpSocket.h>
#include <omnetpp.h>

#include <nlohmann/json.hpp>

#include "apps/ship/messages/FecPacket_m.h"
#include "apps/ship/messages/NackDecision_m.h"
#include "apps/ship/messages/ShipPacket_m.h"
#include "apps/ship/messages/SenderReportPacket_m.h"
#include "model/stream/StreamReceiver.h"
#include "service/SignalListener.h"

#include "messages/AckPacket_m.h"
#include "messages/NackPacket_m.h"
#include "messages/ReceiverReportPacket_m.h"
#include "messages/TransportFeedbackPacket_m.h"

using namespace omnetpp;
using namespace inet;

class ShoreApp : public cSimpleModule {
public:
    ShoreApp();
    ~ShoreApp();
protected:
    // override functions from the parent omnet++ class
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage* msg) override;
    virtual void finish() override;

private:
    void initTraffic();
    void initScheduler();

    // recieve packets
    void receiveDataPacket(cMessage* msg);
    void receiveFecPacket(cMessage* msg);
    void receiveReport(cMessage* msg);
    void receiveNackRefusal(cMessage* msg);

    // rtc
    void sendAck(const inet::Ptr<const ShipPacket>& rtpHeader);
    void sendNackIfNeeded(cMessage* nackScheduledMessage);
    void sendReceiverReport();
    void sendTransportFeedback();

    // repair
    void tryRepairPacket(cMessage* msg);

    // play
    void removeFromLost(std::string& streamName, int sequenceNumber);
    void resumePlay(std::string& streamName, int sequenceNumber);
    void play(std::string& streamName);
    void playData(const inet::Ptr<const ShipPacket>& header, inet::Packet* data, simtime_t playbackTimestamp);

protected:
    // general transport parameters
    int localPortData;
    int destPort;
    L3Address destAddress;
    cModule* shipApp;

    // communication parameters
    UdpSocket socket;

    // time
    simtime_t initTime;
    simtime_t startTime;

    // rtc
    bool isAck;
    bool isSack;

    bool isNack;
    int maxNacks;
    double maxNackDelay;
    double maxPlayoutDelay;
    int nackCount;

    bool isFec;
    double repairPeriod;

    bool isFir;

    bool isDirectReceiverReport;
    double receiverReportPeriod;
    int receiverReportCount;

    bool isDirectTransportFeedback;
    double transportFeedbackPeriod;
    int transportFeedbackCount;
    int firstTwSequenceNumber;
    bool isNewTf;

    // streams processing
    std::unordered_map<std::string, StreamReceiver*> streamReceivers;
    bool isDecode;
    bool isView;

    // verbose
    int verbose;

    // msgs
    cMessage* receiverReportMsg;
    cMessage* transportFeedbackMsg;
};

#endif

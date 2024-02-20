#include "ShipApp.h"

Define_Module(ShipApp);

ShipApp::ShipApp()
    : mobilityMsg(nullptr)
    , streamManager(nullptr)
    , netInstabilityMsg(nullptr)
    , gymCommunicator(nullptr)
    , actionMsg(nullptr)
    , stateMsg(nullptr) {}

ShipApp::~ShipApp() {
    cancelAndDelete(mobilityMsg);
    cancelAndDelete(netInstabilityMsg);
    cancelAndDelete(actionMsg);
    cancelAndDelete(stateMsg);
    for (const auto& p : dataMsgMap) {
        cancelAndDelete(p.second);
        cancelAndDelete(senderReportMsgMap.at(p.first));
    }
    dataMsgMap.clear();
    senderReportMsgMap.clear();

}

void ShipApp::initialize(int stage) {
    cSimpleModule::initialize(stage);
    if (stage != inet::INITSTAGE_APPLICATION_LAYER)
        return;

    initTime = simTime();
    finishTime = std::stod(cSimulation::getActiveEnvir()->getConfigEx()->getConfigValue("sim-time-limit"));
    logUpdatePeriod = par("logUpdatePeriod");

    // multihoming
    isMultiHome = par("isMultiHome").boolValue();
    if (isMultiHome) {
        findMultiHomeApps();
    }

    // mobility
    mobility = Mobility();
    mobilityMsg = new cMessage("mobility");
    maxWaitingTimeForMobility = par("maxWaitingTimeForMobility").doubleValue();
    isVeinsMobility = par("isVeinsMobility").boolValue();
    getParentModule()->subscribe(inet::IMobility::mobilityStateChangedSignal, this);

    // net instability msg
    netInstabilityMsg = new cMessage("instability");

    // set streams through a manager
    streamManager = new StreamManager();
    std::ifstream streamsConfigurationSchema(par("streamsConfigurationSchema").stringValue());
    auto streamCfgs = nlohmann::json::parse(streamsConfigurationSchema);
    logdir = par("logdir").stringValue();
    streamManager->setAllStreams(streamCfgs, logdir);

    // retransmission buffer? if 0 nothing happens
    streamManager->setRetransmissionBuffers(par("maxRetransmitAge").doubleValue());

    // use FEC?
    isFec = par("isFec").boolValue();
    fecAfterPackets = isFec ? par("fecAfterPackets").intValue() : 0;

    // send simultaneously?
    isSendingSimultaneously = par("isSendingSimultaneously").boolValue();
    if (!isSendingSimultaneously) {
        //  TODO: turn this feature into a full-scale pacer.
        // Add delayDelta as bucketDelay and take min(bucketDelay, (nextSendingTime - simTime()).dbl() / dataMap[streamNameStr].data.size();)
        delayDelta = 0.0;
    }

    // use future data?
    isUsingFutureData = par("isUsingFutureData").boolValue();

    // init AS or not?
    algorithm = adaptivealgorithm::fromString(par("adaptiveAlgorithm").stringValue());
    streamManager->setAdaptiveStreaming(algorithm);
    stateMsg = new cMessage("step");
    isIpc = adaptivealgorithm::toString(algorithm).compare(0, 3, "drl") == 0;
    if (algorithm != AdaptiveAlgorithm::NO) {
        // state update period
        stateUpdatePeriod = par("stateUpdatePeriod").doubleValue();
        logUpdatePeriod = stateUpdatePeriod;

        // if starts with DRL we need to init an IPC module
        if (isIpc) {
            // msg objs for pushing a new state and pulling new actions (overwrite "step")
            actionMsg = new cMessage("pull");
            stateMsg = new cMessage("push");

            // init gym communication module
            gymCommunicator = new GymCommunicator();
            gymCommunicator->init(par("hostAddress").stringValue(), par("noReplyTimeout").intValue());

            // run gym communicator in a separate thread
            gymThread = std::thread(&GymCommunicator::run, gymCommunicator);

            // initial request
            pushStates();
        }
        std::cout << "ShipApp::initialize succesfully initialized adaptive streaming, alg: "
                  << adaptivealgorithm::toString(algorithm) << std::endl;
    }

    initTraffic();
}

void ShipApp::finish() {
    delete streamManager;

    if (gymCommunicator) {
        gymCommunicator->finish();
        gymThread.detach();
        delete gymCommunicator;
    }

    std::cout << "ShipApp::finish closing ShipApp..." << std::endl;
}

void ShipApp::handleMessage(cMessage* msg) {
    if (msg->isSelfMessage()) {
        if (!strcmp(msg->getName(), "reinit")) {
            // reinit traffic
            delete msg;
            initTraffic();
        } else if (!strcmp(msg->getName(), "findApps")) {
            // find cellular/wireless apps
            delete msg;
            findMultiHomeApps();
        } else if (!strcmp(msg->getName(), "mobility")) {
            // get mobility
            getMobility();
        } else if (!strcmp(msg->getName(), "instability")) {
            // check network instability
            checkNetworkInstability();
        } else if (!strcmp(msg->getName(), "step")) {
            // apply non-DRL ABR action (use the same update period as for DRL), if asMode = 0 - only logging
            nonDrlStep();
        } else if (!strcmp(msg->getName(), "pull")) {
            // try to pull a new action from OpenAI Gym environment if it is available
            pullActions();
        } else if (!strcmp(msg->getName(), "push")) {
            // push a new state to OpenAI Gym environment
            pushStates();
        } else if (!strcmp(msg->getName(), "offset")) {
            // offset chunks sending
            ShipOffsetMessage* offsetMsg = (ShipOffsetMessage*)msg;
            std::string streamName = std::string(offsetMsg->getStreamName());
            int offset = offsetMsg->getOffset();
            sendDataWithOffsets(streamName, offset);
            delete msg;
        } else if (!strcmp(msg->getName(), "stream")) {
            // either data or sender report for a certain stream
            if (msg->findPar("streamName") >= 0) {
                const char* streamName = msg->par("streamName").stringValue();
                if (msg->par("isData").boolValue()) {
                    // data
                    sendData(streamName);
                } else {
                    // sender report
                    sendSenderReport(streamName);
                }
            } else {
                std::cerr << "[ShipApp::handleMessage] stream msg has no streamName par" << std::endl;
            }
        } else {
            std::cerr << "[ShipApp::handleMessage] unknown self msg with name " << msg->getName() << std::endl;
        }
    } else {
        receivePacket(msg);
    }
}

void ShipApp::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject*) {
    if (signalID == inet::IMobility::mobilityStateChangedSignal) {
        // TODO: add something interesting later to do here
        return;
    }
}


void ShipApp::findMultiHomeApps() {
    // find cellular app which module could be dynamic
    cellularApp = findModuleByPath(par("cellularAppPath"));
    if (!cellularApp) {
        // cellular module can be dynamic, e.g., simu5g.nodes.cars.NRCar
        if ((simTime() - initTime).dbl() > par("maxWaitingTimeToStart").doubleValue()) {
            throw cRuntimeError("ShipApp::findMultiHomeApps: can't find cellular app, aborting...");
        }
        double offset = uniform(0.01, 0.05);
        scheduleAt(simTime() + offset, new cMessage("findApps"));
    } else {
        // if cellular app is found then proceed with wireless (it should not be dynamic) and start data sending
        std::cout << "ShipApp::findMultiHomeApps found cellular app! proceeding..."
                  << std::endl;
        wirelessApp = findModuleByPath(par("wirelessAppPath"));
        if (!wirelessApp) {
            throw cRuntimeError("ShipApp::findMultiHomeApps: can't find wireless app, aborting...");
        }
        std::cout << "ShipApp::findMultiHomeApps found wireless app! proceeding..."
                  << std::endl;
        std::cout << "ShipApp::findMultiHomeApps all apps are found, initializing the traffic..."
                  << std::endl;
    }
}

void ShipApp::initTraffic() {
    cModule* destModule = findModuleByPath(par("destAddress").stringValue());
    if (!destModule) {
        // check if control station (shore) is also initialized
        if ((simTime() - initTime).dbl() > par("maxWaitingTimeToStart").doubleValue()) {
            throw std::runtime_error("ShipApp::initTraffic shore app hasn't been found, stopping...");
        }
        double offset = uniform(0.01, 0.05);
        scheduleAt(simTime() + offset, new cMessage("reinit"));
        std::cout << "ShipApp::initTraffic the node will retry to initialize traffic in "
                  << offset << " seconds " << endl;
    } else {
        destAddress = L3AddressResolver().resolve(par("destAddress"));
        destPort = par("destPort");

        // UDP socket: send datagrams with stream data to the shore
        localPortData = par("localPortData");
        if (!isMultiHome) {
            socket.setOutputGate(gate("socketOut"));
            socket.bind(localPortData);
        }
        int tos = par("tos");
        if (tos != -1)
            socket.setTos(tos);
        std::cout << "ShipApp::initTraffic UDP socket is binded to local port " << localPortData
                  << " and sends datagrams to: " << destAddress << ":" << std::to_string(destPort) << endl;

        // start all threads with streams
        streamManager->runAllStreams();

        // schedule all messages
        initScheduler();

        std::cout << "ShipApp::initTraffic start data sending! ... " << endl;
    }
}

void ShipApp::initScheduler()  {
    startTime = simTime() + par("warmUp");

    // fill stream msg maps
    for (auto streamName : streamManager->getStreamNames()) {
        // set async data filling
        if (isUsingFutureData) {
            getFutureStreamData(streamName, 1);
        }

        // data msg
        cMessage* dataMsg = new cMessage();
        dataMsg->setName("stream");
        dataMsg->addPar("streamName");
        dataMsg->par("streamName").setStringValue(streamName.c_str());
        dataMsg->addPar("isData");
        dataMsg->par("isData").setBoolValue(true);
        dataMsgMap.emplace(streamName, dataMsg);

        // send first data chunk
        scheduleAt(startTime, dataMsgMap[streamName]);

        // sender report msg
        senderReportPeriod = par("senderReportPeriod");
        if (senderReportPeriod > 0) {
            cMessage* senderReportMsg = new cMessage();
            senderReportMsg->setName("stream");
            senderReportMsg->addPar("streamName");
            senderReportMsg->par("streamName").setStringValue(streamName.c_str());
            senderReportMsg->addPar("isData");
            senderReportMsg->par("isData").setBoolValue(false);
            senderReportMsgMap.emplace(streamName, senderReportMsg);

            // send first sender report
            scheduleAt(startTime  + senderReportPeriod, senderReportMsgMap[streamName]);
        }
    }

    // set mobility msg
    scheduleAt(startTime, mobilityMsg);

    // set adaptive streaming msgs
    if (isIpc) {
        scheduleAt(startTime, actionMsg);
        scheduleAt(startTime, netInstabilityMsg);
    } else {
        // until remade: for non-ipc step we don't have initial state pushed in pushState()
        scheduleAt(startTime + 0.01 + stateUpdatePeriod, stateMsg);
    }
}

void ShipApp::getMobility() {
    // get mobility from Veins or from INET
    if (isVeinsMobility) {
        try {
            auto* vim = check_and_cast<veins::VeinsInetMobility*>(getParentModule()->getSubmodule("mobility"));
            if (vim) {
                mobility.setVeinsMobility(vim);
                std::cout << "ShipApp::getMobility veins mobility has been found! "  << "current crds "
                          << mobility.getCoords().str()  << std::endl;
            } else {
                if ((simTime() - startTime).dbl() > maxWaitingTimeForMobility) {
                    throw std::runtime_error("ShipApp::getMobility veins mobility hasn't been found within a max timeout, stopping...");
                } else {
                    scheduleAt(simTime() + 0.1, mobilityMsg);
                }
            }
        } catch (const cRuntimeError& err) {
            throw std::runtime_error("ShipApp::getMobility error by getting veins mobility " + std::string(err.what()));
        }
    } else {
        IMobility* im = check_and_cast<IMobility*>(getContainingNode(this)->getSubmodule("mobility"));
        if (im) {
            mobility.setInetMobility(im);
            std::cout << "ShipApp::getMobility inet mobility has been found! "  << "current crds "
                      << mobility.getCoords().str() << std::endl;
        } else {
            if ((simTime() - startTime).dbl() > maxWaitingTimeForMobility) {
                throw std::runtime_error("ShipApp::getMobility inet mobility hasn't been found within a max timeout, stopping...");
            } else {
                scheduleAt(simTime() + 0.1, mobilityMsg);
            }
        }
    }
}

void ShipApp::getFutureStreamData(std::string& streamName, int futureLookup) {
    // TODO: add some mechanism to allow encoding multiple frames
    StreamType streamType = streamManager->getStreamType(streamName);
    int chunksPlannedDuringStateUpdatePeriod =
        std::floor(stateUpdatePeriod / streamManager->getStreamSendInterval(streamName));

    int tasksNum;
    if (futureLookup == 0) {
        tasksNum = chunksPlannedDuringStateUpdatePeriod;
    } else {
        tasksNum = futureLookup;
    }
    int futureDataSize =
        streamType != SIM ? std::min(streamManager->getStreamQueueSize(streamName), tasksNum) : tasksNum;

    // async execute StreamData generation
    for (int i = 0; i < futureDataSize; i++) {
        futureDataMap[streamName].emplace(
            taskPerformer.addTask(&StreamManager::generateFutureStreamData, this->streamManager, streamName));
    }
}

void ShipApp::sendData(const char* streamName) {
    std::string streamNameStr = std::string(streamName);
    omnetpp::simtime_t nextSendingTime = streamManager->getStreamNextSendingTime(streamNameStr);

    // check if stream has been paused
    if (!streamManager->getStreamIsRunning(streamNameStr)) {
        scheduleAt(nextSendingTime, new cMessage(streamName));
        return;
    }

    // get the new data from the stream
    if (isUsingFutureData) {
        if (!futureDataMap[streamName].empty()) {
            dataMap[streamNameStr] = futureDataMap[streamName].front().get();
            futureDataMap[streamName].pop();
        } else {
            // could be that it was not future data at the moment it was asked to be generated
            // but there is a real data now (esp. for handlers with low bufferSizes)
            streamManager->fillStreamData(streamNameStr);
            dataMap[streamNameStr] = streamManager->getStreamData(streamNameStr);
        }
        // fill async
        getFutureStreamData(streamNameStr, 1);
    } else {
        streamManager->fillStreamData(streamNameStr);
        dataMap[streamNameStr] = streamManager->getStreamData(streamNameStr);
    }

    // check if stream has stopped
    if (dataMap[streamNameStr].isEmpty()) {
        auto streamStatus = streamManager->getStreamStatus(streamNameStr);
        if (streamStatus == FINISHED || streamStatus == FAIL) {
            std::cerr << "ShipApp::sendData Stream " << streamNameStr
                      << " has no data to send! Stopping the stream... \nTime: " \
                      + std::to_string(omnetpp::simTime().dbl()) << std::endl;
        } else {
            scheduleAt(simTime() + 0.01, new cMessage(streamName));
        }
        return;
    }

    // send fragmented data: either all at once or step-by-step
    if (isSendingSimultaneously) {
        int offset = 0;
        for (auto it = dataMap[streamNameStr].data.begin(); it != dataMap[streamNameStr].data.end(); ++it) {
            generateAndSendPacket(streamNameStr, offset);
            offset ++;
        }
    } else {
        // schedule fragments sending with equal delay delta between two consequtive stream data calls
        delayDelta = (nextSendingTime - simTime()).dbl() / dataMap[streamNameStr].data.size();
        scheduleAt(simTime(), new cMessage("offset"));
    }

    // schedule the next stream data
    scheduleAt(nextSendingTime, dataMsgMap[streamNameStr]);
}

void ShipApp::sendSenderReport(const char* streamName) {
    Packet* packet = new Packet();
    packet->setName("senderReport");
    std::string streamNameStr = streamName;
    NetworkType networkType = getNetworkType();

    auto header = inet::makeShared<SenderReportPacket>();
    auto& dts = streamManager->getStreamDataTransmissionState(streamNameStr);
    header->setStreamName(streamName);
    header->setSenderPacketCount(dts.numTxPackets);
    header->setSenderOctetCount(dts.numTxBytes);
    header->setTimestamp(simTime());
    header->setNetworkType(static_cast<int>(networkType));
    packet->insertAtFront(header);

    sendViaSocket(packet, networkType);
    scheduleAt(simTime() + senderReportPeriod, senderReportMsgMap[streamNameStr]);
}

void ShipApp::sendDataWithOffsets(std::string& streamName, int offset) {
    // a version for sending fragments one-by-one between two sending times, should be called only after sendData
    generateAndSendPacket(streamName, offset);
    if (offset < (int)dataMap[streamName].data.size() - 1) {
        auto* newOffsetMsg = new ShipOffsetMessage("offset");
        newOffsetMsg->setName(streamName.c_str());
        newOffsetMsg->setOffset(offset ++);
        scheduleAt(simTime() + delayDelta, newOffsetMsg);
    }
}

void ShipApp::generateAndSendPacket(std::string& streamName, int offset) {
    Packet* packet = new Packet();
    packet->setName("data");

    int sequenceNumber = streamManager->getStreamPacketsSent(streamName) + 1;
    bool isLast = offset == (int)dataMap[streamName].data.size() - 1;
    StreamType streamType = streamManager->getStreamType(streamName);
    int payloadSize = dataMap[streamName].data[offset].second;
    bool isMarked = dataMap[streamName].isImportant;
    NetworkType networkType = getNetworkType();

    // fill the packet header
    auto header = makeShared<ShipPacket>();
    int rtpHeaderSize = B(header->getChunkLength()).get();
    header->setTransportWideSequenceNumber(streamManager->getPacketsNumber() + 1);
    header->setSequenceNumber(sequenceNumber);
    header->setStreamName(streamName.c_str());
    header->setStreamType(streamtype::toString(streamType).c_str());
    header->setElemNumber(dataMap[streamName].elemNumber);
    header->setFragmentOffset(offset);
    header->setIsLastFragment(isLast);
    header->setIsMarked(isMarked);
    header->setPayloadTimestamp(simTime());
    header->setPayloadSize(payloadSize);
    header->setNetworkType(static_cast<int>(networkType));
    header->setChunkLength(streamType != SIM ? B(rtpHeaderSize) : B(rtpHeaderSize + payloadSize));

    // fill the data
    if (streamType != SIM) {
        inet::Ptr<inet::Chunk> payload;
        if (dataMap[streamName].data[offset].first) {
            payload = makeShared<BytesChunk>(dataMap[streamName].data[offset].first,
                                             dataMap[streamName].data[offset].second);
        } else {
            // to get rid of "chunk is empty" exception by insertAtFront set it to 1 byte
            payload = makeShared<ByteCountChunk>(B(1));
        }
        // insert payload
        packet->insertAtFront(payload);
    }

    // insert header
    packet->insertAtFront(header);

    // send packet
    sendViaSocket(packet, networkType);

    // update package statistics
    streamManager->updatePacketSent();
    streamManager->updateWithPacket(streamName, header, PacketType::DATA);

    if (isLast && streamManager->getStreamIsRunning(streamName)) {
        // update stream info
        streamManager->updateElemSent(streamName,
                                      dataMap[streamName].elemSize,
                                      dataMap[streamName].popTimestamp - dataMap[streamName].captureTimestamp,
                                      dataMap[streamName].encodeTimestamp - dataMap[streamName].popTimestamp
                                     );
    }

    // add for possible retranmission, if maxAge = 0 nothing happens
    streamManager->getStreamRetranmissionBuffer(streamName).addPacket(packet->dup(), sequenceNumber, simTime());

    // check if time to send a FEC redundant packet
    if (isFec) {
        if (streamManager->isTimeToSendFec(streamName, fecAfterPackets, isMarked)) {
            Packet* fPacket = new Packet();
            auto fecPacket = makeShared<FecPacket>();
            fecPacket->setStreamName(streamName.c_str());
            fecPacket->setLastSequenceNumber(sequenceNumber);
            fecPacket->setFecCount(streamManager->getStreamDataTransmissionState(streamName).numFecPackets + 1);
            fecPacket->setFirstSequenceNumber(streamManager->getStreamFirstFecSequenceNumber(streamName));
            fecPacket->setLastSequenceNumber(sequenceNumber);
            fecPacket->setNetworkType(static_cast<int>(networkType));
            // assumed packets are homogenious, if not, arrange a temporal container to save packet sizes
            fecPacket->setChunkLength(
                B(fecPacket->getChunkLength()) + B(streamManager->getStreamMaxPacketSize(streamName)));
            fPacket->insertAtFront(fecPacket);
            fPacket->setName("fec");
            sendViaSocket(fPacket, networkType);

            streamManager->updateWithPacket(streamName, fecPacket, PacketType::FEC);
            streamManager->setNewFirstFecSequenceNumber(streamName, sequenceNumber + 1);
            if (streamManager->getStreamVerbose(streamName) > 1) {
                std::cout << "ShipApp::sendData send FEC packet for " << streamName << " of size "
                          << B(fecPacket->getChunkLength()) << " at " << simTime().dbl() << std::endl;
            }
        }
    }

    if (streamManager->getStreamVerbose(streamName) > 1) {
        std::cout << "ShipApp::sendData send packet for stream " << streamName
                  << " sim time " << simTime().dbl()
                  << " packet num: " << streamManager->getPacketsNumber()
                  << " size: " << B(payloadSize + rtpHeaderSize) << std::endl;
    }
}

void ShipApp::sendViaSocket(Packet* packet, NetworkType networkType) {
    if (!isMultiHome) {
        socket.sendTo(packet, destAddress, destPort);
    } else {
        // we are in multihome and socket is located in the cellular/wireless app
        cMessage* msg = dynamic_cast<cMessage*>(packet);
        if (!msg) {
            throw std::runtime_error("ShipApp::sendViaSocket: can't cast packet to cMessage");
        }
        if (networkType == NetworkType::LTE || networkType == NetworkType::NR) {
            // cellular
            sendDirect(msg, cellularApp, cellularApp->findGate("dataIn"));
        } else {
            // wireless
            sendDirect(msg, wirelessApp, wirelessApp->findGate("dataIn"));
        }
    }
}

void ShipApp::receivePacket(cMessage* msg) {
    Packet* packet = nullptr;
    packet = dynamic_cast<Packet*>(msg);
    if (!packet) {
        delete msg;
        return;
    }
    const std::string packetName = packet->getName();

    if (packetName == "ack") {
        const Ptr<const AckPacket>& ackPacket = packet->popAtFront<AckPacket>();
        std::string streamName = ackPacket->getStreamName();
        // update transmission state with ack packet
        streamManager->updateWithPacket(streamName, ackPacket, PacketType::ACK);
        if (streamManager->getStreamVerbose(streamName) > 1) {
            std::cout << "ShipApp::receivePacket received ACK/SACK packet for a stream " << streamName
                      << " seq number " << ackPacket->getSequenceNumber() << " with rtt (ms) "
                      << (simTime() - ackPacket->getSenderTimestamp()).dbl() * 1000 << std::endl;
        }
    } else if (packetName == "nack") {
        const Ptr<const NackPacket>& nackPacket = packet->popAtFront<NackPacket>();
        std::string streamName = nackPacket->getStreamName();
        NetworkType networkType = getNetworkType();
        int nackSn = nackPacket->getSequenceNumber();
        // update transmission state with nack packet (only increments a counter)
        streamManager->updateWithPacket(streamName, nackPacket, PacketType::NACK);
        // check if requested packet is in buffer, if yes -- send
        auto maybePacket = streamManager->getStreamRetranmissionBuffer(streamName).findPacket(nackSn);
        // form NACK decision
        bool decision = getNackDecision() && maybePacket.has_value();
        if (decision) {
            // important to duplicate to maintain ownership, original one in RTB will be deleted automatically
            Packet* rePacket = maybePacket.value()->dup();
            // add decision header
            auto nackDecision = makeShared<NackDecision>();
            nackDecision->setDecision(true);
            nackDecision->setTimestamp(simTime());
            rePacket->insertAtFront(nackDecision);
            rePacket->setName("data-nack");
            sendViaSocket(rePacket, networkType);
        } else {
            Packet* nackRefusalPacket = new Packet();
            auto nackDecision = makeShared<NackDecision>();
            nackDecision->setDecision(false);
            nackDecision->setReason("max age");
            nackDecision->setStreamName(streamName.c_str());
            nackDecision->setSequenceNumber(nackSn);
            nackDecision->setTimestamp(simTime());
            nackRefusalPacket->insertAtFront(nackDecision);
            nackRefusalPacket->setName("nackRefusal");
            sendViaSocket(nackRefusalPacket, networkType);
        }
        if (streamManager->getStreamVerbose(streamName) > 1) {
            std::cout << "ShipApp::receivePacket received NACK packet for a stream " << streamName
                      << " seq number " << nackSn << " and "
                      << (decision ? "tries to retransmit the packet" : "refuses to retransmit the packet")
                      << std::endl;
        }
    } else if (packetName == "receiverReport") {
        const Ptr<const ReceiverReportPacket>& rrPacket = packet->popAtFront<ReceiverReportPacket>();
        int rrNumber = rrPacket->getFeedbackCount();
        const cArray& reportBlocks = rrPacket->getReportBlocks();
        for (int i = 0; i < reportBlocks.size(); i++) {
            if (reportBlocks.exist(i)) {
                const ReportBlock* reportBlockObj = check_and_cast<const ReportBlock*>
                                                    (reportBlocks.get(i));
                const Ptr<const ReportBlock> reportBlock = makeShared<ReportBlock>
                        (*const_cast<ReportBlock*>(reportBlockObj));
                std::string streamName = reportBlock->getStreamName();
                streamManager->updateWithPacket(streamName, reportBlock, PacketType::RECEIVERREPORT, rrNumber);
                if (streamManager->getStreamVerbose(streamName) > 1) {
                    std::cout << "ShipApp::receivePacket received RR packet for a stream " << streamName
                              << " at " << simTime() << std::endl;
                }
            }
        }
    } else if (packetName == "transportFeedback") {
        const Ptr<const TransportFeedbackPacket>& tfPacket = packet->popAtFront<TransportFeedbackPacket>();
        int tfNumber = tfPacket->getFeedbackStatusCount();
        const cArray& feedbackChunks = tfPacket->getFeedbackChunks();
        for (int i = 0; i < feedbackChunks.size(); i++) {
            if (feedbackChunks.exist(i)) {
                const TransportFeedbackChunk* fcObj = check_and_cast<const TransportFeedbackChunk*>
                                                      (feedbackChunks.get(i));
                const Ptr<const TransportFeedbackChunk> feedbackChunk = makeShared<TransportFeedbackChunk>
                        (*const_cast<TransportFeedbackChunk*>(fcObj));
                std::string streamName = feedbackChunk->getStreamName();
                streamManager->updateWithPacket(streamName, feedbackChunk, PacketType::TRANSPORTFEEDBACK, tfNumber);
                if (streamManager->getStreamVerbose(streamName) > 1) {
                    std::cout << "ShipApp::receivePacket received TF chunk for a stream " << streamName
                              << " at " << simTime() << std::endl;
                }
            }
        }
    }
    delete packet;
}

bool ShipApp::getNackDecision() {
    // TODO: add some logic here, e.g. hybrid NACK/FEC algorithm based on RTT or so
    return true;
}

void ShipApp::checkNetworkInstability() {
    // check for instability if the last ack packet was too much time before (10 checks per state)
    streamManager->checkNetworkInstability();
    scheduleAt(simTime() + stateUpdatePeriod / 2, netInstabilityMsg);
}

void ShipApp::log() {
    int runNumber = cSimulation::getActiveEnvir()->getConfigEx()->getActiveRunNumber();
    streamManager->logStreams(runNumber, simTime().dbl());
}

NetworkType ShipApp::getNetworkType() {
    NetworkType nt;
    bool isNR = true;
    if (streamManager->getPacketsNumber() % 2 == 0) {
        // cellular
        nt = isNR ? NetworkType::NR : NetworkType::LTE;
    } else {
        // wlan
        nt = NetworkType::WIFI;
    }
    return nt;
}

// FIXME: unite a part of pushStates and the whole nonDrlStep under a single step method. Delete nonDrlStep afterwards
void ShipApp::pushStates() {
    log();
    if (gymCommunicator->getRequestNum() == 0) {
        // initial request with an initial state
        auto initialState = nlohmann::json::parse("{ \"initial\": true }");
        gymCommunicator->pushRequest(std::move(initialState));
        // FIXME: temporal patch to use with direct feedbacks reception, implement later different state update types
        scheduleAt(simTime() + stateUpdatePeriod + 0.01, stateMsg);
    } else {
        auto states = streamManager->getStates();
        for (auto& state : states) {
            // truncate state if it is the last one in this run
            if (simTime().dbl() + stateUpdatePeriod > finishTime.dbl()) {
                state["truncated"] = true;
            }
            gymCommunicator->pushRequest(std::move(state));
        }
        scheduleAt(simTime() + stateUpdatePeriod, stateMsg);
    }
}

void ShipApp::pullActions() {
    std::optional<nlohmann::json> reply = gymCommunicator->tryPullReply();
    if (!reply.has_value()) {
        // FIXME: add transcoding penalty + action application delay
        scheduleAt(simTime() + 0.01, actionMsg);
    } else {
        nlohmann::json actions = reply.value();
        applyActions(actions);
        scheduleAt(simTime() + 0.01, actionMsg);
    }
}

void ShipApp::applyActions(nlohmann::json& actionsJson) {
    if (actionsJson.contains("reset") || actionsJson.contains("finish")) {
        endSimulation();
    } else {
        // env sent a list of action(s) for stream(s)
        auto actionsList = actionsJson.at("actions");
        for (int i = 0; i < (int)actionsList.size(); i++) {
            std::string streamName = streamManager->getStreamNameById(i);
            auto action = actionsList[i];
            if (isUsingFutureData && !futureDataMap[streamName].empty())
                // to ensure there is no unfinished tasks before potential encoder reinit
                futureDataMap[streamName].front().wait();
            if (action.is_number_integer()) {
                streamManager->applyAction(streamName, (int)action);
            } else {
                streamManager->applyAction(streamName, (double)action);
            }
            if (isUsingFutureData) {
                futureDataMap[streamName].pop();
                getFutureStreamData(streamName, 1);
            }
        }
    }
}

void ShipApp::nonDrlStep() {
    // for non-drl AS
    log();
    if (algorithm == AdaptiveAlgorithm::NO) {
        // do only logging here
        scheduleAt(simTime() + logUpdatePeriod, stateMsg);
    } else {
        auto states = streamManager->getStates();
        for (auto& state : states) {
            std::string streamName = state["streamName"];
            if (isUsingFutureData && !futureDataMap[streamName].empty())
                // to ensure there is no unfinished tasks before potential encoder reinit
                futureDataMap[streamName].front().wait();
            if (algorithm == AdaptiveAlgorithm::ABR_BASE) {
                int nextAction = state["action"];
                streamManager->applyAction(streamName, nextAction);
            } else if (algorithm == AdaptiveAlgorithm::ABR_GCC) {
                double nextAction = state["action"];
                streamManager->applyAction(streamName, nextAction);
            }
            if (isUsingFutureData) {
                futureDataMap[streamName].pop();
                getFutureStreamData(streamName, 1);
            }
        }
        scheduleAt(simTime() + stateUpdatePeriod, stateMsg);
    }
}

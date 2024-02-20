#include "ShoreApp.h"

Define_Module(ShoreApp);

ShoreApp::ShoreApp(): receiverReportMsg(nullptr), transportFeedbackMsg(nullptr) {}

ShoreApp::~ShoreApp() {
    cancelAndDelete(receiverReportMsg);
    cancelAndDelete(transportFeedbackMsg);
}

void ShoreApp::initialize(int stage) {
    cSimpleModule::initialize(stage);
    if (stage != inet::INITSTAGE_APPLICATION_LAYER)
        return;
    initTime = simTime();

    // for decoders
    isView = par("isView").boolValue();
    isDecode = isView ? true : par("isDecode").boolValue();

    // rtc
    isAck = par("isAck");
    isSack = par("isSack");

    isNack = par("isNack");
    maxNacks = isNack ? par("maxNacks").intValue() : 0;
    maxNackDelay = isNack ? par("maxNackDelay").doubleValue() : 0.0;
    maxPlayoutDelay = par("maxPlayoutDelay").doubleValue();
    nackCount = 0;

    isFec = par("isFec");
    repairPeriod = par("repairPeriod");

    isFir = par("isFir");

    isDirectReceiverReport = par("isDirectReceiverReport");
    isDirectTransportFeedback = par("isDirectTransportFeedback");

    // verbosity
    verbose = par("verbose").intValue();
    if (verbose == 0) {
        av_log_set_level(AV_LOG_QUIET);
    } else if (verbose == 1) {
        av_log_set_level(AV_LOG_ERROR);
    } else {
        av_log_set_level(AV_LOG_INFO);
    }

    initTraffic();
}

void ShoreApp::finish() {
    int rxPackets = 0;
    for (auto& sr : streamReceivers) {
        rxPackets += sr.second->streamReporter.totalPacketsReceived;
        delete sr.second;
    }
    streamReceivers.clear();
    std::cout << "ShoreApp::finish has overall " << rxPackets << " packets received " << std::endl;
    std::cout << "ShoreApp::finish closing ShoreApp..." << std::endl;
}

void ShoreApp::handleMessage(cMessage* msg) {
    if (msg->isSelfMessage()) {
        if (!strcmp(msg->getName(), "reinit")) {
            // reinit traffic
            delete msg;
            initTraffic();
        } else if (!strcmp(msg->getName(), "nack")) {
            sendNackIfNeeded(msg);
        } else if (!strcmp(msg->getName(), "receiverReport")) {
            sendReceiverReport();
        } else if (!strcmp(msg->getName(), "transportFeedback")) {
            sendTransportFeedback();
        } else if (!strcmp(msg->getName(), "repair")) {
            tryRepairPacket(msg);
        } else if (!strcmp(msg->getName(), "play")) {
            std::string streamName = msg->par("streamName").stringValue();
            int sequenceNumber = (int)msg->par("sequenceNumber").longValue();
            resumePlay(streamName, sequenceNumber);
        }
    } else {
        // from ship
        if (!strncmp(msg->getName(), "data", 4)) {
            receiveDataPacket(msg);
        } else if (!strcmp(msg->getName(), "senderReport")) {
            receiveReport(msg);
        } else if (!strcmp(msg->getName(), "nackRefusal")) {
            receiveNackRefusal(msg);
        } else if (!strcmp(msg->getName(), "fec")) {
            receiveFecPacket(msg);
        }
    }
}

void ShoreApp::initTraffic() {
    cModule* destModule = findModuleByPath(par("destAddress").stringValue());
    if (destModule == nullptr) {
        // check if a ship is also initialized
        if ((simTime() - initTime).dbl() > par("maxWaitingTimeToStart").doubleValue()) {
            throw std::runtime_error("ShoreApp::initTraffic ship module hasn't been found, stopping...");
        }
        double offset = uniform(0.01, 0.05);
        scheduleAt(simTime() + offset, new cMessage("reinit"));
        std::cout << "ShoreApp::initTraffic the node will retry to initialize traffic in "
                  << offset << " seconds " << std::endl;
    } else {
        destAddress = L3AddressResolver().resolve(par("destAddress"));
        destPort = par("destPort");
        shipApp = destModule->getSubmodule("app", 0);
        if (!shipApp) {
            throw std::runtime_error("ShoreApp::initTraffic ship app hasn't been found, stopping...");
        }

        // UDP socket: receive datagrams with stream data
        localPortData = par("localPortData");
        socket.setOutputGate(gate("socketOut"));
        socket.bind(localPortData);
        int tos = par("tos");
        if (tos != -1)
            socket.setTos(tos);
        std::cout << "ShoreApp::initTraffic UDP socket is binded to local port " << localPortData
                  << " and sends datagrams to: " << destAddress << ":" << std::to_string(destPort) << std::endl;

        initScheduler();

        std::cout << "ShoreApp::initTraffic start data sending! ... " << std::endl;
    }
}

void ShoreApp::initScheduler() {
    startTime = simTime() + par("warmUp");

    receiverReportPeriod = par("receiverReportPeriod");
    transportFeedbackPeriod = par("transportFeedbackPeriod");

    receiverReportMsg = new cMessage("receiverReport");
    transportFeedbackMsg = new cMessage("transportFeedback");

    if (receiverReportPeriod > 0) {
        receiverReportCount = 0;
        scheduleAt(startTime + receiverReportPeriod, receiverReportMsg);
    }

    if (transportFeedbackPeriod > 0 && !isAck) {
        transportFeedbackCount = 0;
        firstTwSequenceNumber = 0;
        isNewTf = true;
        scheduleAt(startTime + transportFeedbackPeriod, transportFeedbackMsg);
    } else {
        isNewTf = false; // count and number won't be ever updated
    }
}

void ShoreApp::receiveDataPacket(cMessage* msg) {
    Packet* packet = check_and_cast<Packet*>(msg);
    if (!packet) {
        return;
    }

    // data packet types: 1) normal RTP packet, 2) retransmitted RTP packet, 3) Redundant RTP packet
    const char* packetName = packet->getName();
    bool isRetransmitted = strstr(packetName, "nack");
    simtime_t retransmissionTimestamp = 0.0;
    if (isRetransmitted) {
        // pop additional NackDecision header
        const auto& nackDecision = packet->popAtFront<NackDecision>();
        retransmissionTimestamp = nackDecision->getTimestamp();
    }

    // duplicate data packet to put it later into the playback buffer if needed
    Packet* playbackPacket = packet->dup();

    // process incoming data packet: get header and its metadata
    const auto& header = packet->popAtFront<ShipPacket>();
    std::string streamName = std::string(header->getStreamName());
    std::string streamType = std::string(header->getStreamType());
    int sequenceNumberTw = header->getTransportWideSequenceNumber();
    int sequenceNumber = header->getSequenceNumber();
    int payloadSize = header->getPayloadSize();
    omnetpp::simtime_t payloadTimestamp = header->getPayloadTimestamp();
    if (verbose > 1) {
        std::cout << "ShoreApp::receiveData received data packet: stream: " << streamName
                  << " seq num: " << sequenceNumber << " size: " << payloadSize << std::endl;
    }

    // create stream receiver if not created
    if (streamReceivers.count(streamName) == 0) {
        streamReceivers.emplace(streamName, new StreamReceiver(streamName, streamtype::fromString(streamType), verbose));
    }
    StreamReceiver* streamReceiver = streamReceivers[streamName];
    LostPacketsMap& lostPacketsMap = streamReceiver->getLostPacketsMap();

    // update transport-wide flag if needed
    if (isNewTf) {
        firstTwSequenceNumber = sequenceNumberTw;
        isNewTf = false;
    }

    // update RTCP statistics
    if (!isRetransmitted) {
        // normal RTP packet
        streamReceiver->streamReporter.processRtpPacket(sequenceNumber, payloadSize, payloadTimestamp, simTime());
        // update repair buffer if needed
        if (isFec) {
            streamReceiver->addSequenceNumberToRepairBuffer(sequenceNumber);
        }

        // check & manage lost packets
        std::vector<int>& lostSequenceNumbers = streamReceiver->streamReporter.getLostSequenceNumbers();
        if (!lostSequenceNumbers.empty() && (isNack || isFec || isFir)) {
            for (int lostSequenceNumber : lostSequenceNumbers) {
                // add new lost packet
                streamReceiver->putIntoLostPacketsMap(lostSequenceNumber, simTime(), isFec, isNack);
                // schedule message for resuming play
                scheduleAt(simTime() + maxPlayoutDelay, lostPacketsMap[lostSequenceNumber].playMsg);
                if (isFec) {
                    // schedule repair message for FEC
                    scheduleAt(simTime() + repairPeriod, lostPacketsMap[lostSequenceNumber].repairMsg);
                }
                if (isNack) {
                    // schedule message for NACK management
                    scheduleAt(simTime(), lostPacketsMap[lostSequenceNumber].nackMsg);
                }
            }
            streamReceiver->updateNextLostSequenceNumber(streamReceiver->getNextLostSequenceNumber());
        }

    } else {
        if (lostPacketsMap.find(sequenceNumber) != lostPacketsMap.end()) {
            // retransmitted RTP packet, in case this is not hit it has arrived too late and considered lost
            removeFromLost(streamName, sequenceNumber);
            streamReceiver->streamReporter.processRetransmittedRtpPacket(
                sequenceNumber, payloadSize, payloadTimestamp, retransmissionTimestamp, simTime());
        }
    }

    // send ack/sack if needed
    if (isAck || (isSack && header->isMarked())) {
        sendAck(header);
    }

    // check if it is not too late to play
    int maxPlayedSequenceNumber = streamReceiver->getMaxPlayedSequenceNumber();
    if (maxPlayedSequenceNumber > 0 && sequenceNumber <= maxPlayedSequenceNumber) {
        // this packet comes too late, it could be a retransmitted packet or just very delayed ooo
        delete playbackPacket;
        delete packet;
    } else {
        if (!isNack && !isFir && !isFec) {
            // if no retransmission or redundancy at all (pure UDP), then no PB, immediately play
            playData(header, packet, 0);
            delete playbackPacket;
        } else {
            // add to the playback buffer
            streamReceiver->pushIntoPlaybackBuffer(playbackPacket, sequenceNumber, simTime());
            // try to play
            play(streamName);
            delete packet;
        }
    }
}

void ShoreApp::receiveFecPacket(cMessage* msg) {
    Packet* packet = check_and_cast<Packet*>(msg);
    if (!packet) {
        return;
    }
    if (!isFec) {
        isFec = true;
    }
    const auto& header = packet->popAtFront<FecPacket>();
    std::string streamName = header->getStreamName();
    int payloadSize = B(header->getChunkLength()).get() - 10;

    if (streamReceivers.find(streamName) != streamReceivers.end()) {
        streamReceivers[streamName]->streamReporter.processFecPacket(payloadSize);
        streamReceivers[streamName]->addRepairSequence(
            header->getFirstSequenceNumber(), header->getLastSequenceNumber());
    }

    delete packet;
}

void ShoreApp::receiveReport(cMessage* msg) {
    Packet* packet = check_and_cast<Packet*>(msg);
    if (!packet) {
        return;
    }
    const auto& header = packet->popAtFront<SenderReportPacket>();
    std::string streamName = header->getStreamName();

    if (streamReceivers.find(streamName) != streamReceivers.end()) {
        streamReceivers[streamName]->streamReporter.processSenderReport(header->getTimestamp());
    }

    delete packet;
}

void ShoreApp::receiveNackRefusal(cMessage* msg) {
    Packet* packet = check_and_cast<Packet*>(msg);
    if (!packet) {
        return;
    }
    const auto& header = packet->popAtFront<NackDecision>();
    std::string streamName = header->getStreamName();
    int sequenceNumber = header->getSequenceNumber();
    resumePlay(streamName, sequenceNumber);
    delete packet;
}

void ShoreApp::sendAck(const inet::Ptr<const ShipPacket>& rtpHeader) {
    // TODO: so far is almost assumed to be turned off
    Packet* ackPacket = new Packet("ack");
    auto ackHeader = inet::makeShared<AckPacket>();
    ackHeader->setStreamName(rtpHeader->getStreamName());
    ackHeader->setSequenceNumber(rtpHeader->getSequenceNumber());
    ackHeader->setPayloadSize(rtpHeader->getPayloadSize());
    ackHeader->setSenderTimestamp(rtpHeader->getPayloadTimestamp());
    ackHeader->setArrivalTimestamp(simTime());
    ackPacket->insertAtFront(ackHeader);
    socket.sendTo(ackPacket, destAddress, destPort);
    if (verbose > 1) {
        std::cout << "ShoreApp::sendAckIfNeeded ACK/SACK is sent for the stream " << rtpHeader->getStreamName()
                  << " seq number " << rtpHeader->getSequenceNumber() << std::endl;
    }
}

void ShoreApp::sendNackIfNeeded(cMessage* nackScheduledMsg) {
    if (isNack) {
        std::string streamName = nackScheduledMsg->par("streamName").stringValue();
        int sequenceNumber = (int)nackScheduledMsg->par("sequenceNumber").longValue();
        LostPacketsMap& lostPacketsMap = streamReceivers[streamName]->getLostPacketsMap();
        NackInfo& nackInfo = lostPacketsMap[sequenceNumber].nackInfo;
        if (nackInfo.nacksSent + 1 > maxNacks) {
            // stops if reached the limit of max nacks
            resumePlay(streamName, sequenceNumber);
            return;
        } else {
            // send next nack
            nackInfo.nacksSent++;
            nackInfo.lastNackTimestamp = simTime();
            Packet* nackPacket = new Packet("nack");
            auto nackHeader = inet::makeShared<NackPacket>();
            nackHeader->setStreamName(streamName.c_str());
            nackHeader->setSequenceNumber(sequenceNumber);
            nackPacket->insertAtFront(nackHeader);
            socket.sendTo(nackPacket, destAddress, destPort);
            scheduleAt(simTime() +  maxNackDelay, nackScheduledMsg);
            nackCount++;
            if (verbose > 1) {
                std::cout << "ShoreApp::sendNackIfNeeded " << nackInfo.nacksSent
                          << " NACK is sent for the stream " << streamName
                          << " seq number " << sequenceNumber << std::endl;
            }
        }
    }
}

void ShoreApp::sendReceiverReport() {
    Packet* rrPacket = new Packet("receiverReport");
    auto rrHeader = inet::makeShared<ReceiverReportPacket>();
    rrHeader->setFeedbackCount(++receiverReportCount);
    // fill Report Blocks for all streams
    for (const auto& streamReceiverPair : streamReceivers) {
        StreamReporter& streamReporter = streamReceiverPair.second->streamReporter;
        streamReporter.estimateFractionStats();
        ReportBlock* reportBlock = new ReportBlock();
        reportBlock->setStreamName(streamReceiverPair.first.c_str());
        reportBlock->setFractionLost(streamReporter.fractionLost);
        reportBlock->setPacketsLostCumulative(streamReporter.totalPacketsLost);
        reportBlock->setMaxSequenceNumber(streamReporter.maxSequenceNumber);
        reportBlock->setJitter(streamReporter.jitter);
        reportBlock->setLastSR(!isDirectReceiverReport ? streamReporter.lastSenderReportSentTimestamp : 0.0);
        reportBlock->setDelaySinceLastSR(!isDirectReceiverReport ? simTime() - streamReporter.lastSenderReportArrivalTimestamp :
                                         0.0);
        reportBlock->setFractionRate(streamReporter.fractionRate);
        reportBlock->setFractionFecRate(streamReporter.fractionFecRate);
        reportBlock->setPacketsReceivedCumulative(streamReporter.totalPacketsReceived);
        reportBlock->setPacketsOutOfOrderCumulative(streamReporter.totalPacketsOutOfOrder);
        reportBlock->setPacketsPlayedCumulative(streamReporter.totalPacketsPlayed);
        reportBlock->setPacketsRetransmittedCumulative(streamReporter.totalPacketsRetransmitted);
        reportBlock->setPacketsRepairedCumulative(streamReporter.totalPacketsRepaired);
        reportBlock->setPacketsRepairedAndRetransmittedCumulative(streamReporter.totalPacketsRepairedAndRetransmitted);
        reportBlock->setBytesReceivedCumulative(streamReporter.totalBytesReceived);
        reportBlock->setFractionInterarrivalDelayMean(streamReporter.interarrivalDelay.getMeanCached(receiverReportPeriod));
        reportBlock->setFractionTransmissionDelayMean(streamReporter.transmissionDelay.getMeanCached(receiverReportPeriod));
        reportBlock->setFractionRetransmissionDelayMean(streamReporter.retransmissionDelay.getMeanCached(0.0));
        reportBlock->setFractionPlayoutDelayMean(streamReporter.playoutDelay.getMeanCached(maxPlayoutDelay));
        reportBlock->setFractionStallingRate(streamReporter.fractionStallingRate);
        reportBlock->setBandwidth(streamReporter.bandwidth);
        rrHeader->addReportBlock(reportBlock);
        streamReporter.eraseCachedStatistic();
    }

    rrPacket->insertAtFront(rrHeader);

    if (isDirectReceiverReport) {
        sendDirect(rrPacket, shipApp, shipApp->findGate("directIn"));
    } else {
        socket.sendTo(rrPacket, destAddress, destPort);
    }

    if (verbose > 1) {
        std::cout << "ShoreApp::sendReceiverReport Receiver Report num " << rrHeader->getFeedbackCount()
                  << "of size " << rrPacket->getByteLength() << " is sent at " << simTime() << std::endl;
    }

    scheduleAt(simTime() + receiverReportPeriod, receiverReportMsg);
}

void ShoreApp::sendTransportFeedback() {
    Packet* tfPacket = new Packet("transportFeedback");
    auto tfHeader = inet::makeShared<TransportFeedbackPacket>();
    tfHeader->setBaseSequenceNumber(firstTwSequenceNumber);
    tfHeader->setFeedbackStatusCount(++transportFeedbackCount);
    int packetStatusCount = 0;
    for (const auto& streamReceiverPair : streamReceivers) {
        StreamReporter& streamReporter = streamReceiverPair.second->streamReporter;
        // fill Transport Feedback chunk
        assert(streamReporter.sequenceNumbersSinceLastTF.size() == streamReporter.deltasSinceLastTF.size());
        for (int i = 0; i < (int)streamReporter.sequenceNumbersSinceLastTF.size(); ++i) {
            // fill Transport Feedback Chunk (for simplicity 1 chunk = 1 packet)
            TransportFeedbackChunk* chunk = new TransportFeedbackChunk();
            chunk->setStreamName(streamReceiverPair.first.c_str());
            chunk->setSequenceNumber(streamReporter.sequenceNumbersSinceLastTF[i]);
            chunk->setRecvDelta(streamReporter.deltasSinceLastTF[i]);
            tfHeader->addFeedbackChunk(chunk);
            ++packetStatusCount;
        }
        streamReporter.eraseTFVectors();
    }

    tfHeader->setPacketStatusCount(packetStatusCount);
    tfPacket->insertAtFront(tfHeader);

    if (isDirectTransportFeedback) {
        sendDirect(tfPacket, shipApp, shipApp->findGate("directIn"));
    } else {
        socket.sendTo(tfPacket, destAddress, destPort);
    }

    isNewTf = true;
    if (verbose > 1) {
        std::cout << "ShoreApp::sendTransportFeedback Transport Feedback num " << transportFeedbackCount
                  << "of size " << tfPacket->getByteLength() << " is sent at " << simTime() << std::endl;
    }

    scheduleAt(simTime() + transportFeedbackPeriod, transportFeedbackMsg);
}

void ShoreApp::tryRepairPacket(cMessage* msg) {
    std::string streamName = msg->par("streamName").stringValue();
    int lostSequenceNumber = (int)msg->par("sequenceNumber").longValue();
    StreamReceiver* streamReceiver = streamReceivers[streamName];
    RepairSequence* repairSequence = streamReceiver->findRepairSequence(lostSequenceNumber);
    if (repairSequence) {
        RepairStatus status = streamReceiver->getRepairStatus(lostSequenceNumber, repairSequence);

        if (verbose > 1) {
            std::cout << "ShoreApp::tryRepairPacket Tried to repair packet " << lostSequenceNumber
                      << " status " << static_cast<int>(status) << std::endl;
        }

        switch(status) {
            case RepairStatus::REPAIRABLE:
                streamReceiver->streamReporter.reportRepairedPacket(lostSequenceNumber,
                        streamReceiver->getRepairedAndRetransmittedPackets());
                removeFromLost(streamName, lostSequenceNumber);
                streamReceiver->pushIntoPlaybackBuffer(new Packet(), lostSequenceNumber, simTime());
                return;
            case RepairStatus::MAYBE_REPAIRABLE:
            case RepairStatus::UNREPAIRABLE:
                break;
            default:
                break;
        }
    }
    scheduleAt(simTime() + repairPeriod, msg);
}

void ShoreApp::removeFromLost(std::string& streamName, int sequenceNumber) {
    StreamReceiver* streamReceiver = streamReceivers[streamName];
    LostPacketsMap& lostPacketsMap = streamReceivers[streamName]->getLostPacketsMap();
    cancelAndDelete(lostPacketsMap[sequenceNumber].playMsg);
    if (isFec) {
        cancelAndDelete(lostPacketsMap[sequenceNumber].repairMsg);
    }
    if (isNack) {
        cancelAndDelete(lostPacketsMap[sequenceNumber].nackMsg);
    }
    streamReceiver->eraseAndUpdateNextLostSequenceNumber(sequenceNumber);
}

void ShoreApp::resumePlay(std::string& streamName, int sequenceNumber) {
    LostPacketsMap& lostPacketsMap = streamReceivers[streamName]->getLostPacketsMap();
    if (lostPacketsMap.find(sequenceNumber) != lostPacketsMap.end()) {
        // NOTE: lost packet is reported as absolutely lost only here
        streamReceivers[streamName]->streamReporter.reportLostPacket();
        removeFromLost(streamName, sequenceNumber);
        play(streamName);
    }
    if (verbose > 1) {
        std::cout << "ShoreApp::resumePlay Resumed playing from "
                  << sequenceNumber << " at " << simTime() << std::endl;
    }
}

void ShoreApp::play(std::string& streamName) {
    StreamReceiver* streamReceiver = streamReceivers[streamName];
    while (true) {
        std::optional<const PacketEntry> packetEntryOpt = streamReceiver->getNextFromPlaybackBuffer();
        if (packetEntryOpt.has_value()) {
            const PacketEntry& packetEntry = packetEntryOpt.value();
            // it is a dummy packet inserted after a FEC reparation
            if (packetEntry.packet->getTotalLength().get() == 0) {
                streamReceiver->streamReporter.reportPlayedPacket(
                    (simTime() - packetEntry.timestamp).dbl());
                if (verbose > 1) {
                    std::cout << "ShoreApp::playData played packet " << packetEntry.sequenceNumber
                              << " for stream " << streamName << " at " << simTime() << std::endl;
                }
                delete packetEntry.packet;
                continue;
            }
            // normal packet
            Packet* packet = const_cast<Packet*>(packetEntry.packet);
            const auto& header = packet->popAtFront<ShipPacket>();
            playData(header, packet, packetEntry.timestamp);
        } else {
            break;
        }
    }
}

void ShoreApp::playData(const inet::Ptr<const ShipPacket>& header, inet::Packet* data, simtime_t playbackTimestamp) {
    std::string streamName = header->getStreamName();
    int sequenceNumber = header->getSequenceNumber();
    int elemNumber = header->getElemNumber();
    const char* streamType = header->getStreamType();
    StreamReceiver* streamReceiver = streamReceivers[streamName];

    // if a ping message or decoding is turned off or SIM stream -- ignore data processing
    if (elemNumber < 0 || !isDecode || streamtype::fromString(streamType) == SIM) {
        streamReceiver->streamReporter.reportPlayedPacket((simTime() - playbackTimestamp).dbl());
        if (verbose > 1) {
            std::cout << "ShoreApp::playData played packet " << sequenceNumber
                      << " for stream " << streamName << " at " << simTime() << std::endl;
        }
        delete data;
        return;
    }

    // DECODING
    // check if previous elements should be force decoded despite their last chunk has not been received
    // NOTE: here is used a static and conservative version of jitter buffer: it waits for the last chunk or next element
    streamReceiver->forceDecodePreviousElements(elemNumber);

    // save as chunk to process further, packetData should be free from the header
    // NOTE: argument inet::Packet* data assumed with already extracted header
    streamReceiver->addChunk(
        elemNumber,
        header->getPayloadSize(),
        header->getFragmentOffset(),
        simTime().dbl(),
        data
    );

    // packet decoding
    if (header->isLastFragment()) {
        streamReceiver->decode(elemNumber);
        if (isView) {
            streamReceiver->view();
        }
    }

    // report
    streamReceiver->streamReporter.reportPlayedPacket((simTime() - playbackTimestamp).dbl());

    if (verbose > 1) {
        std::cout << "ShoreApp::playData played packet " << sequenceNumber
                  << " for stream " << streamName << " at " << simTime() << std::endl;
    }
    delete data;
}

// void ShoreApp::subscribeToSignals() {
//     auto* pingApp = findModuleByPath("shore.app[1]");
//     if (!pingApp) {
//         std::cout << "ShoreApp::subscribeToSignals can't find PingApp, signals won't be subscribed, exiting..."
//                   << std::endl;
//         return;
//     }
//     // see inet.applications.pingapp.PingApp.ned
//     // tx
//     SignalListener* txSl = new SignalListener();
//     pingApp->subscribe("pingTxSeq", txSl);
//     listeners.insert( { "pingTxSeq", txSl } );
//     // lost
//     SignalListener* lostSl = new SignalListener();
//     pingApp->subscribe("numLost", lostSl);
//     listeners.insert( { "numLost", lostSl } );
//     // ooo
//     SignalListener* oooSl = new SignalListener();
//     pingApp->subscribe("numOutOfOrderArrivals", oooSl);
//     listeners.insert( { "numOutOfOrderArrivals", oooSl } );
//     // rtt
//     SignalListener* rttSl = new SignalListener();
//     pingApp->subscribe("rtt", rttSl);
//     listeners.insert( { "rtt", rttSl } );

//     std::cout << "ShoreApp::subscribeToSignals pingApp signals are subscribed to! ..." << std::endl;
// }

// void ShoreApp::updatePingAppStats() {
//     pingAppStats.txPackets = listeners["pingTxSeq"]->getIntValue();
//     pingAppStats.lostPackets = listeners["numLost"]->getIntValue();
//     pingAppStats.oooPackets = listeners["numOutOfOrderArrivals"]->getIntValue();
//     pingAppStats.rxPackets = pingAppStats.txPackets - pingAppStats.lostPackets;
//     pingAppStats.rttSum += listeners["rtt"]->getDoubleValue() * 1000.0;
//     pingAppStats.rttAv = listeners["rtt"]->getAvgValue() * 1000.0;
//     pingAppStats.rttMin = listeners["rtt"]->getMinValue() * 1000.0;
//     pingAppStats.rttMax = listeners["rtt"]->getMaxValue() * 1000.0;
//     scheduleAt(simTime() + 0.01, sgnlMsg);
// }

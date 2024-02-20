#include "ForwardShoreApp.h"

Define_Module(ForwardShoreApp);

ForwardShoreApp::ForwardShoreApp() : logUpdateMsg(nullptr) {}

ForwardShoreApp::~ForwardShoreApp() {
    cancelAndDelete(logUpdateMsg);
}

void ForwardShoreApp::initialize(int stage) {
    UdpSink::initialize(stage);
    if (stage != inet::INITSTAGE_APPLICATION_LAYER)
        return;
    packetsRcvd = 0;
    logdir = par("logdir").stringValue();
    logUpdatePeriod = par("logUpdatePeriod").doubleValue();
    logUpdateMsg = new cMessage("log");
    verbose = par("verbose").boolValue();
    log(); // generate headers
}

void ForwardShoreApp::finish() {
    for (auto& sr : streamReceivers) {
        if (sr.second) delete sr.second;
    }
    for (auto& l : loggers) {
        if (l.second) delete l.second;
    }
}

void ForwardShoreApp::handleMessageWhenUp(cMessage* msg) {
    if (!strcmp(msg->getName(), "log")) {
        log();
    } else {
        UdpSink::handleMessageWhenUp(msg);
    }
}

void ForwardShoreApp::socketDataArrived(UdpSocket* socket, Packet* packet) {
    if (!packet) {
        return;
    }

    if (!strcmp(packet->getName(), "senderReport")) {
        // don't receive sender reports here
        delete packet;
        return;
    }

    if (packetsRcvd == 0) {
        initTime = simTime();
    }
    packetsRcvd++;

    const auto& header = packet->popAtFront<ShipPacket>();
    lastSequenceNumber = std::max(header->getTransportWideSequenceNumber(), lastSequenceNumber);
    std::string streamName = std::string(header->getStreamName());
    std::string streamType = std::string(header->getStreamType());
    int sequenceNumber = header->getSequenceNumber();
    int payloadSize = header->getPayloadSize();
    omnetpp::simtime_t payloadTimestamp = header->getPayloadTimestamp();
    NetworkType networkType = static_cast<NetworkType>(header->getNetworkType());

    if (streamReceivers.count(streamName) == 0) {
        streamReceivers.emplace(streamName, new StreamReceiver(streamName, streamtype::fromString(streamType), 0));
        if (logdir != "null") {
            std::string logfile;
            if (logdir.empty() || logdir == ".") {
                logfile = "stream_" + streamName + "_";
            } else {
                logfile = logdir + "stream_" + streamName + ".csv";
            }
            loggers.emplace(streamName, new CSVLogger(logfile));
        }
        networkTypes.emplace(streamName, std::vector<NetworkType>());
    }
    StreamReceiver* streamReceiver = streamReceivers[streamName];
    streamReceiver->streamReporter.processRtpPacket(sequenceNumber, payloadSize, payloadTimestamp, simTime());

    std::vector<int>& lostSequenceNumbers = streamReceiver->streamReporter.getLostSequenceNumbers();
    if (!lostSequenceNumbers.empty()) {
        for (int lostSequenceNumber : lostSequenceNumbers) {
            streamReceivers[streamName]->streamReporter.reportLostPacket();
        }
    } else {
        if (sequenceNumber < streamReceivers[streamName]->streamReporter.maxSequenceNumber) {
            // ooo packet but in this case we need to decrement the amount of lost packets
            streamReceivers[streamName]->streamReporter.totalPacketsLost--;
        }
    }

    int maxPlayedSequenceNumber = streamReceiver->getMaxPlayedSequenceNumber();
    if (maxPlayedSequenceNumber > 0 && sequenceNumber > maxPlayedSequenceNumber) {
        streamReceiver->streamReporter.reportPlayedPacket(simTime().dbl());
    }

    networkTypes[streamName].push_back(networkType);

    if (verbose) {
        std::cout << "ForwardShoreApp::socketDataArrived: streamName=" << streamName
                  << ", sequenceNumber=" << sequenceNumber << ", payloadSize=" << payloadSize
                  << ", delay=" << simTime() - payloadTimestamp << ", time=" << simTime()
                  << ", networkType=" << networktype::toString(networkType) << std::endl;
    }

    delete packet;
}

void ForwardShoreApp::log() {
    for (auto& sr : streamReceivers) {
        std::string streamName = sr.first;
        StreamReceiver* streamReceiver = sr.second;
        StreamReporter& streamReporter = streamReceiver->streamReporter;
        CSVLogger* logger = loggers.at(streamName);
        if (!logger->getIsHeaderWritten()) {
            *logger <<
                    "run" <<
                    "time" <<
                    "duration" <<
                    "fractionNetworkTypes" <<
                    "fractionNetworkTypeDist" <<
                    "fractionRxRate" <<
                    "rxPackets" <<
                    "rxMBytes" <<
                    "lostPackets" <<
                    "lossRate" <<
                    "fractionLossRate" <<
                    "rtt" <<
                    "interarrivalJitter" <<
                    CSVLogger::endl;
        } else {
            std::pair<std::string, std::string> nts = analyseNetworkTypes(streamName);
            streamReporter.estimateFractionStats();
            *logger <<
                    cSimulation::getActiveEnvir()->getConfigEx()->getActiveRunNumber() <<
                    simTime().dbl() <<
                    (simTime() - initTime).dbl() <<
                    nts.first <<
                    nts.second <<
                    streamReporter.fractionRate <<
                    streamReporter.totalPacketsReceived <<
                    streamReporter.totalBytesReceived * 1e-6 <<
                    streamReporter.totalPacketsLost <<
                    static_cast<double>(streamReporter.totalPacketsLost) / (streamReporter.totalPacketsReceived +
                            streamReporter.totalPacketsLost) <<
                    streamReporter.fractionLost <<
                    streamReporter.transmissionDelay.getMeanCached(logUpdatePeriod) * 1000 * 2 <<
                    streamReporter.jitter * 1000 <<
                    CSVLogger::endl;
            streamReporter.eraseCachedStatistic();
        }
    }
    scheduleAt(simTime() + logUpdatePeriod, logUpdateMsg);
}

std::pair<std::string, std::string> ForwardShoreApp::analyseNetworkTypes(std::string& streamName) {
    std::unordered_map<NetworkType, int> countMap;
    std::vector<NetworkType>& ntVector = networkTypes[streamName];
    for (NetworkType value : ntVector) {
        countMap[value]++;
    }

    std::string names;
    std::string rates;
    int totalElements = ntVector.size();
    for (const auto& entry : countMap) {
        names += networktype::toString(entry.first) + ",";
        rates += std::to_string(static_cast<double>(entry.second) / totalElements) + ",";
    }
    names.pop_back();
    rates.pop_back();
    return { names, rates };
}

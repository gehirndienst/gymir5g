#include "ForwardUdpApp.h"

Define_Module(ForwardUdpApp);

void ForwardUdpApp::initialize(int stage) {
    cSimpleModule::initialize(stage);
    if (stage != INITSTAGE_APPLICATION_LAYER)
        return;

    initTime = simTime();
    packetName = par("packetName");

    // find multihome app
    multiHomeApp = findModuleByPath(par("multiHomeAppPath"));
    if (!multiHomeApp) {
        throw cRuntimeError("[ForwardUdpApp::initialize]: cant't find multiHome app, aborting...");
    }

    initTraffic();
}

void ForwardUdpApp::initTraffic() {
    cModule* destModule = findModuleByPath(par("destAddress").stringValue());
    if (!destModule) {
        // check if control station (shore) is also initialized
        if ((simTime() - initTime).dbl() > par("maxWaitingTimeToStart").doubleValue()) {
            throw std::runtime_error("ForwardUdpApp::initTraffic dest app hasn't been found, stopping...");
        }
        double offset = uniform(0.01, 0.05);
        scheduleAt(simTime() + offset, new cMessage("reinit"));
        std::cout << "ForwardUdpApp::initTraffic the node will retry to initialize traffic in "
                  << offset << " seconds " << endl;
    } else {
        destAddress = L3AddressResolver().resolve(par("destAddress"));
        destPort = par("destPort");

        localPort = par("localPort");
        socket.setOutputGate(gate("socketOut"));
        socket.bind(localPort);
        std::cout << "ForwardShipApp::initTraffic UDP socket is binded to local port " << localPort
                  << " and sends datagrams to: " << destAddress << ":" << std::to_string(destPort) << endl;
    }
}

void ForwardUdpApp::handleMessage(cMessage* msg) {
    if (msg->getArrivalGate() == gate("dataIn")) {
        forwardData(msg);
    } else if (msg->getArrivalGate() == gate("socketIn") || msg->getArrivalGate() == gate("directIn")) {
        processData(msg);
    } else {
        std::cerr << "[ForwardUdpApp::handleMessage] error! an extra gate?.." << std::endl;
    }
}

void ForwardUdpApp::forwardData(cMessage* msg) {
    Packet* packet = new Packet;
    packet->setName(packetName);

    // c-cast
    MultiHomeMessage* mhmsg = (MultiHomeMessage*)msg;

    auto header = inet::makeShared<MultiHomePacket>();
    header->setId(mhmsg->getId());

    // reset tos if cellular
    NetworkType nt = static_cast<NetworkType>(mhmsg->getNetworkType());
    if (nt == NetworkType::LTE) {
        socket.setTos(-1);
    } else if (nt == NetworkType::NR) {
        socket.setTos(10);
    }
    header->setNetworkType(mhmsg->getNetworkType());
    header->setDestAddress(destAddress.str().c_str());
    header->setDestPort(destPort);
    header->setSendingTime(mhmsg->getSendingTime());
    header->setPayloadSize(mhmsg->getPayloadSize());
    header->setChunkLength(B(mhmsg->getPayloadSize() + 32));

    // insert header
    packet->insertAtFront(header);

    // send packet
    socket.sendTo(packet, destAddress, destPort);
    delete msg;
}

void ForwardUdpApp::processData(cMessage* msg) {
    sendDirect(msg, multiHomeApp, multiHomeApp->findGate("directIn"));
}

void ForwardUdpApp::finish() {
}
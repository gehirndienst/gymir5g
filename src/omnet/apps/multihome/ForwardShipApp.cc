#include "ForwardShipApp.h"

Define_Module(ForwardShipApp);

void ForwardShipApp::forwardData(cMessage* msg) {
    Packet* packet = nullptr;
    packet = dynamic_cast<Packet*>(msg);
    if (!packet) {
        delete msg;
        return;
    }
    Packet* packetCopy = packet->dup();

    NetworkType networkType;
    const std::string packetName = packet->getName();
    if (packetName == "data") {
        const auto& h = packet->popAtFront<ShipPacket>();
        networkType = static_cast<NetworkType>(h->getNetworkType());
    } else if (packetName == "data-nack") {
        const auto& h = packet->popAtFront<NackDecision>();
        networkType = static_cast<NetworkType>(h->getNetworkType());
    } else if (packetName == "senderReport") {
        const auto& h = packet->popAtFront<SenderReportPacket>();
        networkType = static_cast<NetworkType>(h->getNetworkType());
    } else if (packetName == "nackRefusal") {
        const auto& h = packet->popAtFront<NackDecision>();
        networkType = static_cast<NetworkType>(h->getNetworkType());
    } else if (packetName == "fec") {
        const auto& h = packet->popAtFront<FecPacket>();
        networkType = static_cast<NetworkType>(h->getNetworkType());
    } else {
        std::cerr << "[ForwardShipApp::forwardData] error! unknown packet: " << packetName << std::endl;
        delete packet;
        return;
    }
    delete packet;

    // reset tos if cellular
    if (networkType == NetworkType::LTE) {
        socket.setTos(-1);
    } else if (networkType == NetworkType::NR) {
        socket.setTos(10);
    }

    // send packet
    socket.sendTo(packetCopy, destAddress, destPort);
}
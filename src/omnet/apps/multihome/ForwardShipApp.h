/*
 * ForwardShipApp.h
 *
 * an app that forwards ship packets to the ShoreApp and vice versa
 *
 * Created on: Nov 13, 2023
 *      Author: Nikita Smirnov
 */

#ifndef FORWARDSHIPAPP_H
#define FORWARDSHIPAPP_H

#include <random>

#include "omnetpp.h"
#include "inet/common/INETDefs.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

#include "apps/ship/messages/ShipPacket_m.h"
#include "apps/ship/messages/FecPacket_m.h"
#include "apps/ship/messages/NackDecision_m.h"
#include "apps/ship/messages/SenderReportPacket_m.h"
#include "apps/ship/messages/ShipPacket_m.h"
#include "apps/shore/messages/NackPacket_m.h"
#include "model/common/NetworkType.h"
#include "omnet/apps/multihome/ForwardUdpApp.h"

using namespace omnetpp;
using namespace inet;

class ForwardShipApp: public ForwardUdpApp {
public:
    ForwardShipApp() = default;
    virtual ~ForwardShipApp() = default;

protected:
    void forwardData(cMessage* msg) override;
};

#endif

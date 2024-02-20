/*
 * PackeetEntry.h
 *
 * a packet entry pushed in buffers
 *
 *  Created on: Aug 14, 2023
 *      Author: Nikita Smirnov
 */

#ifndef PACKETENTRY_H
#define PACKETENTRY_H

#include "inet/common/INETDefs.h"
#include "inet/common/packet/Packet.h"
#include "omnetpp.h"

struct PacketEntry {
    const inet::Packet* packet;
    int sequenceNumber;
    omnetpp::simtime_t timestamp;

    PacketEntry(const inet::Packet* pkt, int seq, omnetpp::simtime_t ts)
        : packet(pkt), sequenceNumber(seq), timestamp(ts) {
    }

    ~PacketEntry() {}

    // usually inet::Packet is managed externally, so to avoid faults delete it with a separate method
    void deletePacket() { if (packet) delete packet; }

};

struct PacketEntryDescendingOrder {
    bool operator()(const PacketEntry& lhs, const PacketEntry& rhs) const {
        return lhs.sequenceNumber > rhs.sequenceNumber;
    }
};

#endif
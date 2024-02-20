/*
 * RetransmissionBuffer.h
 *
 * an aging buffer for transmitted packets that can be retransmitted after a NACK or FIR request
 * NOTE: for simplicity inject it into each stream as it uses stream-related sequence numbers
 *
 *  Created on: Aug 14, 2023
 *      Author: Nikita Smirnov
 */

#ifndef RETRANSMISSIONBUFFER_H
#define RETRANSMISSIONBUFFER_H

#include <deque>
#include <optional>

#include "PacketEntry.h"

class RetransmissionBuffer {
public:
    RetransmissionBuffer(double maxAge) : maxAge_(maxAge) {}
    RetransmissionBuffer() = default;
    ~RetransmissionBuffer() { packets_.clear(); }

    inline void setMaxAge(double maxAge) {
        // it sets only if it was not zero
        if (maxAge_ == 0.0) {
            maxAge_ = maxAge;
        } else {
            std::cerr << "[RetransmissionBuffer::setMaxAge] can't set maxAge, it is already non-zero" << std::endl;
        }
    }

    inline void addPacket(const inet::Packet* packet, int sequenceNumber, omnetpp::simtime_t currTime) {
        if (maxAge_ > 0.0) {
            packets_.emplace_back(packet, sequenceNumber, currTime);
            pruneOldPackets(currTime);
        }
    }

    inline std::optional<const inet::Packet*> findPacket(int sequenceNumber) const noexcept {
        for (const auto& entry : packets_) {
            if (entry.sequenceNumber == sequenceNumber) {
                return entry.packet;
            }
        }
        return std::nullopt;
    }


private:
    inline void pruneOldPackets(omnetpp::simtime_t currentTime) {
        while (!packets_.empty() && isPacketTooOld(packets_.front().timestamp, currentTime)) {
            PacketEntry& packetEntry = packets_.front();
            packetEntry.deletePacket();
            packets_.pop_front();
        }
    }

    inline bool isPacketTooOld(omnetpp::simtime_t packetTime, omnetpp::simtime_t currentTime) const {
        return (currentTime - packetTime).dbl() > maxAge_;
    }

private:
    double maxAge_;
    std::deque<PacketEntry> packets_;
};

#endif
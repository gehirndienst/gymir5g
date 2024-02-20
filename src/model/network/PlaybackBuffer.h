/*
 * PlaybackBuffer.h
 *
 * playback buffer for received packets
 *
 *  Created on: Aug 15, 2023
 *      Author: Nikita Smirnov
 */

#ifndef PLAYBACKBUFFER_H
#define PLAYBACKBUFFER_H

#include <queue>

#include "PacketEntry.h"

class PlaybackBuffer {
public:
    PlaybackBuffer()
        : nextLostSequenceNumber_(0)
        , maxPushedSequenceNumber_(0)
        , maxPlayedSequenceNumber_(0)
        , repairedAndRetransmittedPackets_(0) {}

    inline void pushPacket(const inet::Packet* packet, int sequenceNumber, omnetpp::simtime_t currTime) {
        buffer_.emplace(packet, sequenceNumber, currTime);
        maxPushedSequenceNumber_ = std::max(maxPushedSequenceNumber_, sequenceNumber);
    }

    inline std::optional<const PacketEntry> popNextPacket() noexcept {
        // cheap copy since the pointer is not copied and owned at the application layer
        if (!buffer_.empty()) {
            // nextLostSequenceNumber_ == 0 means there are no packets lost or retranmission is off
            if (nextLostSequenceNumber_ == 0 ||
                    (nextLostSequenceNumber_ > 0 && buffer_.top().sequenceNumber <= nextLostSequenceNumber_)) {
                const PacketEntry nextPacketEntry = buffer_.top();
                buffer_.pop();
                // it could be rarely that a packet was succesfully repaired meanwhile also rentransmitted
                // then it is doubled in queue and an extra pop is required (we pop the first repaired one)
                if (!buffer_.empty() && buffer_.top().sequenceNumber == nextPacketEntry.sequenceNumber) {
                    const PacketEntry retransmittedPacketEntry = buffer_.top();
                    buffer_.pop();
                    maxPlayedSequenceNumber_ = retransmittedPacketEntry.sequenceNumber;
                    repairedAndRetransmittedPackets_++;
                    return retransmittedPacketEntry;
                } else {
                    maxPlayedSequenceNumber_ = nextPacketEntry.sequenceNumber;
                    return nextPacketEntry;
                }
            }
        }
        return std::nullopt;
    }

    // this update is intentionally done externally since this value is maintained by other objecst
    // there should be a continuous interaction between a lost packet manager (e.g., LostPacketsMap) and a PB
    inline void updateNextLostSequenceNumber(int nextLostSequenceNumber) {
        nextLostSequenceNumber_ = nextLostSequenceNumber;
    }

    inline int getMaxPushedSequenceNumber() { return maxPushedSequenceNumber_; }

    inline int getMaxPlayedSequenceNumber() { return maxPlayedSequenceNumber_; }

    inline int getRepairedAndRetransmittedPackets() { return repairedAndRetransmittedPackets_; }

    inline bool isEmpty() const {
        return buffer_.empty();
    }

private:
    std::priority_queue<PacketEntry, std::vector<PacketEntry>, PacketEntryDescendingOrder> buffer_;
    int nextLostSequenceNumber_;
    int maxPushedSequenceNumber_;
    int maxPlayedSequenceNumber_;
    int repairedAndRetransmittedPackets_; // for statistic
};

#endif

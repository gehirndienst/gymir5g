/*
 * RepairBuffer.h
 *
 * a repair buffer that holds redundant packets recieved with FEC mechanism
 * it implements repairing fully simulative, pointing whether a packet should be considered as "not lost"
 * if it is theoretically repairable with sequential XOR FEC mechanism
 *
 *  Created on: Aug 17, 2023
 *      Author: Nikita Smirnov
 */

#ifndef REPAIRBUFFER_H
#define REPAIRBUFFER_H

#include <algorithm>
#include <queue>
#include <vector>

enum class RepairStatus {
    UNREPAIRABLE,
    MAYBE_REPAIRABLE,
    REPAIRABLE
};

struct RepairSequence {
    int leftSequenceNumber;
    int rightSequenceNumber;
    int packetsReceived;

    RepairSequence(): leftSequenceNumber(0), rightSequenceNumber(0), packetsReceived(0) {}
    RepairSequence(int leftSequenceNumber, int rightSequenceNumber, int packetsReceived)
        : leftSequenceNumber(leftSequenceNumber), rightSequenceNumber(rightSequenceNumber), packetsReceived(packetsReceived) {}


    inline bool belongs(int sequenceNumber) const {
        return sequenceNumber >= leftSequenceNumber && sequenceNumber <= rightSequenceNumber;
    }

    inline void incrementPacketsReceived() {
        packetsReceived++;
    }

    inline bool isRepairable (int sequenceNumber) const {
        // A_i is repairable if max 1 packet is missing from FEC sequence A_0 XOR ... XOR A_N
        return packetsReceived >= rightSequenceNumber - leftSequenceNumber;
    }
};

class RepairBuffer {
public:
    RepairBuffer() : firstRightRepairSequenceNumber_(0), lastRightRepairSequenceNumber_(0) {}
    ~RepairBuffer() = default;

    inline void addSequenceNumber(int sequenceNumber) {
        auto repairSequenceNullable = findRepairSequence(sequenceNumber);
        if (repairSequenceNullable) {
            repairSequenceNullable->incrementPacketsReceived();
        } else {
            packetsQueue_.emplace(sequenceNumber);
        }
    }

    inline void addRepairSequence(int firstSequenceNumber, int lastSequenceNumber) {
        int packetsAlreadyReceived = 0;
        while(!packetsQueue_.empty()) {
            int sequenceNumber = packetsQueue_.top();
            if (sequenceNumber < firstSequenceNumber) {
                // from previous fecs which are lost, drop it
                packetsQueue_.pop();
                continue;
            } else if (sequenceNumber >= firstSequenceNumber && sequenceNumber <= lastSequenceNumber) {
                // our client
                packetsAlreadyReceived ++;
                packetsQueue_.pop();
                continue;
            } else {
                // there are packets for later repair sequences, nothing for us, break
                break;
            }
        }
        buffer_.emplace_back(firstSequenceNumber, lastSequenceNumber, packetsAlreadyReceived);
        lastRightRepairSequenceNumber_ = std::max(lastRightRepairSequenceNumber_, lastSequenceNumber);
    }

    inline RepairSequence* findRepairSequence(int sequenceNumber) {
        for (auto it = buffer_.begin(); it != buffer_.end(); ++it) {
            if ((*it).belongs(sequenceNumber)) {
                return &(*it);
            }
        }
        return nullptr;
    }

    inline RepairStatus getRepairStatus(int lostSequenceNumber) {
        if (buffer_.empty()) {
            return RepairStatus::UNREPAIRABLE;
        } else {
            pruneOld(lostSequenceNumber);
            auto repairSequenceNullable = findRepairSequence(lostSequenceNumber);
            if (repairSequenceNullable) {
                // if it is not old then we need to check if we have all needed packets
                bool isRepairable = repairSequenceNullable->isRepairable(lostSequenceNumber);
                return isRepairable ? RepairStatus::REPAIRABLE : RepairStatus::MAYBE_REPAIRABLE;
            } else {
                // it is already pruned
                return RepairStatus::UNREPAIRABLE;
            }
        }
    }

    inline RepairStatus getRepairStatus(int lostSequenceNumber, RepairSequence* repairSequenceNullable) {
        if (!repairSequenceNullable) {
            return RepairStatus::UNREPAIRABLE;
        } else {
            pruneOld(lostSequenceNumber);
            bool isRepairable = repairSequenceNullable->isRepairable(lostSequenceNumber);
            return isRepairable ? RepairStatus::REPAIRABLE : RepairStatus::MAYBE_REPAIRABLE;
        }
    }

private:
    inline void pruneOld(int sequenceNumberAsked) {
        if (firstRightRepairSequenceNumber_ < sequenceNumberAsked) {
            // next lost packet is newer than oldest right repair sequence number
            buffer_.erase(std::remove_if(
                              buffer_.begin(), buffer_.end(),
            [sequenceNumberAsked](const RepairSequence & repair) {
                return repair.rightSequenceNumber < sequenceNumberAsked;
            }),
            buffer_.end());
            firstRightRepairSequenceNumber_ = buffer_.empty() ? buffer_.begin()->rightSequenceNumber : 0;
        }
    }

private:
    std::vector<RepairSequence> buffer_;
    std::priority_queue <int, std::vector<int>, std::greater<int>> packetsQueue_;
    int firstRightRepairSequenceNumber_;
    int lastRightRepairSequenceNumber_;
};

#endif
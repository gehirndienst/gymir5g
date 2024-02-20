/*
 * JitterBuffer.h
 *
 * jitter buffer for jitter frames
 *
 *  Created on: Oct 10, 2023
 *      Author: Nikita Smirnov
 */

#ifndef JITTERBUFFER_H
#define JITTERBUFFER_H

#include <algorithm>
#include <vector>
#include <omnetpp.h>

#include "model/network/JitterFrame.h"

class JitterBuffer {
public:
    JitterBuffer(const std::string& streamName = "stream")
        : streamName_(streamName), lastRemovedJitterFrameNumber_(0) {}

    inline std::pair<bool, bool> pushPacket(const inet::Packet* packet, int sequenceNumber, int frameNumber,
                                            omnetpp::simtime_t currTime, bool isLast) {
        // returns a pair of flags: first means that a new frame was added (used for playMsg scheduling),
        // second means that a frame was completed (used for resumePlay calls)
        bool isFrameCompleted = false;
        if (frameNumber > lastAddedJitterFrameNumber_) {
            JitterFrame jitterFrame(streamName_, frameNumber);
            isFrameCompleted = jitterFrame.addPacket(packet, sequenceNumber, currTime, isLast);
            auto it = std::lower_bound(buffer_.begin(), buffer_.end(), jitterFrame);
            buffer_.insert(it, jitterFrame);
            lastAddedJitterFrameNumber_ = std::max(lastAddedJitterFrameNumber_, frameNumber);
            return std::make_pair(true, isFrameCompleted);
        } else {
            JitterFrame* maybeJitterFrame = findFrameNullable(frameNumber);
            if (maybeJitterFrame) {
                isFrameCompleted = maybeJitterFrame->addPacket(packet, sequenceNumber, currTime, isLast);
                return std::make_pair(false, isFrameCompleted);
            } else {
                JitterFrame jitterFrame(streamName_, frameNumber);
                isFrameCompleted = jitterFrame.addPacket(packet, sequenceNumber, currTime, isLast);
                auto it = std::lower_bound(buffer_.begin(), buffer_.end(), jitterFrame);
                buffer_.insert(it, jitterFrame);
                return std::make_pair(true, isFrameCompleted);
            }
        }
    }

    inline std::vector<JitterFrame> removeFrame(int frameNumber) {
        std::vector<JitterFrame> frames;
        int eraseTo = 0;
        for (JitterFrame& jitterFrame : buffer_) {
            if (jitterFrame.frameNumber <= frameNumber) {
                frames.push_back(jitterFrame);
                eraseTo++;
            }
        }
        if (eraseTo > 0) {
            std::sort(buffer_.begin(), buffer_.end());
            buffer_.erase(buffer_.begin(), buffer_.begin() + eraseTo);
        }
        lastRemovedJitterFrameNumber_ = frameNumber;
        return frames;
    }

    omnetpp::cMessage* getPlayMsg(int frameNumber) {
        JitterFrame* maybeJitterFrame = findFrameNullable(frameNumber);
        if (maybeJitterFrame) {
            return maybeJitterFrame->playMsg;
        }
        return nullptr;
    }

    // TODO: make private?
    inline JitterFrame* findFrameNullable(int frameNumber) {
        auto it = std::find_if(buffer_.begin(), buffer_.end(), [frameNumber](const JitterFrame & frame) {
            return frame.frameNumber == frameNumber;
        });
        return it != buffer_.end() ? &(*it) : nullptr;
    }

    int getLastPlayedFrame() {
        return lastRemovedJitterFrameNumber_;
    }

private:
    std::vector<JitterFrame> buffer_;
    std::string streamName_; // aux, needed for cMsg assignment
    int lastAddedJitterFrameNumber_;
    int lastRemovedJitterFrameNumber_;
};

#endif
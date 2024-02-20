#include "StreamReceiver.h"

// StreamDecoder

StreamDecoder::StreamDecoder(const std::string& streamName, StreamType streamType,  int verbose)
    : streamName_(streamName), streamType_(streamType), vDecoder_(), pcDecoder_() {
    if (streamType_ == VIDEO) {
        vDecoder_ .verbose = verbose;
        vDecoder_.init(AV_CODEC_ID_H264);
    } else if (streamType_ == LIDAR) {
        pcDecoder_.verbose = verbose;
        pcDecoder_.init(streamName_);
    }
}

bool StreamDecoder::decode(uint8_t* buffer, int bufferSize) {
    if (streamType_ == VIDEO) {
        return vDecoder_.decode(buffer, bufferSize);
    } else if (streamType_ == LIDAR) {
        return pcDecoder_.decode(buffer, bufferSize);
    } else {
        return true;
    }
}

void StreamDecoder::view() {
    if (streamType_ == VIDEO) {
        if (!vDecoder_.isPlaying) {
            vDecoder_.initViewer(streamName_);
        }
        vDecoder_.view();
    } else if (streamType_ == LIDAR) {
        pcDecoder_.view();
    } else {
        std::cerr << "StreamDecoder::view can't view a byte stream, please deselect isView for BLOB or SIM streams"
                  << std::endl;
    }
}

void StreamDecoder::write() {
    if (streamType_ == VIDEO) {
        if (vDecoder_.getFramesDecoded() == 1) {
            // add current timestamp with mss to the filename
            auto now = std::chrono::system_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
            auto timer = std::chrono::system_clock::to_time_t(now);
            std::tm bt = *std::localtime(&timer);
            std::ostringstream oss;
            oss << std::put_time(&bt, "%H-%M-%S");
            oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
            std::string filename = streamName_ + "_" + oss.str() + ".mp4";
            vDecoder_.initWriter(filename);
        }
        vDecoder_.write();
    }
}

void StreamDecoder::clearAssemblingContainer() {
    // special method for point cloud decoder to let him know that a new portion of clouds is going to be decoded
    pcDecoder_.clearAssemblingContainer();
}

// StreamReport
StreamReporter::StreamReporter()
    : relativeTransitTime(0.0)
    , jitter(0.0)
    , totalPacketsReceived(0)
    , totalPacketsLost(0)
    , totalPacketsOutOfOrder(0)
    , totalPacketsPlayed(0)
    , totalPacketsRepaired(0)
    , totalPacketsRetransmitted(0)
    , totalBytesReceived(0)
    , maxSequenceNumber(0)
    , fractionLastMaxSequenceNumber(0)
    , fractionPacketsReceived(0)
    , fractionPacketsRepaired(0)
    , fractionPacketsRetransmitted(0)
    , fractionPacketsRepairedAndRetransmitted(0)
    , fractionLost(0.0)
    , fractionRate(0.0)
    , fractionFecRate(0.0)
    , fractionStallingRate(0.0)
    , stallingSum(0.0)
    , lastPacketArrivalTimestamp(0.0)
    , lastSenderReportSentTimestamp(0.0)
    , lastSenderReportArrivalTimestamp(0.0)
    , lastPacketPlayTimestamp(0.0)
    , bytesReceivedSinceLastRR(0)
    , bytesFecReceivedSinceLastRR(0) {}

StreamReporter::~StreamReporter() {
    lostSequenceNumbers.clear();
    sequenceNumbersSinceLastRR.clear();
    sequenceNumbersSinceLastTF.clear();
    deltasSinceLastTF.clear();
}

void StreamReporter::processRtpPacket(int sequenceNumber, int payloadSize,
                                      simtime_t senderTimestamp, simtime_t arrivalTimestamp) {
    // calculate transit time and interarrival jitter according to RFC 3550, A8, but directly in sec instead of TS units
    double relativeTransitTimeNew = (arrivalTimestamp - senderTimestamp).dbl();
    double packetSpacingDiff = abs(relativeTransitTimeNew - relativeTransitTime);
    double smoothedJitterGradient = (1.0 / 16.0) * (packetSpacingDiff - jitter);
    jitter += smoothedJitterGradient;
    relativeTransitTime = relativeTransitTimeNew;
    // interarrival time
    if (totalPacketsReceived > 0) {
        interarrivalDelay.add((arrivalTimestamp - lastPacketArrivalTimestamp).dbl());
    }
    // transmission delay
    transmissionDelay.add((arrivalTimestamp - senderTimestamp).dbl());

    // check if have lost sequence numbers
    lostSequenceNumbers.clear();
    int sequenceNumbersDifference = sequenceNumber - maxSequenceNumber;
    if (sequenceNumbersDifference > 1) {
        for (int i = 1; i < sequenceNumbersDifference; ++i) {
            lostSequenceNumbers.push_back(maxSequenceNumber + i);
        }
    } else if (sequenceNumbersDifference < 1) {
        // out-of-order packet
        totalPacketsOutOfOrder++;
    }

    // update reference sequence numbers
    maxSequenceNumber = std::max(maxSequenceNumber, sequenceNumber);

    // push new values
    sequenceNumbersSinceLastRR.push_back(sequenceNumber);
    sequenceNumbersSinceLastTF.push_back(sequenceNumber);
    lastPacketArrivalTimestamp = arrivalTimestamp;
    if (lastPacketArrivalTimestamp > 0.0) {
        deltasSinceLastTF.push_back(arrivalTimestamp - lastPacketArrivalTimestamp);
    }
    totalPacketsReceived++;
    totalBytesReceived += payloadSize;
    bytesReceivedSinceLastRR += payloadSize;

    // process packet with gcc estimator
    gccEstimator.updateWithRtpPacket(sequenceNumber, senderTimestamp.dbl() * 1000, arrivalTimestamp.dbl() * 1000,
                                     payloadSize);
}

void StreamReporter::processRetransmittedRtpPacket(int sequenceNumber, int payloadSize,
        simtime_t payloadTimestamp, simtime_t retransmissionTimestamp, simtime_t arrivalTimestamp) {
    // don't update loss vector, otherwise the same except two timestamps
    double relativeTransitTimeNew = (arrivalTimestamp - retransmissionTimestamp).dbl();
    double packetSpacingDiff = abs(relativeTransitTimeNew - relativeTransitTime);
    double smoothedJitterGradient = (1.0 / 16.0) * (packetSpacingDiff - jitter);
    jitter += smoothedJitterGradient;
    relativeTransitTime = relativeTransitTimeNew;
    if (totalPacketsReceived > 0) {
        interarrivalDelay.add((arrivalTimestamp - lastPacketArrivalTimestamp).dbl());
    }
    maxSequenceNumber = std::max(maxSequenceNumber, sequenceNumber);
    transmissionDelay.add((arrivalTimestamp - retransmissionTimestamp).dbl());
    // delay for how much time it lasts from INITIAL sending to receiving
    retransmissionDelay.add((arrivalTimestamp - payloadTimestamp).dbl());
    // NOTE: if it is a retransmission for a previous fraction, don't add it to the current fraction vector
    if (sequenceNumber > fractionLastMaxSequenceNumber) {
        sequenceNumbersSinceLastRR.push_back(sequenceNumber);
        bytesReceivedSinceLastRR += payloadSize;
    }
    sequenceNumbersSinceLastTF.push_back(sequenceNumber);
    lastPacketArrivalTimestamp = arrivalTimestamp;
    if (lastPacketArrivalTimestamp > 0.0) {
        deltasSinceLastTF.push_back(arrivalTimestamp - lastPacketArrivalTimestamp);
    }
    fractionPacketsRetransmitted++;
    totalPacketsReceived++;
    totalPacketsRetransmitted++;
    totalBytesReceived += payloadSize;
    // process packet with gcc estimator
    gccEstimator.updateWithRtpPacket(sequenceNumber, retransmissionTimestamp.dbl() * 1000, arrivalTimestamp.dbl() * 1000,
                                     payloadSize);
}

void StreamReporter::processFecPacket(int payloadSize) {
    totalBytesReceived += payloadSize;
    bytesFecReceivedSinceLastRR += payloadSize;
}

void StreamReporter::processSenderReport(simtime_t senderTimestamp) {
    lastSenderReportSentTimestamp = senderTimestamp;
    lastSenderReportArrivalTimestamp = simTime();
}

void StreamReporter::estimateFractionStats() {
    // check RR cache vector
    if (!sequenceNumbersSinceLastRR.empty()) {
        fractionPacketsReceived = (int)sequenceNumbersSinceLastRR.size();
        int fractionPacketsGotAtAll = fractionPacketsReceived + fractionPacketsRepaired;
        int fractionPacketsLost = maxSequenceNumber - fractionLastMaxSequenceNumber - fractionPacketsGotAtAll;
        fractionLost =
            std::max(0.0, static_cast<double>(fractionPacketsLost) / (fractionPacketsLost + fractionPacketsGotAtAll));
        fractionLastMaxSequenceNumber = maxSequenceNumber;
        fractionRate =
            static_cast<double>(bytesReceivedSinceLastRR) * 0.000008 / (simTime() - lastReceiverReportSentTimestamp).dbl();
        fractionFecRate =
            static_cast<double>(bytesFecReceivedSinceLastRR) * 0.000008 / (simTime() - lastReceiverReportSentTimestamp).dbl();
        fractionStallingRate = stallingSum / (simTime() - lastReceiverReportSentTimestamp).dbl();
        bandwidth = gccEstimator.estimateBandwidth();
    } else {
        fractionPacketsReceived = 0;
        fractionLost = 1.0;
        fractionRate = 0.0;
    }

    // reset values
    fractionPacketsRepaired = 0;
    fractionPacketsRetransmitted = 0;
    lastReceiverReportSentTimestamp = simTime();
    bytesReceivedSinceLastRR = 0;
    bytesFecReceivedSinceLastRR = 0;
    stallingSum = 0.0;
    sequenceNumbersSinceLastRR.clear();
}

void StreamReporter::reportLostPacket() {
    totalPacketsLost++;
}

void StreamReporter::reportPlayedPacket(double playoutDelayReported) {
    totalPacketsPlayed++;
    // FIXME: report delay = 0? And should we add some additional playback delay for smoothness?
    playoutDelay.add(playoutDelayReported);
    double playbackDelayCalculated = (simTime() - lastPacketPlayTimestamp).dbl();
    lastPacketPlayTimestamp = simTime();
    playbackDelay.add(playbackDelayCalculated);
    // stalling considered as playbackDelay > 300ms
    double stalling = playbackDelayCalculated - 0.3;
    if (stalling > 0)
        stallingSum += stalling;
}

void StreamReporter::reportRepairedPacket(int sequenceNumber, int repairedAndRetransmittedPackets) {
    fractionPacketsRepaired++;
    totalPacketsRepaired++;
    totalPacketsRepairedAndRetransmitted = repairedAndRetransmittedPackets;
    // could be 1 2 3 4 5(LOST) FEC_1-5 -- then repaired 5th packet will be the last sequence number
    maxSequenceNumber = std::max(maxSequenceNumber, sequenceNumber);
}

void StreamReporter::eraseTFVectors() {
    sequenceNumbersSinceLastTF.clear();
    deltasSinceLastTF.clear();
}

void StreamReporter::eraseCachedStatistic() {
    playoutDelay.clearCache();
    interarrivalDelay.clearCache();
    transmissionDelay.clearCache();
    retransmissionDelay.clearCache();
}


std::vector<int>& StreamReporter::getLostSequenceNumbers() { return lostSequenceNumbers; }

// StreamReceiver

StreamReceiver::StreamReceiver(const std::string& streamName, StreamType streamType, int verbose)
    : streamName(streamName)
    , streamType(streamType)
    , elementsDecoded(0)
    , jitterBuffer_(streamName)
    , decoder_(streamName, streamType, verbose) {}

StreamReceiver::StreamReceiver()
    : StreamReceiver::StreamReceiver("simstream", SIM, 0) {}

// CHUNKS OR SEGMENTS

void StreamReceiver::addChunk(int elemNumber, int payloadSize, int offset, double arrivalTime,
                              inet::Packet* packetData) {
    StreamChunk chunk { payloadSize, offset, arrivalTime };
    packetData->peekDataAt<inet::BytesChunk>(inet::b(0), inet::B(payloadSize))->copyToBuffer(
        chunk.data, (size_t)payloadSize);
    if (chunks_.count(elemNumber) == 0) {
        std::vector<StreamChunk> chunksVector;
        chunksVector.push_back(chunk);
        chunks_.insert({elemNumber, chunksVector});
    } else {
        chunks_[elemNumber].push_back(chunk);
    }
}

void StreamReceiver::forceDecodePreviousElements(int elemNumber) {
    // if a new element comes but previous ones are still waiting for missing chunks -- force decode the previous one
    std::vector<int> tmpChunksOfElemNumsToDecode;
    if (!chunks_.empty()) {
        for (auto& it : chunks_) {
            if (it.first < elemNumber) {
                tmpChunksOfElemNumsToDecode.push_back(it.first);
            }
        }
        if (!tmpChunksOfElemNumsToDecode.empty()) {
            for (int& elemNum : tmpChunksOfElemNumsToDecode) {
                decode(elemNum);
            }
        }
    }
}

int StreamReceiver::getSizeOfAllChunks(int elemNumber) {
    assert(chunks_.count(elemNumber) == 1);
    int sizeOfAllChunks = 0;
    for (StreamChunk& chunk : chunks_[elemNumber]) {
        sizeOfAllChunks += chunk.size;
    }
    return sizeOfAllChunks;
}

std::vector<StreamChunk> StreamReceiver::getChunks(int elemNumber) {
    return chunks_[elemNumber];
}

// PACKETS LOST (NACK + FEC)

LostPacketsMap& StreamReceiver::getLostPacketsMap() { return lostPacketsMap_; }

int StreamReceiver::getNextLostSequenceNumber() {
    return !lostPacketsMap_.empty() ? lostPacketsMap_.begin()->first : 0;
}

int StreamReceiver::getFirstLostSequenceNumberInRange(int left, int right) {
    LostPacketsMap& lostPacketMap = getLostPacketsMap();
    int firstKeyInRange = -1;
    for (auto it = lostPacketMap.lower_bound(left); it != lostPacketMap.end() && it->first <= right; ++it) {
        firstKeyInRange = it->first;
    }
    return firstKeyInRange;
}

void StreamReceiver::putIntoLostPacketsMap(int lostSequenceNumber, simtime_t currentTime, bool isFec, bool isNack) {
    if (lostPacketsMap_.find(lostSequenceNumber) == lostPacketsMap_.end()) {
        lostPacketsMap_.emplace(lostSequenceNumber,
                                LostPacket(streamName, lostSequenceNumber, currentTime, isFec, isNack));
    }
}

void StreamReceiver::updateNextLostSequenceNumber(int sequenceNumber) {
    playbackBuffer_.updateNextLostSequenceNumber(sequenceNumber);
}

void StreamReceiver::eraseAndUpdateNextLostSequenceNumber(int sequenceNumber) {
    lostPacketsMap_.erase(sequenceNumber);
    playbackBuffer_.updateNextLostSequenceNumber(getNextLostSequenceNumber());
}

// PLAYBACK BUFFER

void StreamReceiver::pushIntoPlaybackBuffer(const inet::Packet* packet, int sequenceNumber,
        omnetpp::simtime_t currTime) {
    playbackBuffer_.pushPacket(packet, sequenceNumber, currTime);
}

std::optional<const PacketEntry> StreamReceiver::getNextFromPlaybackBuffer() {
    return playbackBuffer_.popNextPacket();
}

int StreamReceiver::getMaxPushedSequenceNumber() { return playbackBuffer_.getMaxPushedSequenceNumber(); }
int StreamReceiver::getMaxPlayedSequenceNumber() { return playbackBuffer_.getMaxPlayedSequenceNumber(); }
int StreamReceiver::getRepairedAndRetransmittedPackets() {
    return playbackBuffer_.getRepairedAndRetransmittedPackets();
}

// JITTER BUFFER

std::pair<bool, bool> StreamReceiver::pushIntoJitterBuffer(
    const inet::Packet* packet,
    int sequenceNumber,
    int frameNumber,
    omnetpp::simtime_t currTime,
    bool isLast
) {
    return jitterBuffer_.pushPacket(packet, sequenceNumber, frameNumber, currTime, isLast);
}

std::vector<JitterFrame> StreamReceiver::removeJitterFrame(int frameNumber) {
    return jitterBuffer_.removeFrame(frameNumber);
}

omnetpp::cMessage* StreamReceiver::getPlayMsg(int frameNumber) {
    return jitterBuffer_.getPlayMsg(frameNumber);
}

int StreamReceiver::getLastPlayedFrame() { return jitterBuffer_.getLastPlayedFrame(); }

// REPAIR BUFFER

void StreamReceiver::addSequenceNumberToRepairBuffer(int sequenceNumber) {
    repairBuffer_.addSequenceNumber(sequenceNumber);
}

void StreamReceiver::addRepairSequence(int firstSequenceNumber, int lastSequenceNumber) {
    repairBuffer_.addRepairSequence(firstSequenceNumber, lastSequenceNumber);
}

RepairSequence* StreamReceiver::findRepairSequence(int sequenceNumber) {
    return repairBuffer_.findRepairSequence(sequenceNumber);
}

RepairStatus StreamReceiver::getRepairStatus(int lostSequenceNumber, RepairSequence* repairSequence) {
    if (!repairSequence) {
        return repairBuffer_.getRepairStatus(lostSequenceNumber);
    } else {
        return repairBuffer_.getRepairStatus(lostSequenceNumber, repairSequence);
    }
}

// DECODING

void StreamReceiver::decode(int elemNumber) {
    // sort chunks according to their offsets (in case of out-of-order)
    auto& chunksVector = chunks_[elemNumber];
    sort(chunksVector.begin(), chunksVector.end(), [](const StreamChunk a, const StreamChunk b) {return b > a; });

    if (streamType == VIDEO) {
        // make a buffer
        int bufferSize = getSizeOfAllChunks(elemNumber);
        uint8_t* buffer = new uint8_t[bufferSize];
        // assemble all nals into one buffer via memcpy and then decode
        unsigned int tmpSize = 0;
        for (StreamChunk& chunk : chunksVector) {
            memcpy(&(buffer[tmpSize]), chunk.data, chunk.size);
            tmpSize += chunk.size;
        }
        if (decoder_.decode(buffer, bufferSize)) {
            elementsDecoded ++;
        }
        delete buffer;
        // ready
    } else if (streamType == LIDAR) {
        //  with lidar a bit different: first it should be decoded only after that assembled
        bool isOkDecoded = true;
        decoder_.clearAssemblingContainer();
        for (StreamChunk& chunk : chunksVector) {
            isOkDecoded = isOkDecoded && decoder_.decode(chunk.data, chunk.size);
        }
        if (isOkDecoded) {
            elementsDecoded ++;
        }
        // ready
    } else {
        return; // no decoding is needed? maybe some unpacking for BLOB?
    }
    chunks_.erase(elemNumber);
}

void StreamReceiver::view() { decoder_.view(); }

void StreamReceiver::write() { decoder_.write(); }

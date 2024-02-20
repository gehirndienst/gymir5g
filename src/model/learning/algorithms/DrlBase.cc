#include "DrlBase.h"

void DrlBase::applyAction(std::unique_ptr<Stream>& stream, int actionNum) {
    StreamActions::setQuality(stream, actionNum);
}

void DrlBase::applyAction(std::unique_ptr<Stream>& stream, double actionVal) {
    StreamActions::setRate(stream, actionVal);
}

nlohmann::json& DrlBase::makeState(UnitState& state) {
    adaptiveStreamingState["transmission"] = state.getTransmissionState().toJson();
    adaptiveStreamingState["stream"] = state.getStreamState().toJson();
    adaptiveStreamingState["time"] = omnetpp::simTime().dbl();
    return adaptiveStreamingState;
}

/* [[deprecated]]
nlohmann::json& DrlBase::getState(std::unique_ptr<Stream>& stream) {
    double encodingTimesSum = 0.0;
    double queueingTimesSum = 0.0;
    std::vector<int> chunks;

    // get data transmission state and other stream parameters with mutual exclusion
    stream->streamMutex.lock();
    stream->dataTransmissionManager.setPeriod(omnetpp::simTime().dbl());
    DataTransmissionManager dtm(stream->dataTransmissionManager);
    stream->dataTransmissionManager.update();
    int chunksSent = stream->elemsSent - stateLastChunkNum;
    if (chunksSent == 0) {
        state["obs"] = {};
        stream->streamMutex.unlock();
        return state;
    }
    stateLastChunkNum = stream->elemsSent;
    for (int i = chunksSent - 1; i >= 0; i--) {
        encodingTimesSum += (double)stream->lastEncodingTimes.back() / 1000;
        stream->lastEncodingTimes.pop_back();
        queueingTimesSum += (double)stream->lastQueueingTimes.back() / 1000;
        stream->lastQueueingTimes.pop_back();
        chunks.push_back(stream->lastChunkSizes.back());
        stream->lastChunkSizes.pop_back();
    }
    // catch initial sending: could be that stream was turned off but initial packets were send
    bool isRunning = statesMade > 0 ? stream->isRunning : chunksSent > 0;
    stream->streamMutex.unlock();

    // quality is updated only after actioning, so it could be grasped safely without locking a mutex
    double qualityChangeRate = stream->getQualityChanges(stream->pastLookout) / stream->pastLookout;

    // mbits and packets transmitted and received
    double txPacketsPer = dtm.getTxPacketsPer();
    double rxPacketsPer = dtm.getRxPacketsPer();
    double packetLossRate = dtm.getPacketLossRate();
    double packetLossRatePer = dtm.getPacketLossRatePer();

    double txGoodputPerAv = dtm.getTxGoodputAvPer();
    double txGoodputGlobAv = dtm.getTxGoodputAv();
    double rxGoodputPerAv = dtm.getRxGoodputAvPer();
    double rxGoodputGlobAv = dtm.getRxGoodputAv();
    double txGoodputPerToGlob = txGoodputPerAv / txGoodputGlobAv;
    double rxGoodputPerToGlob = rxGoodputPerAv / rxGoodputGlobAv;

    // instability rate
    double networkInstabilityRate = dtm.getInstabilityRatePer();

    // get relative std of chunks
    double meanChunkSize = chunksSent > 0 ? dtm.getTxBytesPer() / (double)chunksSent : 0.0;
    double varChunkSize = 0.0;
    double relStdChunkSize = 0.0;
    for (int i = 0; i < (int)chunks.size(); i++) {
        varChunkSize += std::pow(chunks[i] - meanChunkSize, 2);
    }
    if (chunksSent > 0) {
        varChunkSize /= (double)chunksSent;
        relStdChunkSize = std::sqrt(varChunkSize) / meanChunkSize;
    }

    // delay and jitter
    double averageDelayPer = dtm.getDelayAvPer() / 1000;
    double averageRttPer = dtm.getRttAvPer() / 1000;
    double averageDelayJitterPer = dtm.getDelayJitterAvPer() / 1000;
    double averageRttJitterPer = dtm.getRttJitterAvPer() / 1000;
    double averageDelayGlob = dtm.getDelayAv() / 1000;
    double averageDelayJitterGlob = dtm.getDelayJitterAv() / 1000;
    double averageRttGlob = dtm.getRttAv() / 1000;
    double averageRttJitterGlob = dtm.getRttJitterAv() / 1000;

    // prev state
    auto prevState = state;

    // get over all keys in set observations vector
    for (auto& key : stateKeys) {
        if (key == "txGoodputPerAv") {
            state["obs"]["txGoodputPerAv"] = std::min(txGoodputPerAv, 100.0);
            continue;
        }
        if (key == "txGoodputPrevPerAv") {
            state["obs"]["txGoodputPrevPerAv"] = prevState["obs"]["txGoodputPerAv"];
            continue;
        }
        if (key == "txGoodputGlobAv") {
            state["obs"]["txGoodputGlobAv"] = std::min(txGoodputGlobAv, 100.0);
            continue;
        }
        if (key == "txGoodputPerToGlob") {
            state["obs"]["txGoodputPerToGlob"] = std::min(txGoodputPerToGlob, 5.0);
            continue;
        }
        if (key == "rxGoodputPerAv") {
            state["obs"]["rxGoodputPerAv"] = std::min(rxGoodputPerAv, 100.0);
            continue;
        }
        if (key == "rxGoodputPrevPerAv") {
            state["obs"]["rxGoodputPrevPerAv"] = prevState["obs"]["rxGoodputPerAv"];
            continue;
        }
        if (key == "rxGoodputGlobAv") {
            state["obs"]["rxGoodputGlobAv"] = std::min(rxGoodputGlobAv, 100.0);
            continue;
        }
        if (key == "rxGoodputPerToGlob") {
            state["obs"]["rxGoodputPerToGlob"] = std::min(rxGoodputPerToGlob, 5.0);
            continue;
        }
        if (key == "rttPerAv") {
            state["obs"]["rttPerAv"] = std::min(averageRttPer, 5.0);
            continue;
        }
        if (key == "rttPrevPerAv") {
            state["obs"]["rttPrevPerAv"] = prevState["obs"]["rttPerAv"];
            continue;
        }
        if (key == "rttJitterPerAv") {
            state["obs"]["rttJitterPerAv"] = std::min(averageRttJitterPer, 5.0);
            continue;
        }
        if (key == "rttGlobAv") {
            state["obs"]["rttGlobAv"] = std::min(averageRttGlob, 5.0);
            continue;
        }
        if (key == "rttJitterGlobAv") {
            state["obs"]["rttJitterGlobAv"] = std::min(averageRttJitterGlob, 5.0);
            continue;
        }
        if (key == "plrPer") {
            state["obs"]["plrPer"] = packetLossRatePer;
            continue;
        }
        if (key == "plrPrevPer") {
            state["obs"]["plrPrevPer"] = prevState["obs"]["plrPer"];
            continue;
        }
        if (key == "plrGlob") {
            state["obs"]["plrGlob"] = packetLossRate;
            continue;
        }
        if (key == "txPacketsPer") {
            state["obs"]["rxPacketsPer"] = txPacketsPer;
            continue;
        }
        if (key == "rxPacketsPer") {
            state["obs"]["rxPacketsPer"] = rxPacketsPer;
            continue;
        }
        if (key == "chunkVar") {
            state["obs"]["chunkVar"] = std::min(relStdChunkSize, 1.0);
            continue;
        }
        if (key == "netInstabilRate") {
            state["obs"]["netInstabilRate"] = networkInstabilityRate;
            continue;
        }
        if (key == "qualChangeRate") {
            state["obs"]["qualChangeRate"] = qualityChangeRate;
            continue;
        }
        if (key == "isRunning") {
            state["obs"]["isRunning"] = isRunning ? 1 : -1;
            continue;
        }

        // currently not used
        if (key == "queueingTimeAv") {
            state["obs"]["queueingTimeAv"] = std::min(queueingTimesSum / (double)chunksSent, 5.0);
            continue;
        }
        if (key == "encodingTimeAv") {
            state["obs"]["encodingTimeAv"] = std::min(encodingTimesSum / (double)chunksSent, 5.0);
            continue;
        }
        if (key == "relThroughputRate") {
            continue;
        }
        if (key == "delayPerAv") {
            state["obs"]["delayPerAv"] = std::min(averageDelayPer, 5.0);
            continue;
        }
        if (key == "delayGlobAv") {
            state["obs"]["delayGlobAv"] = std::min(averageDelayGlob, 5.0);
            continue;
        }
        if (key == "delayJitterPerAv") {
            state["obs"]["delayJitterPerAv"] = std::min(averageDelayJitterPer, 5.0);
            continue;
        }
        if (key == "delayJitterGlobAv") {
            state["obs"]["delayJitterGlobAv"] = std::min(averageDelayJitterGlob, 5.0);
            continue;
        }
    }
    statesMade ++;
    stateLastTimestamp = omnetpp::simTime();
    return state;
*/
#include "StreamState.h"

StreamState::StreamState()
    : elemsSent(0)
    , lastAction(-1)
    , actionsTaken(0)
    , isQualityChanged(false)
    , fecCounter(0)
    , firstFecSequenceNumber(0) {
    lastQualities = boost::circular_buffer<double>();
}

StreamState::~StreamState() {
    clearBuffers();
    lastQualities.clear();
}

void StreamState::set(const std::string& setStreamName, int setHistorySize) {
    streamName = setStreamName;
    assert(setHistorySize > 0);
    historySize = setHistorySize;
    lastQualities.set_capacity(historySize);
}

bool StreamState::getIsQualityChanged() {
    if (isQualityChanged) {
        // reset this parameter
        isQualityChanged = false;
        return true;
    } else {
        return false;
    }
}

int StreamState::getQualityChanges(int lookup) {
    int qualityChanges = 0;
    int qualitesCollected = std::min((int)lastQualities.size(), lookup);
    for (int i = (int)lastQualities.size() - 1; i >= (int)lastQualities.size() - qualitesCollected; i--) {
        if (i > (int)lastQualities.size() - qualitesCollected && lastQualities[i] != lastQualities[i - 1]) {
            qualityChanges ++;
        }
    }
    return qualityChanges;
}


bool StreamState::isTimeToSendFec(int currentFecAfterPackets, bool isHigh) {
    fecCounter ++;
    bool isFec = false;
    if (isHigh) {
        // so far high/not high uneven protection differs with a fixed two-times ratio
        isFec = fecCounter > 0 && fecCounter % (currentFecAfterPackets / 2) == 0;
    } else {
        isFec = fecCounter > 0 && fecCounter % currentFecAfterPackets == 0;
    }
    if (isFec) {
        fecCounter = 0;
    }
    return isFec;
}

// UPDATERS
void StreamState::updateElemSent(int elemSize, time_t queueingTime, time_t encodingTime) {
    elemsSent ++;
    lastQueueingTimes.push_back(queueingTime);
    lastEncodingTimes.push_back(encodingTime);
    lastElems.push_back(elemSize);
}

// SERIALIZER
nlohmann::json StreamState::toJson() {
    nlohmann::json sstJson;
    sstJson["streamName"] = streamName;
    sstJson["elemsSent"] = elemsSent;

    // copy vectors and erase them
    std::vector<int> elems(lastElems.begin(), lastElems.end());
    std::vector<double> qualities(lastQualities.begin(), lastQualities.end());
    std::vector<time_t> encodingTimes(lastEncodingTimes.begin(), lastEncodingTimes.end());
    std::vector<time_t> queueingTimes(lastQueueingTimes.begin(), lastQueueingTimes.end());
    sstJson["elems"] = elems;
    sstJson["qualities"] = qualities;
    sstJson["encodingTimes"] = encodingTimes;
    sstJson["queueingTimes"] = queueingTimes;
    clearBuffers();
    return sstJson;
}

void StreamState::clearBuffers() {
    lastElems.clear();
    lastEncodingTimes.clear();
    lastQueueingTimes.clear();
}
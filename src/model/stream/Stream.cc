#include "Stream.h"


// StreamQuality

StreamQuality::StreamQuality(int initQuality, int minQuality, int maxQuality, int initEncodingSpeed,
                             int minEncodingSpeed, int maxEncodingSpeed)
    : currQuality(initQuality), minQuality(minQuality), maxQuality(maxQuality)
    , currEncodingSpeed(initEncodingSpeed), minEncodingSpeed(minEncodingSpeed), maxEncodingSpeed(maxEncodingSpeed) {}

std::pair<VideoQuality, VideoEncodingSpeed> StreamQuality::getVideoPreset() {
    return std::make_pair(getVideoQualityByIndex(currQuality), getVideoEncodingSpeedByIndex(currEncodingSpeed));
}

std::vector<std::pair<VideoQuality, VideoEncodingSpeed>> StreamQuality::getAllVideoPresets() {
    // FIXME: so far we don't vary encoding speed, we work with the initial one
    std::vector<std::pair<VideoQuality, VideoEncodingSpeed>> presets;
    for (int quality = 0; quality <= std::min(maxQuality, VIDEO_QUALITY_MAX - 1); quality++) {
        presets.push_back(std::make_pair(getVideoQualityByIndex(quality), getVideoEncodingSpeedByIndex(currEncodingSpeed)));
    }
    return presets;
}

std::pair<PointCloudQuality, bool> StreamQuality::getLidarPreset() {
    return std::make_pair(getPointCloudQualityByIndex(currQuality),
                          currEncodingSpeed > (maxEncodingSpeed - minEncodingSpeed + 1) / 2);
}

std::vector<std::pair<PointCloudQuality, bool>> StreamQuality::getAllLidarPresets() {
    std::vector<std::pair<PointCloudQuality, bool>> presets;
    for (int quality = 0; quality <= std::min(maxQuality, POINT_CLOUD_QUALITY_MAX - 1); quality++) {
        presets.push_back(std::make_pair(getPointCloudQualityByIndex(quality),
                                         currEncodingSpeed > (maxEncodingSpeed - minEncodingSpeed + 1) / 2));
    }
    return presets;
}

// StreamTimer

StreamTimer::StreamTimer(double sendInterval, const std::string& timestamps) : sendInterval(sendInterval) {
    if (sendInterval != 0 && timestamps != "") {
        std::cerr << "StreamTimer::StreamTimer set interval to 0 or don't use timestamps file; send interval will be used" <<
                  std::endl;
        isPeriodic = true;
        isFromFile = false;
    }
    if (sendInterval == 0 && timestamps == "") {
        std::cerr << "StreamTimer::StreamTimer set either interval not 0 or use timestamps file" << std::endl;
        return;
    }
    if (sendInterval == 0 && timestamps != "") {
        std::ifstream ifs(timestamps, std::ios::in);
        if (!ifs) {
            std::cerr << "StreamTimer::StreamTimer error opening timestamps file" << std::endl;
            return;
        }
        for (std::string line; std::getline(ifs, line);) {
            timestampsVec.push_back(std::stod(line));
        }
        timestampsOffset = timestampsVec[0] < 0 ? -timestampsVec[0] : 0;
        timestampsIndex = 0;
        isFromFile = true;
        isPeriodic = false;
    }
    if (sendInterval != 0 && timestamps == "") {
        isPeriodic = true;
        isFromFile = false;
    }
    startTime = omnetpp::simTime();
}

omnetpp::simtime_t StreamTimer::getNextSendingTime() {
    if (isPeriodic) {
        return omnetpp::simTime() + sendInterval;
    }
    if (isFromFile) {
        if (timestampsIndex < (int)timestampsVec.size()) {
            double nextTimestamp = timestampsVec[timestampsIndex] + timestampsOffset;
            timestampsIndex ++;
            return omnetpp::simTime() + nextTimestamp;
        } else {
            std::cerr << "StreamTimer::getNextSendingTime there are no more timestamps in file!" << std::endl;
            return 0;
        }
    } else {
        throw std::runtime_error("StreamTimer::getNextSendingTime StreamTimer should have set either isPeriodic or isFromFile as true");
    }
}

void StreamTimer::pauseStream() {
    timeInactiveStart = omnetpp::simTime().dbl();
}

void StreamTimer::resumeStream() {
    timeInactive += omnetpp::simTime().dbl() - timeInactiveStart;
    timeInactiveStart = 0;
}

// StreamDataGenerator

StreamDataGenerator::StreamDataGenerator(
    double dataRate,
    double dataRateMin,
    double dataRateMax,
    double sendInterval,
    double dataRateVariance,
    double dataRateFluctuationChance,
    double keyFramePeriod,
    std::vector<int>& qualityPresets
)
    : dataRate(dataRate)
    , dataRateInitial(dataRate)
    , dataRateMin(dataRateMin)
    , dataRateMax(dataRateMax)
    , dataRateVariance(dataRateVariance)
    , dataRateFluctuationChance(dataRateFluctuationChance)
    , qualityPresets(qualityPresets) {

    if (dataRateVariance < 0 || dataRateVariance > 1 || dataRateFluctuationChance < 0 || dataRateFluctuationChance > 1) {
        throw std::invalid_argument("please check the arguments for a SIM stream");
    }
    if (!qualityPresets.empty() && dataRateMax < qualityPresets.back()) {
        throw std::invalid_argument("max rate in kbps in qualityPresets is higher than dataRateMax");
    }
    // dataRate is given in KBits, transform from kbits to bytes
    double bytesPerSecond = dataRate * 125;
    // set vars
    framesPerSecond = 1 / sendInterval;
    frameSize = bytesPerSecond / framesPerSecond;
    keyFrameInterval = (int)(framesPerSecond * keyFramePeriod);
    isKeyFrame = keyFrameInterval > 0; // always start with a keyFrame as in real video
    // set random dists
    randomEngine = std::default_random_engine{std::random_device{}()};
    dataRateVarianceDist = std::uniform_real_distribution<double>(1 - dataRateVariance, 1 + dataRateVariance);
    dataRateFluctuationDist = std::uniform_real_distribution<double>(0.0, 1.0);
}

int StreamDataGenerator::generateFrameSize() {
    if (isKeyFrame) {
        return (int)(frameSize * 4.0);
    }
    auto thisFluctuationChance = dataRateFluctuationDist(randomEngine);
    if (thisFluctuationChance > dataRateFluctuationChance) {
        return (int)frameSize;
    } else {
        double thisFluctuation = dataRateVarianceDist(randomEngine);
        double newPacketSize = frameSize * thisFluctuation;
        return (int)newPacketSize;
    }
}

double StreamDataGenerator::getAbsoluteRate(double relativeRate) {
    if (relativeRate < -1 || relativeRate > 1) {
        throw std::invalid_argument("StreamDataGenerator::getAbsoluteRate relativeRate should be within [-1, 1]");
    }
    return 0.5 * (relativeRate + 1) * (dataRateMax - dataRateMin) + dataRateMin;
}

void StreamDataGenerator::updateRate(double newRate) {
    dataRate = std::clamp(newRate, dataRateMin, dataRateMax);
    frameSize = (dataRate * 125) / framesPerSecond;
}

void StreamDataGenerator::updateRateWithQualityPreset(int qualityPresetIndex) {
    if (!qualityPresets.empty()) {
        if (qualityPresetIndex < 0 || qualityPresetIndex >= (int)qualityPresets.size()) {
            throw std::invalid_argument("StreamDataGenerator::updateRateWithQualityPreset quality preset index is wrong");
        }
        dataRate = qualityPresets[qualityPresetIndex];
        double newBytesPerSecond = qualityPresets[qualityPresetIndex] * 125;
        frameSize = newBytesPerSecond / framesPerSecond;
    }
}

bool StreamDataGenerator::checkKeyFrame(int framesSent) {
    isKeyFrame = keyFrameInterval > 0 ? framesSent % keyFrameInterval == 0 : false;
    return isKeyFrame;
}

int StreamDataGenerator::getQualityPresetIndexByRate(int rateKbps) {
    if (qualityPresets.size() > 1) {
        if (rateKbps >= qualityPresets.back()) {
            return qualityPresets.size() - 1;
        } else {
            int left = 0;
            int right = 1;
            for (int i = 0; i < (int)qualityPresets.size() - 1; i++) {
                if (qualityPresets[i] <= rateKbps && rateKbps < qualityPresets[i + 1]) {
                    left = i;
                    right = i + 1;
                    break;
                }
            }
            int lrMedian = (qualityPresets[right] - qualityPresets[left]) / 2;
            if (rateKbps > lrMedian + qualityPresets[left]) {
                return right;
            } else {
                return left;
            }
        }
    } else {
        return 0;
    }
}

void StreamDataGenerator::resetToDefaultRate() {
    frameSize = dataRateInitial * 125.0 / framesPerSecond;
}

double StreamDataGenerator::getRate() {
    return dataRate;
}

double StreamDataGenerator::getMinRate() {
    return dataRateMin;
}

double StreamDataGenerator::getMaxRate() {
    return dataRateMax;
}

/*
 *
 * Stream class implementation
 *
 */

Stream::Stream(StreamType type, std::string name, int typeId, int globalId)
    : name(name), type(type), typeId(typeId), globalId(globalId)
    , maxPacketSize(64000)
    , isRunning(false)
    , verbose(0) {
    quality = StreamQuality();
    timer = StreamTimer();
    dataGen = StreamDataGenerator();
    state = UnitState();
}

Stream::~Stream() { if (logger) delete logger; }

// SET OOP-PLACEHOLDERS

void Stream::set(StreamQuality setQuality, StreamTimer setTimer, StreamDataGenerator setDataGen,
                 int setMaxPacketSize, int setHistorySize, int setVerbose, const std::string& setLogdir) {
    // validate quality
    if (setQuality.minQuality <= setQuality.currQuality  && setQuality.currQuality <= setQuality.maxQuality
            && 0 <= setQuality.minQuality && setQuality.minQuality < setQuality.maxQuality && setQuality.maxQuality <= 5
            && setQuality.minEncodingSpeed <= setQuality.currEncodingSpeed
            && setQuality.currEncodingSpeed <= setQuality.maxEncodingSpeed
            && 0 <= setQuality.minEncodingSpeed && setQuality.minEncodingSpeed < setQuality.maxEncodingSpeed
            && setQuality.maxEncodingSpeed <= 4) {
        quality = setQuality;
    } else {
        throw std::invalid_argument("Stream::set quality parameter is incorrect");
    }

    // validate timer
    if ((setTimer.sendInterval > 0.0 && setTimer.timestampsFile == "") || (setTimer.sendInterval == 0.0
            && setTimer.timestampsFile != "")) {
        timer = setTimer;
    } else {
        throw std::invalid_argument("Stream::set timer parameter is incorrect");
    }

    // validate data gen
    if (type == SIM) {
        if (setDataGen.dataRate > 0.0 && setDataGen.dataRateVariance >= 0 && setDataGen.dataRateVariance <= 1
                && setDataGen.dataRateFluctuationChance >= 0 && setDataGen.dataRateFluctuationChance <= 1) {
            dataGen = setDataGen;
        } else {
            throw std::invalid_argument("Stream::set dataGen parameter is incorrect");
        }
    }

    // validate max packet size, should be less than max UDP datagram
    if (setMaxPacketSize > 0 && setMaxPacketSize < 65000) {
        maxPacketSize = setMaxPacketSize;
    } else {
        throw std::invalid_argument("Stream::set maxPacket parameter is incorrect");
    }

    // validate history size and set verbose mode
    if (setHistorySize > 0) {
        verbose = std::max(0, std::min(2, setVerbose));
        state.set(name, setHistorySize, verbose);
    } else {
        throw std::invalid_argument("Stream::set history parameter is incorrect");
    }

    // logging enabled
    if (setVerbose >= 1) {
        createLogger(setLogdir);
    }
}

// SAVE ITERATION

void Stream::checkNetworkInstability() {
    std::unique_lock<std::mutex> lock(streamMutex);
    state.getTransmissionState().checkNetworkInstability();
}

void Stream::updateWithPacket(const inet::Ptr<const inet::Chunk>& packet, PacketType packetType, int twNumber) {
    std::unique_lock<std::mutex> lock(streamMutex);
    state.getTransmissionState().updateWithPacket(packet, packetType, twNumber);
}

void Stream::updateElemSent(int elemSize, time_t queueingTime, time_t encodingTime) {
    std::unique_lock<std::mutex> lock(streamMutex);
    if (!isRunning) {
        std::cerr << "Stream::updateElemSent stream "  << name << " isn't running now" << std::endl;
        return;
    }
    state.getStreamState().updateElemSent(elemSize, queueingTime, encodingTime);
}

// REDUNDANCY (FEC)

bool Stream::isTimeToSendFec(int currentFecAfterPackets, bool isHigh) {
    std::unique_lock<std::mutex> lock(streamMutex);
    return state.getStreamState().isTimeToSendFec(currentFecAfterPackets, isHigh);
}

// LOGGING

void Stream::createLogger(const std::string& logdir) {
    logger = nullptr;
    if (logdir != "null") {
        std::string logfile;
        if (logdir.empty() || logdir == ".") {
            // in the current folder, create multiple files for each run with timestamps
            logfile = "stream_" + name + "_";
        } else {
            // redirected, append to a single file, give .csv extension
            logfile = logdir + "stream_" + name + ".csv";
        }
        logger = new CSVLogger(logfile);
    }
}

void Stream::log(int runNumber, double timestamp) {
    // so far logging only data transmission state to csv file, may be enchaning it later
    std::unique_lock<std::mutex> lock(streamMutex);
    DataTransmissionState& dataTransmissionState = state.getTransmissionState();
    if (logger) {
        if (!logger->getIsHeaderWritten()) {
            *logger <<
                    "run" <<
                    "time" <<
                    "lastAction" <<
                    "duration" <<
                    "fractionTxRate" <<
                    "fractionRxRate" <<
                    "txPackets" <<
                    "rxPackets" <<
                    "txMBytes" <<
                    "rxMBytes" <<
                    "lostPackets" <<
                    "oooPackets" <<
                    "lossRate" <<
                    "fractionLossRate" <<
                    "rtt" <<
                    "interarrivalJitter" <<
                    "timeCurrentReceiverReport" <<
                    "timeCurrentTransportFeedback" << CSVLogger::endl;
        } else {
            *logger <<
                    runNumber <<
                    timestamp <<
                    state.getStreamState().lastAction <<
                    dataTransmissionState.txDuration <<
                    dataGen.getRate() / 1000.0 <<
                    dataTransmissionState.rxGoodput <<
                    dataTransmissionState.numTxPackets <<
                    dataTransmissionState.numRxPackets <<
                    dataTransmissionState.numTxBytes * 1e-6 <<
                    dataTransmissionState.numRxBytes * 1e-6 <<
                    dataTransmissionState.numLostPackets <<
                    dataTransmissionState.numOutOfOrderPackets <<
                    dataTransmissionState.lossRate <<
                    dataTransmissionState.fractionLossRate <<
                    dataTransmissionState.rtt <<
                    dataTransmissionState.interarrivalJitter <<
                    dataTransmissionState.timeCurrentReceiverReport.dbl() <<
                    dataTransmissionState.timeCurrentTransportFeedback.dbl() <<
                    CSVLogger::endl;
        }
    }
}

// DOWNCALLERS FOR QUALITY AND TIME PLACEHOLDERS

int Stream::getQualityChanges(int lookup) {
    std::unique_lock<std::mutex> lock(streamMutex);
    return state.getStreamState().getQualityChanges(lookup);
}

bool Stream::getIsQualityChanged() {
    std::unique_lock<std::mutex> lock(streamMutex);
    return state.getStreamState().getIsQualityChanged();
}

std::pair<VideoQuality, VideoEncodingSpeed> Stream::getVideoPreset() {
    return quality.getVideoPreset();
}

std::vector<std::pair<VideoQuality, VideoEncodingSpeed>> Stream::getAllVideoPresets() {
    return quality.getAllVideoPresets();
}

std::pair<PointCloudQuality, bool> Stream::getLidarPreset() {
    return quality.getLidarPreset();
}

std::vector<std::pair<PointCloudQuality, bool>> Stream::getAllLidarPresets() {
    return quality.getAllLidarPresets();
}

int Stream::getSimPreset() {
    return quality.currQuality;
}

omnetpp::simtime_t Stream::getNextSendingTime() {
    return timer.getNextSendingTime();
}

double Stream::getSendInterval() {
    return timer.sendInterval;
}

double Stream::getTimeElapsed() {
    return (omnetpp::simTime() - timer.startTime).dbl();
}

double Stream::getTimeInactive() {
    double ifNowIsInactive = timer.timeInactiveStart > 0 ? omnetpp::simTime().dbl() - timer.timeInactiveStart : 0;
    return timer.timeInactive + ifNowIsInactive;
}

int Stream::getPacketsSent() {
    std::unique_lock<std::mutex> lock(streamMutex);
    return state.getTransmissionState().numTxPackets;
}

void Stream::runStream() {
    isRunning = true;
}

void Stream::pauseStream() {
    timer.pauseStream();
    isRunning = false;
}

void Stream::resumeStream() {
    timer.resumeStream();
    isRunning = true;
}

void Stream::stopStream() {
    isRunning = false;
}

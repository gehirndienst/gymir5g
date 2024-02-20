#include "StreamManager.h"

StreamManager::StreamManager(): packetsSent_(0) {}

StreamManager::~StreamManager() {
    free();
    std::cout << "[StreamManager::~StreamManager] StreamManager is correctly destroyed.." << std::endl;
}

// INIT

void StreamManager::setAllStreams(nlohmann::json& streamConfigsJson, const std::string& logdir) {
    std::vector<StreamInitConfig> streamInitConfigs = getStreamConfigs(streamConfigsJson);

    StreamInitConfig config;
    std::string streamName;
    StreamType streamType;

    for (auto it = streamInitConfigs.begin(); it != streamInitConfigs.end(); ++it) {
        config = *it;
        streamName = config.streamName;
        streamType = config.streamType;

        // check if registered
        if (!isRegistered(streamName)) {
            throw std::invalid_argument("StreamManager::initAllStreams Stream " + streamName + " is not registered");
        }

        // create quality and time placeholders and history size
        StreamTimer timer = StreamTimer(config.sendInterval, config.timestampsFile);
        StreamQuality quality = StreamQuality(
                                    config.initQuality,
                                    config.minQuality,
                                    config.maxQuality,
                                    config.initEncodingSpeed,
                                    config.minEncodingSpeed,
                                    config.maxEncodingSpeed
                                );
        StreamDataGenerator dataGen = StreamDataGenerator(
                                          config.dataRate,
                                          config.dataRateMin,
                                          config.dataRateMax,
                                          config.sendInterval,
                                          config.dataRateVariance,
                                          config.dataRateFluctuationChance,
                                          config.keyFramePeriod,
                                          config.qualityPresets
                                      );

        // set streams
        streams_[streamName]->set(quality, timer, dataGen, config.maxPacketSize, config.historySize, config.verbose, logdir);

        // init stream data
        if (streamType == VIDEO) {
            setVideoStream(streamName, config.source, config.bufferLimit, config.maxPacketSize, config.verbose);
        } else if (streamType == LIDAR) {
            setLidarStream(streamName, config.source, config.bufferLimit, config.maxPacketSize, config.verbose);
        } else if (streamType == SIM) {
            setSimStream(streamName, config.maxPacketSize, config.verbose);
        } else {
            //TODO: add option for a BLOB stream type
            continue;
        }
    }
}

void StreamManager::runAllStreams() {
    for (auto const& [streamName, stream] : streams_) {
        if (streams_[streamName]->type == VIDEO) {
            runVideoStream(const_cast<std::string&>(streamName));
        } else if (streams_[streamName]->type == LIDAR) {
            runLidarStream(const_cast<std::string&>(streamName));
        } else {
            continue;
            // same as above
        }
    }
}

// DATA

void StreamManager::fillStreamData(std::string& streamName, bool isWaiting) {
    if (!isRegistered(streamName)) {
        throw std::invalid_argument("StreamManager::initAllStreams Stream " + streamName + " is not registered");
    }
    auto& stream = getStream(streamName);
    stream->data = {};
    StreamType type = stream->type;

    if (type == VIDEO) {
        std::optional<VideoData> elem;
        elem = vCapturers_[streamName]->pop(isWaiting);

        // check if we have some data
        if (!elem.has_value()) {
            stream->data = {};
            return;
        }
        // fix pop time
        auto popTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>
                            (std::chrono::system_clock::now().time_since_epoch()).count();

        // extract metadata from raw data
        auto frame = elem.value().frame;
        auto captureTimestamp = elem.value().metadata.captureTime;
        int elemNumber = elem.value().metadata.frameNum;

        // select current encoder
        auto& encoder = vEncoders_[streamName];

        // encode data
        encoder->encode(frame.data);
        bool isKeyFrame = encoder->isKeyFrame();
        auto encodingTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>
                                 (std::chrono::system_clock::now().time_since_epoch()).count();

        // get final data for transmission
        stream->data = {
            streamName,
            type,
            elemNumber,
            encoder->getFrameSize(),
            encoder->getNals(),
            std::make_unique<QualityOfContent>(encoder->getQov()),
            captureTimestamp,
            popTimestamp,
            encodingTimestamp,
            isKeyFrame
        };
    } else if (type == LIDAR) {
        std::optional<PointCloudData> elem;
        elem = lGrabbers_[streamName]->pop(isWaiting);

        // check if we have some data
        if (!elem.has_value()) {
            stream->data = {};
            return;
        }
        // fix pop time
        auto popTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>
                            (std::chrono::system_clock::now().time_since_epoch()).count();

        // extract metadata from raw data
        auto cloud = elem.value().cloud;
        auto captureTimestamp = elem.value().metadata.grabTime;
        int elemNumber = elem.value().metadata.cloudNum;

        // select current encoder
        auto& encoder = lEncoders_.at(streamName);

        // encode data
        encoder->encode(cloud);
        auto encodingTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>
                                 (std::chrono::system_clock::now().time_since_epoch()).count();

        // get final data for transmission
        stream->data = {
            streamName,
            type,
            elemNumber,
            encoder->getEncodedDataSize(),
            encoder->getEncodedPointClouds(),
            nullptr,
            captureTimestamp,
            popTimestamp,
            encodingTimestamp,
            false
        };
    } else if (type == SIM) {
        StreamDataGenerator& dataGenerator = streams_[streamName]->dataGen;
        bool isKeyFrame = dataGenerator.checkKeyFrame(streams_[streamName]->state.getStreamState().elemsSent);
        int dataSize = dataGenerator.generateFrameSize();
        std::vector<std::pair<uint8_t*, int>> data;

        int maxPacketSize = streams_[streamName]->maxPacketSize;
        int packetsNum = std::ceil(static_cast<float>(dataSize) / maxPacketSize);
        int offset = 0;
        for (int i = 0; i < packetsNum; i++) {
            int packetSize = std::min(std::abs(dataSize - offset), maxPacketSize);
            data.push_back(std::make_pair(nullptr, packetSize));
            offset += packetSize;
        }

        stream->data = {
            streamName,
            type,
            getStreamElemsSent(streamName) + 1,
            dataSize,
            data,
            nullptr,
            0,
            0,
            0,
            isKeyFrame
        };
    } else {
        // expand for new stream types
        stream->data = {};
        return;
    }
}

StreamData StreamManager::generateFutureStreamData(std::string& streamName) {
    fillStreamData(streamName);
    return getStreamData(streamName);
}

// ADAPTIVE STREAMING

void StreamManager::setAdaptiveStreaming(AdaptiveAlgorithm algorithm) {
    for (auto& streamName : getStreamNames()) {
        streamActors_.emplace(streamName, AdaptiveStreamingFactory::create(algorithm));
    }
}

std::vector<nlohmann::json> StreamManager::getStates() {
    std::vector<nlohmann::json> states;
    for (auto& streamName : getStreamNames()) {
        UnitState& internalState = streams_[streamName]->state;
        nlohmann::json& state = streamActors_[streamName]->makeState(internalState);
        if (!getStreamIsRunning(streamName)) {
            state["off"] = true;
        }
        states.push_back(state);
    }
    return states;
}

// TRANSMISSION STATE

void StreamManager::updateWithPacket(std::string& streamName, const inet::Ptr<const inet::Chunk>& packet,
                                     PacketType packetType, int twNumber) {
    streams_[streamName]->updateWithPacket(packet, packetType, twNumber);
}

void StreamManager::checkNetworkInstability() {
    for (auto& it : streams_) {
        it.second->checkNetworkInstability();
    }
}

// RETRANMISSION BUFFERS

void StreamManager::setRetransmissionBuffers(double maxRetransmitAge) {
    if (maxRetransmitAge > 0) {
        for (auto& streamName : getStreamNames()) {
            getStreamRetranmissionBuffer(streamName).setMaxAge(maxRetransmitAge);
        }
    }
}

// REDUNDAANCY (FEC)

bool StreamManager::isTimeToSendFec(std::string& streamName, int currentFecAfterPackets, bool isHigh) {
    return streams_[streamName]->isTimeToSendFec(currentFecAfterPackets, isHigh);
}

void StreamManager::setNewFirstFecSequenceNumber(std::string& streamName, int sequenceNumber) {
    streams_[streamName]->state.getStreamState().firstFecSequenceNumber = sequenceNumber;
}

// ACTIONS

void StreamManager::applyAction(std::string& streamName, int action) {
    std::unique_ptr<Stream>& stream = streams_[streamName];
    streamActors_[streamName]->applyAction(stream, action);
    // quality actions
    if (stream->getIsQualityChanged()) {
        if (stream->type == VIDEO) {
            std::pair<VideoQuality, VideoEncodingSpeed> encodingPair = stream->getVideoPreset();
            vEncoders_[streamName]->init(encodingPair.first, encodingPair.second,
                                         vCapturers_[streamName]->srcSize.first,
                                         vCapturers_[streamName]->srcSize.second);
            if (stream->verbose > 1) {
                std::cout << "StreamManager::applyAction stream " << streamName
                          << " switched video encoder to quality "
                          << getVideoQualityStringByIndex(stream->quality.currQuality)
                          << " in [" << getVideoQualityStringByIndex(stream->quality.minQuality)
                          << ", " << getVideoQualityStringByIndex(stream->quality.maxQuality)
                          << "] range" << std::endl;
            }
        } else if (stream->type == LIDAR) {
            std::pair<PointCloudQuality, bool> encodingPair = stream->getLidarPreset();
            lEncoders_[streamName]->init(encodingPair.first, encodingPair.second);
            if (stream->verbose > 1) {
                std::cout << "StreamManager::applyAction stream " << streamName
                          << " switched PCL encoder to quality "
                          << getPointCloudQualityStringByIndex(stream->quality.currQuality)
                          << " in [" << getPointCloudQualityStringByIndex(stream->quality.minQuality)
                          << ", " << getPointCloudQualityStringByIndex(stream->quality.maxQuality)
                          << "] range" << std::endl;
            }
        } else if (stream->type == SIM) {
            int currentQuality = stream->getSimPreset();
            stream->dataGen.updateRateWithQualityPreset(currentQuality);
        }
    }
}

void StreamManager::applyAction(std::string& streamName, double action) {
    // NOTE: for now we don't have any quality actions for double values for real streams, only for SIM
    // for SIM stream the new rate is set in StreamActions.setRate method, so nothing to do here
    std::unique_ptr<Stream>& stream = streams_[streamName];
    streamActors_[streamName]->applyAction(stream, action);
}

// EVENTS SAVING

void StreamManager::updateElemSent(std::string& streamName, int elemSize,
                                   time_t queueingTime, time_t encodingTimestamp) {
    streams_[streamName]->updateElemSent(elemSize, queueingTime, encodingTimestamp);
}

void StreamManager::updatePacketSent() {
    std::unique_lock<std::mutex> lock(globalPacketsMutex_);
    packetsSent_ ++;
}

// STREAM LOGGING

void StreamManager::logStreams(int runNumber, double timestamp) {
    for (auto& streamName : getStreamNames()) {
        if (getStreamVerbose(streamName) >= 1) {
            streams_[streamName]->log(runNumber, timestamp);
        }
    }
}

// PRIVATE TRANSFORMERS

std::vector<StreamInitConfig> StreamManager::getStreamConfigs(nlohmann::json& streamConfigsJson) {
    // get configs for all streams from json file and register them
    std::vector<StreamInitConfig> sicVector;
    std::vector<std::tuple<std::string, StreamType>> sdVector;
    auto sicJsonObj = streamConfigsJson["streamConfigs"].get<std::vector<nlohmann::json>>();
    for (const auto& sicJson : sicJsonObj) {
        std::string streamName = std::string(sicJson["streamName"]);
        StreamType streamType = streamtype::fromString(std::string(sicJson["streamType"]));
        sdVector.push_back(std::make_tuple(streamName, streamType));

        StreamInitConfig sic = {
            streamName,
            streamType,
            std::filesystem::absolute(std::filesystem::path(std::string(sicJson["source"]))).lexically_normal().string(),
            (int)sicJson["bufferLimit"],
            (int)sicJson["maxPacketSize"],
            (int)sicJson["initQuality"],
            (int)sicJson["minQuality"],
            (int)sicJson["maxQuality"],
            (int)sicJson["initEncodingSpeed"],
            (int)sicJson["minEncodingSpeed"],
            (int)sicJson["maxEncodingSpeed"],
            (double)sicJson["sendInterval"],
            std::string(sicJson["timestampsFile"]), //FIXME: need to be fixed later
            (int)sicJson["historySize"],
            (double)sicJson["dataRate"],
            (double)sicJson["dataRateMin"],
            (double)sicJson["dataRateMax"],
            (double)sicJson["dataRateVariance"],
            (double)sicJson["dataRateFluctuationChance"],
            (double)sicJson["keyFramePeriod"],
            (std::vector<int>)sicJson["qualityPresets"],
            (int)sicJson["verbose"]
        };
        sicVector.push_back(sic);
    }

    // register all streams and assign them ids within a manager
    registerAllStreams(sdVector);

    return sicVector;
}

void StreamManager::registerAllStreams(std::vector<std::tuple<std::string, StreamType>>& streamDeclarations) {
    int videoStreamsNumber = 0;
    int lidarStreamsNumber = 0;
    int blobStreamsNumber = 0;
    int simStreamsNumber = 0;
    int globalId = 0;
    for (auto& streamDeclaration : streamDeclarations) {
        std::string streamName = std::get<0>(streamDeclaration);
        StreamType streamType = std::get<1>(streamDeclaration);
        int typeId = 0;
        switch (streamType) {
            case VIDEO:
                typeId = videoStreamsNumber;
                videoStreamsNumber ++;
                break;
            case LIDAR:
                typeId = lidarStreamsNumber;
                lidarStreamsNumber ++;
                break;
            case BLOB:
                typeId = blobStreamsNumber;
                blobStreamsNumber ++;
                break;
            case SIM:
                typeId = simStreamsNumber;
                simStreamsNumber ++;
                break;
            default: break;
        }

        Stream* stream = new Stream(streamType, streamName, typeId, globalId);
        globalId ++;
        streams_.insert(std::make_pair(streamName, stream));
    }
}

// STREAM DATA SETTERS

void StreamManager::setVideoStream(std::string& streamName, std::string& source, int bufferLimit, int maxPacketSize,
                                   int verbose) {
    // set capturer
    auto capturer = std::make_shared<VideoCapturer>(source, bufferLimit, verbose);
    std::pair<int, int> srcSize = capturer->srcSize;
    if (capturer->isOk)
        vCapturers_.insert(std::make_pair(streamName, std::move(capturer)));
    else
        std::cerr << "StreamManager::setVideoStream can't set a capturer for the source: " << source << std::endl;

    // init encoder
    auto encoder = std::make_unique<VideoEncoderH264>(maxPacketSize, verbose);
    std::pair<VideoQuality, VideoEncodingSpeed> preset = streams_[streamName]->getVideoPreset();
    encoder->init(preset.first, preset.second, srcSize.first, srcSize.second);
    if (encoder->isOk)
        vEncoders_.insert(std::make_pair(streamName, std::move(encoder)));
    else
        std::cerr << "StreamManager::initVideoStream can't init an encoder for the source: " << source << std::endl;

    streams_[streamName]->runStream();
}

void StreamManager::setLidarStream(std::string& streamName, std::string& source, int bufferLimit, int maxPacketSize,
                                   int verbose) {
    // set grabber
    // TODO: expand for the case ip;port
    auto grabber = std::make_shared<VLPLidarGrabber>(source, bufferLimit, verbose);
    if (grabber->isOk)
        lGrabbers_.insert(std::make_pair(streamName, std::move(grabber)));
    else
        std::cerr << "StreamManager::initLidarStream can't set a grabber for the source: " << source << std::endl;

    // init encoder
    auto encoder = std::make_unique<PointCloudEncoder>(maxPacketSize, verbose);
    std::pair<PointCloudQuality, bool> preset = streams_[streamName]->getLidarPreset();
    encoder->init(preset.first, preset.second);
    if (encoder->isOk)
        lEncoders_.insert(std::make_pair(streamName, std::move(encoder)));
    else
        std::cerr << "StreamManager::initLidarStream can't set a compressor for the source: " << source << std::endl;

    streams_[streamName]->runStream();
}

void StreamManager::setBlobStream(std::string& streamName, std::string& source, int bufferLimit) {
    // TODO: write a class that captures blob streams (rosbags?) into queues and then sends them
}

void StreamManager::setSimStream(std::string& streamName, int maxPacketSize, int verbose) {
    streams_[streamName]->maxPacketSize = maxPacketSize;
    streams_[streamName]->verbose = verbose;
    streams_[streamName]->runStream();
}

void StreamManager::runVideoStream(std::string& streamName) {
    threads_.push_back(std::thread(&VideoCapturer::run, vCapturers_[streamName]));
}

void StreamManager::runLidarStream(std::string& streamName) {
    threads_.push_back(std::thread(&VLPLidarGrabber::run, lGrabbers_[streamName]));
}

void StreamManager::pauseStream(std::string& streamName) {
    if (!isRegistered(streamName)) {
        throw std::invalid_argument("StreamManager::pauseStream Stream " + streamName + " is not registered");
    }
    StreamType type = streams_[streamName]->type;
    if (type == VIDEO) {
        vCapturers_[streamName]->pause();
    } else if (type == LIDAR) {
        lGrabbers_[streamName]->pause();
    }
    streams_[streamName]->pauseStream();
}

void StreamManager::resumeStream(std::string& streamName) {
    if (!isRegistered(streamName)) {
        throw std::invalid_argument("StreamManager::resumeStream Stream " + streamName + " is not registered");
    }
    StreamType type = streams_[streamName]->type;
    if (type == VIDEO) {
        vCapturers_[streamName]->resume();
    } else if (type == LIDAR) {
        lGrabbers_[streamName]->resume();
    }
    streams_[streamName]->resumeStream();
}

void StreamManager::stopStream(std::string& streamName) {
    if (!isRegistered(streamName)) {
        throw std::invalid_argument("StreamManager::stopStream Stream " + streamName + " is not registered");
    }
    StreamType type = streams_[streamName]->type;
    if (type == VIDEO) {
        vCapturers_[streamName]->stop();
    } else if (type == LIDAR) {
        lGrabbers_[streamName]->stop();
    }
    streams_[streamName]->stopStream();
}

// GETTERS

std::unique_ptr<Stream>& StreamManager::getStream(std::string& streamName) {
    return streams_.at(streamName);
}

int StreamManager::getStreamGlobalId(std::string& streamName) {
    return streams_[streamName]->globalId;
}

StreamType StreamManager::getStreamType(std::string& streamName) {
    return streams_[streamName]->type;
}

StreamStatus StreamManager::getStreamStatus(std::string& streamName) {
    switch(streams_[streamName]->type) {
        case VIDEO:
            return vCapturers_[streamName]->getStatus();
        case LIDAR:
            return lGrabbers_[streamName]->getStatus();
        case BLOB: // FIXME:
            return FAIL;
        case SIM:
            return streams_[streamName]->isRunning ? RUNNING : FINISHED;
        default: break;
    }
    return FAIL;
}

StreamData& StreamManager::getStreamData(std::string& streamName) {
    return streams_[streamName]->data;
}

std::mutex& StreamManager::getStreamMutex(std::string& streamName) {
    return streams_[streamName]->streamMutex;
}

UnitState& StreamManager::getUnitState(std::string& streamName) {
    return streams_[streamName]->state;
}

int StreamManager::getStreamMaxPacketSize(std::string& streamName) {
    return streams_[streamName]->maxPacketSize;
}

DataTransmissionState& StreamManager::getStreamDataTransmissionState(std::string& streamName) {
    return streams_[streamName]->state.getTransmissionState();
}

RetransmissionBuffer& StreamManager::getStreamRetranmissionBuffer(std::string& streamName) {
    return streams_[streamName]->retransmissionBuffer;
}

int StreamManager::getStreamFirstFecSequenceNumber(std::string& streamName) {
    return streams_[streamName]->state.getStreamState().firstFecSequenceNumber;
}

omnetpp::simtime_t StreamManager::getStreamNextSendingTime(std::string& streamName) {
    return streams_[streamName]->getNextSendingTime();
}

double StreamManager::getStreamSendInterval(std::string& streamName) {
    return streams_[streamName]->getSendInterval();
}

int StreamManager::getStreamElemsAcquired(std::string& streamName) {
    switch(streams_[streamName]->type) {
        case VIDEO:
            return vCapturers_[streamName]->getFramesCaptured();
        case LIDAR:
            return lGrabbers_[streamName]->getCloudsGrabbed();
        case BLOB: // FIXME: change
            return 0;
        case SIM:
            return streams_[streamName]->state.getStreamState().elemsSent;
        default: break;
    }
    return 0;
}

int StreamManager::getStreamQueueSize(std::string& streamName) {
    switch(streams_[streamName]->type) {
        case VIDEO:
            return vCapturers_[streamName]->getQueueSize();
        case LIDAR:
            return lGrabbers_[streamName]->getQueueSize();
        case BLOB: // FIXME: change
            return 0;
        case SIM:
            return 0;
        default: break;
    }
    return 0;
}


int StreamManager::getStreamElemsSent(std::string& streamName) {
    return streams_[streamName]->state.getStreamState().elemsSent;
}

int StreamManager::getStreamActionsTaken(std::string& streamName) {
    return streams_[streamName]->state.getStreamState().actionsTaken;
}

int StreamManager::getStreamPacketsSent(std::string& streamName) {
    return streams_[streamName]->getPacketsSent();
}

boost::circular_buffer<double>& StreamManager::getStreamLastQualities(std::string& streamName) {
    return streams_[streamName]->state.getStreamState().lastQualities;
}

bool StreamManager::getStreamIsRunning(std::string& streamName) {
    return streams_[streamName]->isRunning;
}

int StreamManager::getStreamVerbose(std::string& streamName) {
    return streams_[streamName]->verbose;
}

std::string StreamManager::getStreamNameById(int globalStreamId) {
    std::string streamName;
    for (auto& it : streams_) {
        if (it.second->globalId == globalStreamId) {
            return it.second->name;
            break;
        }
    }
    if (streamName.empty()) {
        throw std::invalid_argument("StreamManager::getStreamNameById There is no stream with such ID: " + globalStreamId);
    } else {
        return streamName;
    }
}

int StreamManager::getStreamsNumber() {
    return streams_.size();
}

int StreamManager::getPacketsNumber() {
    std::unique_lock<std::mutex> lock(globalPacketsMutex_);
    return packetsSent_;
}

std::vector<std::string> StreamManager::getStreamNames() {
    std::vector<std::string> names;
    for (auto& it : streams_) {
        names.push_back(it.first);
    }
    return names;
}

std::vector<std::string> StreamManager::getStreamNamesOfTypeOrd(StreamType type) {
    std::vector<std::string> namesOfType;
    for (auto& it : streams_) {
        if (it.second->type == type)
            namesOfType.push_back(it.first);
    }
    return namesOfType;
}

bool StreamManager::isRegistered(std::string& streamName) {
    return streams_.count(streamName) == 1;
}

void StreamManager::free() {
    for (auto& thr : threads_) {
        thr.detach();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(30));

    for (auto& it : vCapturers_) {
        it.second->stop();
    }
    for (auto& it : lGrabbers_) {
        it.second->stop();
    }

    vCapturers_.clear();
    lGrabbers_.clear();
    vEncoders_.clear();
    lEncoders_.clear();

    for (auto& it : streams_) {
        std::cout << "Final data transmission state for the stream " << it.first << std::endl;
        it.second->state.getTransmissionState().print();
    }
}

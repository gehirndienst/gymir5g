/*
 * Stream.h
 *
 * a class that encapsulates all stream management activities: data, time, quality etc.
 *
 *  Created on: Oct 19, 2022
 *      Author: Nikita Smirnov
 */

#ifndef STREAM_H
#define STREAM_H

#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <boost/circular_buffer.hpp>
#include <nlohmann/json.hpp>
#include <omnetpp.h>

#include "domain/CSVLogger.h"
#include "model/common/PacketType.h"
#include "model/common/QualityOfContent.h"
#include "model/network/RetransmissionBuffer.h"
#include "model/states/DataTransmissionManager.h"
#include "model/states/UnitState.h"
#include "model/presets/PointCloudCompressionPresets.h"
#include "model/presets/VideoQualityPresets.h"

/*
 * Stream enums
 *
 * StreamType: enum for stream types, currently supported VIDEO, LIDAR, BLOB (real byte data), SIM (simulated byte data)
 * StreamStatus: enum for stream statuses depending on the queue and general condition
 */

enum StreamType { VIDEO, LIDAR, BLOB, SIM };

namespace streamtype {

    static const std::unordered_map<StreamType, std::string> streamTypeToStringMap{
        {StreamType::VIDEO, "video"},
        {StreamType::LIDAR, "lidar"},
        {StreamType::BLOB, "blob"},
        {StreamType::SIM, "sim"}
    };

    static const std::unordered_map<std::string, StreamType> stringToStreamTypeMap{
        {"video", StreamType::VIDEO},
        {"lidar", StreamType::LIDAR},
        {"blob", StreamType::BLOB},
        {"sim", StreamType::SIM}
    };

    inline const std::string& toString(StreamType streamType) {
        auto it = streamTypeToStringMap.find(streamType);
        if (it != streamTypeToStringMap.end()) {
            return it->second;
        } else {
            throw std::invalid_argument("StreamType::toString Invalid StreamType value");
        }
    }

    inline StreamType fromString(const std::string& streamTypeStr) {
        std::string lowercaseStr = streamTypeStr;
        std::transform(lowercaseStr.begin(), lowercaseStr.end(), lowercaseStr.begin(), ::tolower);

        auto it = stringToStreamTypeMap.find(lowercaseStr);
        if (it != stringToStreamTypeMap.end()) {
            return it->second;
        } else {
            throw std::invalid_argument("StreamType::toString Invalid string for StreamType");
        }
    }
}

enum StreamStatus { RUNNING, PAUSED, OVERFLOWED, EMPTY, FINISHED, FAIL };

/*
 * Init configuration: initial config for encapsulating stream creation
 *
 * General:
 *  - streamName: name of the stream
 *  - streamType: type of the stream, one of the following: {"video", "lidar", "blob", "sim"}
 *  - source: either a filepath or a web address of the stream source
 *  - bufferLimit: how much grabbed unbroken elements can be stored in a local buffer, default 100
 *  - maxPacketSize: max size of one packet in bytes. By default is 64000 bytes.
 *
 * Quality (validated in the Stream class):
 *  - initQuality: initial quality.
 *  - minQuality: min quality, default 0
 *  - maxQuality: max quality, default 4
 *  - initEncodingSpeed: encoding speed, default 4 (the fastest)
 *  - minEncodingSpeed: min encoding speed, default 0
 *  - maxEncodingSpeed: max encoding speed, default 4 (ultrafast x264)
 *
 * Timer (validated in the Stream class):
 *  - sendInterval: an interval in seconds to send the next stream chunk
 *  - timestampsFile: a path to a file with exact timestamps (assumed each line contains exactly one timestamp)
 *
 * History:
 *  - historySize: how much last useful metrics are saved in existed circular buffers (see Stream class declaration)
 *
 * Simulated streams params:
 *  - dataRate: [ONLY FOR SIM STREAMS] average and/or initial data rate in kbit/s for a stream
 *  - dataRateMin: [ONLY FOR SIM STREAMS] min data rate in kbit/s for a stream
 *  - dataRateMax: [ONLY FOR SIM STREAMS] max data rate in kbit/s for a stream
 *  - dataRateVariance: [ONLY FOR SIM STREAMS] [0, 1] whether dataflow is 100% at cbr (0) or a chunk size is randomly chosen from a range of [dr - dr * drV, dr + dr * drV]
 *  - dataRateFluctuationChance: [ONLY FOR SIM STREAMS] [0, 1] a chance to get value from -var interval above. 0 means CBR
 *  - keyFramePeriod: [ONLY FOR SIM STREAMS] a period in seconds, after each a stream imitates a keyframe production (dataRate * 3 so far), not used if 0
 *  - qualityPresets: [ONLY FOR SIM STREAMS] a vector of discrete values for dataRate in kbit/s, which imitates quality presets for video/lidar
 *
 * Verbose:
 *  - verbose: verbose mode: 0 -- stdout: off, logger: off, 1 -- stdout: off, logger: on, 2 -- stdout: on, logger: on
 */

struct StreamInitConfig {
    std::string streamName;
    StreamType streamType;
    std::string source;
    int bufferLimit;

    int maxPacketSize;

    int initQuality;
    int minQuality;
    int maxQuality;
    int initEncodingSpeed;
    int minEncodingSpeed;
    int maxEncodingSpeed;

    double sendInterval;
    std::string timestampsFile;

    int historySize;

    double dataRate;
    double dataRateMin;
    double dataRateMax;
    double dataRateVariance;
    double dataRateFluctuationChance;
    double keyFramePeriod;
    std::vector<int> qualityPresets;

    int verbose;
};

/*
* StreamData: encapsulates byte stream data with some metadata
*/

struct StreamData {
    std::string streamName;
    StreamType streamType;
    int elemNumber;
    int elemSize;
    std::vector<std::pair<uint8_t*, int>> data;
    std::unique_ptr<QualityOfContent> qoc;
    time_t captureTimestamp;
    time_t popTimestamp;
    time_t encodeTimestamp;
    bool isImportant;

    StreamData() = default;

    StreamData(std::string& name, StreamType type, int num, int size,
               std::vector<std::pair<uint8_t*, int>>& dataVec, std::unique_ptr<QualityOfContent> qoc,
               time_t capture, time_t pop, time_t encode, bool important)
        : streamName(name), streamType(type), elemNumber(num), elemSize(size),
          data(dataVec), qoc(std::move(qoc)), captureTimestamp(capture), popTimestamp(pop),
          encodeTimestamp(encode), isImportant(important) {}

    ~StreamData() {
        // lidar and video streams hold copy-pointers from encoders that are freed after each iteration,
        // the final cleanup is controlled by encoders themselves, so calling delete for their data here will result in segfault
        if (!data.empty() && streamType != VIDEO && streamType != LIDAR) {
            for (auto& pair : data) {
                delete[] pair.first;
            }
        }
        data.clear();
    }

    StreamData(const StreamData& other)
        : streamName(other.streamName), streamType(other.streamType), elemNumber(other.elemNumber),
          elemSize(other.elemSize), data(std::size(other.data)), captureTimestamp(other.captureTimestamp),
          popTimestamp(other.popTimestamp), encodeTimestamp(other.encodeTimestamp),
          isImportant(other.isImportant) {
        std::copy(std::begin(other.data), std::end(other.data), std::begin(data));
        if (other.qoc) {
            qoc = std::make_unique<QualityOfContent>(*other.qoc);
        }
    }

    StreamData(StreamData&& other) noexcept
        : streamName(std::move(other.streamName)), streamType(other.streamType),
          elemNumber(other.elemNumber), elemSize(other.elemSize),
          data(std::move(other.data)), qoc(std::move(other.qoc)),
          captureTimestamp(other.captureTimestamp), popTimestamp(other.popTimestamp),
          encodeTimestamp(other.encodeTimestamp), isImportant(other.isImportant) {}

    StreamData& operator = (const StreamData& other) {
        if (this != &other) {
            streamName = other.streamName;
            streamType = other.streamType;
            elemNumber = other.elemNumber;
            elemSize = other.elemSize;
            data.resize(std::size(other.data));
            std::copy(std::begin(other.data), std::end(other.data), std::begin(data));
            if (other.qoc) {
                qoc = std::make_unique<QualityOfContent>(*other.qoc);
            } else {
                qoc.reset();
            }
            captureTimestamp = other.captureTimestamp;
            popTimestamp = other.popTimestamp;
            encodeTimestamp = other.encodeTimestamp;
            isImportant = other.isImportant;
        }
        return *this;
    }

    StreamData& operator = (StreamData&& other) noexcept {
        if (this != &other) {
            streamName = std::move(other.streamName);
            streamType = other.streamType;
            elemNumber = other.elemNumber;
            elemSize = other.elemSize;
            data = std::move(other.data);
            qoc = std::move(other.qoc);
            captureTimestamp = other.captureTimestamp;
            popTimestamp = other.popTimestamp;
            encodeTimestamp = other.encodeTimestamp;
            isImportant = other.isImportant;
        }
        return *this;
    }

    bool isEmpty() { return data.empty(); }
};

/*
 * StreamQuality class: collects all quality-related information about the stream
 */
class StreamQuality {
public:
    StreamQuality(int initQuality, int minQuality, int maxQuality,
                  int encodingSpeed = 4, int minEncodingSpeed = 0, int maxEncodingSpeed = 4);
    StreamQuality() = default;
    virtual ~StreamQuality() = default;

private:
    std::pair<VideoQuality, VideoEncodingSpeed> getVideoPreset();
    std::vector<std::pair<VideoQuality, VideoEncodingSpeed>> getAllVideoPresets();
    std::pair<PointCloudQuality, bool> getLidarPreset();
    std::vector<std::pair<PointCloudQuality, bool>> getAllLidarPresets();

public:
    int currQuality;
    int minQuality;
    int maxQuality;
    int currEncodingSpeed;
    int minEncodingSpeed;
    int maxEncodingSpeed;

    friend class Stream;
};

/*
 * StreamTimer class: collects all time-related information about the stream
 */
class StreamTimer {
public:
    StreamTimer(double sendInterval, const std::string& timestampsFile = "");
    StreamTimer() = default;
    virtual ~StreamTimer() = default;

private:
    omnetpp::simtime_t getNextSendingTime();
    void pauseStream();
    void resumeStream();

private:
    double sendInterval;
    bool isPeriodic;

    std::string timestampsFile;
    std::vector<double> timestampsVec;
    int timestampsIndex;
    double timestampsOffset;
    bool isFromFile;

    omnetpp::simtime_t startTime;
    double timeInactive;
    double timeInactiveStart;

    friend class Stream;
};

/*
 * StreamDataGenerator class: generates artificial bytes data at given rates
 * ONLY FOR SIM STREAMS
 */
class StreamDataGenerator {
public:
    StreamDataGenerator(double dataRate,
                        double dataRateMin,
                        double dataRateMax,
                        double sendInterval,
                        double dataRateVariance,
                        double dataRateFluctuationChance,
                        double keyFramePeriod,
                        std::vector<int>& qualityPresets);
    StreamDataGenerator() = default;
    virtual ~StreamDataGenerator() = default;

public:
    int generateFrameSize();

    double getAbsoluteRate(double relativeRate);
    void updateRate(double newRate);
    void updateRateWithQualityPreset(int qualityPresetIndex);

    bool checkKeyFrame(int framesSent);
    int getQualityPresetIndexByRate(int rateKbps);

    void resetToDefaultRate();
    double getRate();
    double getMinRate();
    double getMaxRate();

private:
    double dataRate;
    double dataRateInitial;
    double dataRateMin;
    double dataRateMax;
    double dataRateVariance;
    double dataRateFluctuationChance;
    std::vector<int> qualityPresets;

    int keyFrameInterval;
    bool isKeyFrame;

    double frameSize;
    double framesPerSecond;
    std::default_random_engine randomEngine;
    std::uniform_real_distribution<double> dataRateVarianceDist;
    std::uniform_real_distribution<double> dataRateFluctuationDist;

    friend class Stream;
};

/*
 * Stream class
 */
class Stream {
public:
    // how far can we look in the past (e.g., for previous actions)
    inline static const int pastLookout = 10;
public:
    Stream(StreamType type, std::string name, int typeId, int globalId);
    Stream() = default;
    virtual ~Stream();

protected:
    // set main placeholders
    void set(StreamQuality quality, StreamTimer timer, StreamDataGenerator dataGen,
             int maxPacketSize, int historySize, int verbose, const std::string& logdir = "");

    // update data transmission state
    void checkNetworkInstability();
    void updateWithPacket(const inet::Ptr<const inet::Chunk>& packet, PacketType packetType, int twNumber);

    // update buffers
    void updateElemSent(int elemSize, time_t queueingTime, time_t encodingTime);

    // FEC redundancy
    bool isTimeToSendFec(int currentFecAfterPackets, bool isHigh);

    // logging
    void createLogger(const std::string& logdir);
    void log(int runNumber, double timestamp);

    // get time information
    omnetpp::simtime_t getNextSendingTime();
    double getSendInterval();
    double getTimeElapsed();
    double getTimeInactive();

    // get new quality presets
    int getQualityChanges(int lookup);
    bool getIsQualityChanged();
    std::pair<VideoQuality, VideoEncodingSpeed> getVideoPreset();
    std::vector<std::pair<VideoQuality, VideoEncodingSpeed>> getAllVideoPresets();
    std::pair<PointCloudQuality, bool> getLidarPreset();
    std::vector<std::pair<PointCloudQuality, bool>> getAllLidarPresets();
    int getSimPreset();

    // get number of packets generated
    int getPacketsSent();

    // pause/resume/stop stream (a mirror of methods from manager class)
    void runStream();
    void pauseStream();
    void resumeStream();
    void stopStream();

protected:
    std::string name;
    StreamType type;
    int typeId;
    int globalId;

    // data
    StreamData data;

    // splitting to packets
    int maxPacketSize;

    // placeholders for time, quality and sim data generator
    StreamTimer timer;
    StreamQuality quality;
    StreamDataGenerator dataGen;

    // stream info
    UnitState state;
    bool isRunning;

    // buffer for restransmissions
    RetransmissionBuffer retransmissionBuffer;

    // csv logger
    std::string logfile;
    CSVLogger* logger;

    // verbose mode
    int verbose;

    // concurrency
    mutable std::mutex streamMutex;

    // friends
    friend class StreamActions;
    friend class StreamManager;
    // TODO: remake it so that IAdaptiveStreaming and its childs extend Stream class
    friend class AbrBase;
    friend class AbrGcc;
    friend class DrlBase;
};

#endif

/*
 * StreamManager.h
 *
 * a class that holds all streams in one structure including capturing and encoding in separate threads
 *
 * Created on: July 19, 2022
 *      Author: Nikita Smirnov
 */

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <future>
#include <iostream>
#include <map>
#include <mutex>
#include <stdexcept>
#include <utility>
#include <thread>
#include <vector>

#include <nlohmann/json.hpp>
#include <omnetpp.h>

#include "model/media/lidar/PointCloudEncoder.h"
#include "model/media/lidar/VLPLidarGrabber.h"
#include "model/media/video/VideoCapturer.h"
#include "model/media/video/VideoEncoderH264.h"
#include "model/learning/AdaptiveStreamingFactory.h"
#include "model/stream/Stream.h"

#ifndef STREAMMANAGER_H
#define STREAMMANAGER_H

class StreamManager {
public:
    StreamManager();
    StreamManager& operator = (const StreamManager& other) = default;
    ~StreamManager();

    // main methods for a collection of streams
    void setAllStreams(nlohmann::json& streamConfigsJson, const std::string& logdir = "");
    void runAllStreams();

    /* DATA */
    // fill captured/grabbed/generated data from a stream
    void fillStreamData(std::string& streamName, bool isWaiting = false);
    // generate future stream data
    StreamData generateFutureStreamData(std::string& streamName);

    /* adaptive streaming */
    // set as mode
    void setAdaptiveStreaming(AdaptiveAlgorithm algorithm);
    // get states for all registered streams
    std::vector<nlohmann::json> getStates();
    // apply an action to a stream, action is either int or double
    void applyAction(std::string& streamName, int action);
    void applyAction(std::string& streamName, double action);

    // set and update data transmission state
    void updateWithPacket(std::string& streamName, const inet::Ptr<const inet::Chunk>& packet, PacketType packetType,
                          int twNumber = 0);
    void checkNetworkInstability();

    // retransmission buffers (controlled at the application layer)
    void setRetransmissionBuffers(double maxRetransmitAge);

    // increment a counter and checks it is a time to send a new redundannt (FEC) packet, call after each packet sent
    bool isTimeToSendFec(std::string& streamName, int currentFecAfterPackets, bool isHigh = false);
    void setNewFirstFecSequenceNumber(std::string& streamName, int sequenceNumber);

    // update stream info after sending the data
    void updateElemSent(std::string& streamName, int elemSize, time_t queueingTime, time_t encodingTime);
    void updatePacketSent();

    // logger for a stream
    void logStreams(int runNumber, double timestamp);

    // turn stream data acquision on/off
    // TODO: add feature to completely stop the stream not just its data transmission
    void pauseStream(std::string& streamName);
    void resumeStream(std::string& streamName);
    void stopStream(std::string& streamName);

    // getters single
    std::unique_ptr<Stream>& getStream(std::string& streamName);
    int getStreamGlobalId(std::string& streamName);
    StreamType getStreamType(std::string& streamName);
    StreamStatus getStreamStatus(std::string& streamName);
    StreamData& getStreamData(std::string& streamName);
    std::mutex& getStreamMutex(std::string& streamName);
    UnitState& getUnitState(std::string& sstreamName);
    int getStreamMaxPacketSize(std::string& streamName);
    DataTransmissionState& getStreamDataTransmissionState(std::string& streamName);
    RetransmissionBuffer& getStreamRetranmissionBuffer(std::string& streamName);
    int getStreamFirstFecSequenceNumber(std::string& streamName);
    omnetpp::simtime_t getStreamNextSendingTime(std::string& streamName);
    double getStreamSendInterval(std::string& streamName);
    int getStreamElemsAcquired(std::string& streamName);
    int getStreamQueueSize(std::string& streamName);
    int getStreamElemsSent(std::string& streamName);
    int getStreamActionsTaken(std::string& streamName);
    int getStreamPacketsSent(std::string& streamName);
    boost::circular_buffer<double>& getStreamLastQualities(std::string& streamName);
    bool getStreamIsRunning(std::string& streamName);
    int getStreamVerbose(std::string& streamName);
    std::string getStreamNameById(int globalStreamId);

    // getters multiple
    int getStreamsNumber();
    int getPacketsNumber();
    std::vector<std::string> getStreamNames();
    std::vector<std::string> getStreamNamesOfTypeOrd(StreamType type);

private:
    std::vector<StreamInitConfig> getStreamConfigs(nlohmann::json& streamConfigsJson);
    void registerAllStreams(std::vector<std::tuple<std::string, StreamType>>& streamDeclarations);
    void setVideoStream(std::string& streamName, std::string& source, int bufferLimit, int maxPacketSize, int verbose);
    void setLidarStream(std::string& streamName, std::string& source, int bufferLimit, int maxPacketSize, int verbose);
    // TODO: write a capturing class for blob streams
    void setBlobStream(std::string& streamName, std::string& source, int bufferLimit);
    void setSimStream(std::string& streamName, int maxPacketSize, int verbose);
    void runVideoStream(std::string& streamName);
    void runLidarStream(std::string& streamName);

    bool isRegistered(std::string& streamName);
    void free();

private:
    std::unordered_map<std::string, std::unique_ptr<Stream>> streams_;
    std::unordered_map<std::string, std::unique_ptr<IAdaptiveStreaming>> streamActors_;

    std::vector<std::thread> threads_;

    std::unordered_map<std::string, std::shared_ptr<VideoCapturer>> vCapturers_;
    std::unordered_map<std::string, std::shared_ptr<VLPLidarGrabber>> lGrabbers_;
    std::unordered_map<std::string, std::unique_ptr<VideoEncoderH264>> vEncoders_;
    std::unordered_map<std::string, std::unique_ptr<PointCloudEncoder>> lEncoders_;

    int packetsSent_;
    mutable std::mutex globalPacketsMutex_;
};

#endif
/*
 * VLPLidarGrabber.h
 *
 * grabbes VLP-16 input from pcap file or address and save in a thread-safe queue
 * supposed to be run in a single thread
 * overflow and pause is modelled
 *
 * Created on: May 25, 2022
 *      Author: Nikita Smirnov
 */

#ifndef VLPLIDARGRABBER_H
#define VLPLIDARGRABBER_H

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>

#include <boost/lexical_cast.hpp>
#include <pcl/common/io.h>
#include <pcl/io/vlp_grabber.h>

#include "domain/TSQueue.h"
#include "model/stream/Stream.h"

using Point = pcl::PointXYZI;
using Cloud = pcl::PointCloud<pcl::PointXYZI>;
using CloudPtr = typename Cloud::Ptr;
using CloudConstPtr = typename Cloud::ConstPtr;

struct PointCloudMetadata {
    long pointsHas;
    int fps;
    time_t grabTime;
    int cloudNum;

    void print() {
        std::cout << "PointCloudMetadata: " <<
                  "\npointsHas:   " << pointsHas  <<
                  "\nfpsInstant:  " << fps <<
                  "\ntimeGrabbed: " << grabTime <<
                  "\ncouldNumber: " << cloudNum << std::endl;
    }
};

struct PointCloudData {
    Cloud cloud;
    PointCloudMetadata metadata;

    bool isEmpty() { return cloud.empty(); }
};

class VLPLidarGrabber {
    static const int popMaxWaitingTimeSec = 3;
    static constexpr double maxSecondsInactive = 2.0;
public:
    VLPLidarGrabber(const std::string& pcap, int bufferSizeLimit = 0, int verbose = 0);
    VLPLidarGrabber(const std::string& ip, const std::string& port, int bufferSizeLimit = 0, int verbose = 0);
    VLPLidarGrabber(const VLPLidarGrabber& other);
    ~VLPLidarGrabber();

    void run();
    void pause();
    void resume();
    void stop();
    std::optional<PointCloudData> pop(bool isWaiting = false);

    int getCloudsGrabbed();
    int getQueueSize();
    StreamStatus getStatus();

private:
    void handlePointCloud(const CloudConstPtr& cloud);
    void awakeAfterSleep();
    void awakeAfterOverflow();

public:
    bool isQueueOverflowed;
    bool isRunning;
    bool isOk;
    int verbose;

private:
    std::unique_ptr<pcl::VLPGrabber> grabber_;
    TSQueue<PointCloudData> pcQueue_;
    CloudPtr cloudContainer_;
    int cloudsGrabbed_;
    double secondsInactive_;
    // for multithreading
    mutable std::mutex mutex_;
    std::condition_variable cvar_;
};

#endif //VLPLIDARGRABBER_H
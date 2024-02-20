/*
 * VideoCapturer.h
 *
 * captures video input and save in a thread-safe queue
 * supposed to be run in a single thread
 * overflow and pause is modelled
 *
 * Created on: Apr 24, 2022
 *      Author: Nikita Smirnov
 */

#ifndef VIDEOCAPTURER_H
#define VIDEOCAPTURER_H

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>

#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "domain/TSQueue.h"
#include "model/stream/Stream.h"

struct VideoFrameMetadata {
    time_t captureTime;
    int frameNum;
    int frameWidth;
    int frameHeight;
    int fps;

    void print() {
        std::cout << "VideoFrameMetadata: " <<
                  "\nwidth:     " << frameWidth  <<
                  "\nheight:    " << frameHeight <<
                  "\nfps:       " << fps         <<
                  "\ntime:      " << captureTime <<
                  "\nnum:       " << frameNum    << std::endl;
    }
};

struct VideoData {
    cv::Mat frame;
    VideoFrameMetadata metadata;

    bool isEmpty() { return frame.empty(); }
};

class VideoCapturer {
    static const int popMaxWaitingTimeSec = 3;
public:
    VideoCapturer(const std::string& source, int bufferSizeLimit, int verbose = 0);
    VideoCapturer(const std::string& source, int verbose = 0);
    VideoCapturer(const VideoCapturer& other);
    ~VideoCapturer();

    std::optional<VideoData> pop(bool isWaiting = false);
    void run();
    void pause();
    void resume();
    void stop();

    int getFramesCaptured();
    int getQueueSize();
    StreamStatus getStatus();

private:
    void capture();
    void awakeAfterSleep();
    void awakeAfterOverflow();

private:
    std::unique_ptr<cv::VideoCapture> cap_;
    TSQueue<VideoData> captureQueue_;
    int framesCaptured_;
    // for multithreading
    mutable std::mutex mutex_;
    std::condition_variable cvar_;
public:
    bool isQueueOverflowed;
    bool isRunning;
    bool isOk;
    int verbose;
    std::pair<int, int> srcSize;
};

#endif
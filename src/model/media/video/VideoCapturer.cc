#include "VideoCapturer.h"

VideoCapturer::VideoCapturer(const std::string& source, int queueLimitSize, int verb) {
    cap_ = std::unique_ptr<cv::VideoCapture>(new cv::VideoCapture (source, cv::CAP_FFMPEG));
    if (!cap_->isOpened()) {
        throw std::runtime_error("[VideoCapturer::VideoCapturer] Capture is not opened, check source, current is: " + source);
    } else {
        std::cout << "[VideoCapturer::VideoCapturer] Capture is succesfully opened, source: " << source << std::endl;
    }
    captureQueue_.setLimitSize(queueLimitSize);
    srcSize = std::make_pair(
                  (int)cap_->get(cv::CAP_PROP_FRAME_WIDTH), (int)cap_->get(cv::CAP_PROP_FRAME_HEIGHT));
    framesCaptured_ = 0;
    isRunning = false;
    isQueueOverflowed = false;
    isOk = true;
    verbose = verb;
}

VideoCapturer::VideoCapturer(const std::string& source, int verb) {
    cap_ = std::unique_ptr<cv::VideoCapture>(new cv::VideoCapture (source, cv::CAP_FFMPEG));
    if (!cap_->isOpened()) {
        throw std::runtime_error("[VideoCapturer::VideoCapturer] Capture is not opened, check source, current is: " + source);
    } else {
        std::cout << "[VideoCapturer::VideoCapturer] Capture is succesfully opened, source: " << source << std::endl;
    }
    srcSize = std::make_pair(
                  (int)cap_->get(cv::CAP_PROP_FRAME_WIDTH), (int)cap_->get(cv::CAP_PROP_FRAME_HEIGHT));
    framesCaptured_ = 0;
    isRunning = false;
    isQueueOverflowed = false;
    isOk = true;
    verbose = verb;
}

VideoCapturer::VideoCapturer(const VideoCapturer& other) {}

VideoCapturer::~VideoCapturer() {
    if (isOk) {
        stop();
    }
}

void VideoCapturer::run() {
    if (!isOk) {
        std::cerr << "[VideoCapturer::run] video capturer is set up wrong" << std::endl;
        return;
    }
    isRunning = true;
    capture();
    isRunning = false;
}

void VideoCapturer::capture() {
    cv::Mat frame;
    time_t captureTime;
    int frameWidth;
    int frameHeight;
    int fps;
    bool isSetLimit = captureQueue_.getLimitSize() > 0;
    //cv::namedWindow("input video", cv::WINDOW_NORMAL);

    while (isOk) {
        *cap_.get() >> frame;
        frameWidth = (int)cap_->get(cv::CAP_PROP_FRAME_WIDTH);
        frameHeight = (int)cap_->get(cv::CAP_PROP_FRAME_HEIGHT);
        fps = (int)cap_->get(cv::CAP_PROP_FPS);
        //cv::imshow("input video", frame);
        if (frame.empty() && framesCaptured_ > 0) {
            std::cerr << "[VideoCapturer::capture] a blank frame is captured, finished capturing" << std::endl;
            break;
        }
        framesCaptured_ ++;
        captureTime = std::chrono::duration_cast<std::chrono::milliseconds>
                      (std::chrono::system_clock::now().time_since_epoch()).count();
        VideoFrameMetadata metadata{captureTime, framesCaptured_, frameWidth, frameHeight, fps};
        VideoData data{frame, metadata};
        captureQueue_.push(data);
        if (isSetLimit) {
            if (captureQueue_.isOverflowed()) {
                if (verbose > 1) {
                    std::cout << "[VideoCapturer::capture] frame queue has been oveflowed, "
                              << "postpone capturing, set a bigger limit size or speed up the processing" << std::endl;
                }
                isQueueOverflowed = true;
                std::unique_lock<std::mutex> lock(mutex_);
                while (isQueueOverflowed) {
                    cvar_.wait(lock);
                }
                if (!isOk) {
                    break;
                }
                if (verbose > 1) {
                    std::cout << "[VideoCapturer::capture] frame queue is no more overflowed, "
                              << "resume capturing" << std::endl;
                }
            }
        }
        if (!isRunning) {
            if (verbose > 1) {
                std::cout << "[VideoCapturer::capture] capturing is paused, waiting for resuming" << std::endl;
            }
            std::unique_lock<std::mutex> lock(mutex_);
            while (!isRunning) {
                cvar_.wait(lock);
            }
            if (!isOk) {
                break;
            }
            if (verbose > 1) {
                std::cout << "[VideoCapturer::capture] capturing is resumed" << std::endl;
            }
        }
    }
}

std::optional<VideoData> VideoCapturer::pop(bool isWaiting) {
    VideoData data;
    bool result = captureQueue_.tryPop(data);
    if (result) {
        awakeAfterOverflow();
        return data;
    } else {
        if (isWaiting) {
            captureQueue_.waitPop(data, popMaxWaitingTimeSec * 1000);
            if (data.isEmpty()) {
                if (verbose > 1) {
                    std::cout << "[VideoCapturer::pop] wait for default timeout: "
                              << popMaxWaitingTimeSec << " sec., but still empty" << std::endl;
                }
                return {};
            } else {
                awakeAfterOverflow();
                return data;
            }
        } else {
            return {};
        }
    }
}

StreamStatus VideoCapturer::getStatus() {
    int queueSize = getQueueSize();
    if (isQueueOverflowed) {
        return OVERFLOWED;
    } else if (isRunning && queueSize > 0) {
        return RUNNING;
    } else if (!isRunning && queueSize > 0) {
        return PAUSED;
    } else if (isRunning && queueSize == 0 && isOk) {
        return EMPTY;
    } else if (!isRunning && queueSize == 0 && isOk) {
        return FINISHED;
    } else {
        return FAIL;
    }
}

int VideoCapturer::getFramesCaptured() {
    return framesCaptured_;
}

int VideoCapturer::getQueueSize() {
    return captureQueue_.size();
}

void VideoCapturer::awakeAfterOverflow() {
    mutex_.lock();
    if (isQueueOverflowed && !captureQueue_.isOverflowed()) {
        // queue is no more overflowed, notify capture method
        isQueueOverflowed = false;
        cvar_.notify_all();
    }
    mutex_.unlock();
}

void VideoCapturer::awakeAfterSleep() {
    mutex_.lock();
    if (!isRunning) {
        // extern resuming the capturing, notify capture method
        isRunning = true;
        cvar_.notify_all();
    }
    mutex_.unlock();
}

void VideoCapturer::pause() {
    isRunning = false;
}

void VideoCapturer::resume() {
    awakeAfterSleep();
}

void VideoCapturer::stop() {
    mutex_.lock();
    isOk = false;
    mutex_.unlock();
    awakeAfterOverflow();
    awakeAfterSleep();
    captureQueue_.clear();
    cap_->release();
    std::cout << "[VideoCapturer::stop]: succesfully stopped capturing" << std::endl;
}

#include "VLPLidarGrabber.h"

VLPLidarGrabber::VLPLidarGrabber(const std::string& pcap, int queueLimitSize, int verb) {
    grabber_ = std::unique_ptr<pcl::VLPGrabber>(new pcl::VLPGrabber(pcap));
    if (!grabber_) {
        throw std::runtime_error(
            "[VLPLidarGrabber::VLPLidarGrabber] Grabbing is not started, check pcap file path, current is: "
            + pcap);
    } else {
        std::cout << "[VLPLidarGrabber::VLPLidarGrabber] Grabbing is succesfully started, pcap file: "
                  << pcap << std::endl;
    }

    if (queueLimitSize > 0) {
        pcQueue_.setLimitSize(queueLimitSize);
    }

    cloudContainer_ = CloudPtr(new Cloud);

    cloudsGrabbed_ = 0;
    secondsInactive_ = 0.0;

    isRunning = false;
    isQueueOverflowed = false;
    isOk = true;
    verbose = verb;
}

VLPLidarGrabber::VLPLidarGrabber(const std::string& ip, const std::string& port, int queueLimitSize, int verb) {
    grabber_ = std::unique_ptr<pcl::VLPGrabber>(
                   new pcl::VLPGrabber(boost::asio::ip::address::from_string(ip),
                                       boost::lexical_cast<unsigned short>(port)));
    if (!grabber_) {
        throw std::runtime_error("[VLPLidarGrabber::VLPLidarGrabber] Grabbing is not started, check address current is: "
                                 + ip + ": " + port);
    } else {
        std::cout << "[VLPLidarGrabber::VLPLidarGrabber] Grabbing is succesfully started, address: "
                  << ip << ": " << port << std::endl;
    }

    if (queueLimitSize > 0) {
        pcQueue_.setLimitSize(queueLimitSize);
    }

    cloudContainer_ = CloudPtr(new Cloud);

    cloudsGrabbed_ = 0;
    secondsInactive_ = 0.0;

    isRunning = false;
    isQueueOverflowed = false;
    isOk = true;
    verbose = verb;
}

VLPLidarGrabber::VLPLidarGrabber(const VLPLidarGrabber& other) {}

VLPLidarGrabber::~VLPLidarGrabber() {
    if (isOk) {
        stop();
    }
}

void VLPLidarGrabber::run() {
    if (!isOk) {
        std::cerr << "[VLPLidarGrabber::run] VLP grabber is set up wrong" << std::endl;
        return;
    }

    // register the callback function and start the grabber
    std::function<void(const CloudConstPtr&)> callback =
    [this](const CloudConstPtr & cloudPtr) { handlePointCloud(cloudPtr); };
    grabber_->registerCallback(callback);
    grabber_->start();
    if (!grabber_->isRunning()) {
        std::cerr << "[VLPLidarGrabber::run] VLP grabber is not running after the start, shutdown..." << std::endl;
        return;
    }
    isRunning = true;

    // main loop
    while (secondsInactive_ < maxSecondsInactive && isOk) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        // unfortunately there is no smarter mechanism to stop it automatically after there are no more clouds to grab
        if (!isQueueOverflowed && isRunning)
            secondsInactive_ += 0.5;
    };
}

void VLPLidarGrabber::handlePointCloud(const CloudConstPtr& inputCloud) {
    pcl::copyPointCloud(*inputCloud, *cloudContainer_);
    Cloud cloud = *cloudContainer_;

    auto grabTime = std::chrono::duration_cast<std::chrono::milliseconds>
                    (std::chrono::system_clock::now().time_since_epoch()).count();
    auto pointsHas = cloud.size();
    int fpsAtTheMoment = (int)grabber_->getFramesPerSecond();
    PointCloudMetadata metadata{(long)pointsHas, fpsAtTheMoment, grabTime, cloudsGrabbed_};
    PointCloudData data{cloud, metadata};
    pcQueue_.push(data);
    cloudsGrabbed_ ++;

    if (pcQueue_.getLimitSize() > 0) {
        if (pcQueue_.isOverflowed()) {
            if (verbose > 1) {
                std::cout << "[VLPLidarGrabber::handlePointCloud] frame queue has been oveflowed, "
                          << "postpone grabbing, set a bigger limit size or speed up the processing"
                          << std::endl;
            }
            isQueueOverflowed = true;
            std::unique_lock<std::mutex> lock(mutex_);
            while (isQueueOverflowed) {
                cvar_.wait(lock);
            }
            if (!isOk) return;
            if (verbose > 1) {
                std::cout << "[VLPLidarGrabber::handlePointCloud] frame queue is no more overflowed, "
                          << "resume grabbing" << std::endl;
            }
        }
    }
    if (!isRunning) {
        if (verbose > 1) {
            std::cout << "[VLPLidarGrabber::handlePointCloud] grabbing is paused, waiting for resuming"
                      << std::endl;
        }
        std::unique_lock<std::mutex> lock(mutex_);
        while (!isRunning) {
            cvar_.wait(lock);
        }
        if (!isOk) return;
        if (verbose > 1) std::cout << "[VLPLidarGrabber::handlePointCloud] grabbing is resumed" << std::endl;
    }
    secondsInactive_ = 0.0;
}

std::optional<PointCloudData> VLPLidarGrabber::pop(bool isWaiting) {
    PointCloudData data;
    bool result = pcQueue_.tryPop(data);
    if (result) {
        awakeAfterOverflow();
        return data;
    } else {
        if (isWaiting) {
            pcQueue_.waitPop(data, popMaxWaitingTimeSec * 1000);
            if (data.isEmpty()) {
                if (verbose > 1) {
                    std::cout << "[VLPLidarGrabber::pop] wait for default timeout: "
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

StreamStatus VLPLidarGrabber::getStatus() {
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


int VLPLidarGrabber::getCloudsGrabbed() {
    return cloudsGrabbed_;
}

int VLPLidarGrabber::getQueueSize() {
    return pcQueue_.size();
}

void VLPLidarGrabber::awakeAfterOverflow() {
    mutex_.lock();
    if ((isQueueOverflowed && !pcQueue_.isOverflowed()) || !isOk) {
        // queue is no more overflowed, notify grab method
        isQueueOverflowed = false;
        cvar_.notify_all();
    }
    mutex_.unlock();
}

void VLPLidarGrabber::awakeAfterSleep() {
    mutex_.lock();
    if (!isRunning) {
        // extern resuming the capturing, notify grab method
        isRunning = true;
        cvar_.notify_all();
    }
    mutex_.unlock();
}

void VLPLidarGrabber::pause() {
    isRunning = false;
}

void VLPLidarGrabber::resume() {
    awakeAfterSleep();
}

void VLPLidarGrabber::stop() {
    mutex_.lock();
    isOk = false;
    mutex_.unlock();
    awakeAfterOverflow();
    awakeAfterSleep();
    pcQueue_.clear();
    grabber_->stop();
    std::cout << "[VLPLidarGrabber::stop]: succesfully stopped grabbing" << std::endl;
}
// GymCommunicator.cc

#include "GymCommunicator.h"

void GymCommunicator::init(std::string address, int noReplyTimeoutMillis) {
    // zmq init
    context_ = zmq::context_t(1);
    socket_ = zmq::socket_t(context_, zmq::socket_type::req);
    socket_.setsockopt(ZMQ_RCVTIMEO, noReplyTimeoutMillis);

    // init connection to gym
    socket_.connect("tcp://" + address);
    // check if connected
    zmq::pollitem_t items[] = { {socket_, 0, ZMQ_POLLOUT, 0} };
    int ret = zmq::poll(items, 1, noReplyTimeoutMillis);
    if (ret == -1 || (ret == 0 || !(items[0].revents & ZMQ_POLLOUT))) {
        throw std::runtime_error("GymCommunicator::init failed to connect to OpenAI Gym server at tcp://" + address);
    } else {
        std::cout << "GymCommunicator::init connected to OpenAI Gym server at tcp://" << address << std::endl;
    }

    sharedRequest_ = std::shared_ptr<nlohmann::json>(new nlohmann::json);
    sharedReply_ = std::shared_ptr<nlohmann::json>(new nlohmann::json);
    isNewReply_ = false;
    isNewRequest_ = false;
    isRunning_ = true;
}

void GymCommunicator::run() {
    try {
        while (isRunning_) {
            std::unique_lock<std::mutex> lock(mutex_);
            cvar_.wait(lock, [this]() { return isNewRequest_ || !isRunning_; });
            if (!isRunning_) break;
            isNewRequest_ = false;
            requestNum_++;
            exchange();
            if (sharedReply_->empty()) {
                throw std::runtime_error("GymCommunicator::run gym communicator receives empty reply");
            }
            isNewReply_ = true;
            replyNum_++;
        }
        std::cout << "GymCommunicator::run main loop has been externally finished..." << std::endl;
    } catch (const zmq::error_t& e) {
        if (!e.num() == ETERM) {
            // if ETERM, it is ok: the context is always terminated at the replier side
            throw std::runtime_error("GymCommunicator::run ZMQ error occurred with error code: " + std::to_string(e.num()) + " with message: " + e.what());
        }
    }
}

void GymCommunicator::exchange() {
    std::string request = sharedRequest_->dump();
    socket_.send(zmq::message_t(request.data(), request.size()), zmq::send_flags::none);
    
    zmq::message_t replyMsg;
    auto ret = socket_.recv(replyMsg);
    if (!ret.has_value()) {
        throw std::runtime_error("GymCommunicator::exchange a timeout for no reply, probably no replier on the python side");
    }
    std::string reply(static_cast<const char*>(replyMsg.data()), replyMsg.size());
    sharedReply_ = std::make_shared<nlohmann::json>(nlohmann::json::parse(reply));
}

void GymCommunicator::pushRequest(nlohmann::json&& requestBody) {
    std::unique_lock<std::mutex> lock(mutex_);
    sharedRequest_ = std::make_shared<nlohmann::json>(std::move(requestBody));
    isNewRequest_ = true;
    cvar_.notify_all();
}

std::optional<nlohmann::json> GymCommunicator::tryPullReply() {
    std::unique_lock<std::mutex> lock(mutex_);
    if (isNewReply_) {
        isNewReply_ = false;
        return *sharedReply_;
    } else {
        return {};
    }
}

int GymCommunicator::getRequestNum() {
    std::unique_lock<std::mutex> lock(mutex_);
    return requestNum_;
}

int GymCommunicator::getReplyNum() {
    std::unique_lock<std::mutex> lock(mutex_);
    return replyNum_;
}

void GymCommunicator::finish() {
    isRunning_ = false;
    cvar_.notify_all();
    socket_.close();
    std::cout << "GymCommunicator::finish zmq-socket for the communicator is correctly closed" << std::endl;
}

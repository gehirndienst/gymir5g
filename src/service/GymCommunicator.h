/*
 * GymCommunicator.h
 *
 * Communication app for exchanging the data between OMNeT++ and OpenAI Gym enviroenment
 * It works as a request-reply socket and should be run in a separate thread to guarantee workability
 *
 * Created on: Aug 1, 2022
 *      Author: Nikita Smirnov
 */

#ifndef GYMCOMMUNICATOR_H
#define GYMCOMMUNICATOR_H


#include <iostream>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <thread>

#include <nlohmann/json.hpp>
#include <omnetpp.h>
#include <zmq.hpp>

class GymCommunicator {
public:
    GymCommunicator() = default;
    virtual ~GymCommunicator() = default;

    void init(std::string address, int noReplyTimeoutMillis = 10000);
    void run();
    void finish();

    void pushRequest(nlohmann::json&& requestBody);
    std::optional<nlohmann::json> tryPullReply();

    int getRequestNum();
    int getReplyNum();

private:
    void exchange();

private:
    zmq::context_t context_;
    zmq::socket_t socket_;

    std::shared_ptr<nlohmann::json> sharedRequest_;
    std::shared_ptr<nlohmann::json> sharedReply_;
    bool isNewRequest_;
    bool isNewReply_;
    bool isRunning_;

    int requestNum_;
    int replyNum_;

    mutable std::mutex mutex_;
    std::condition_variable cvar_;
};

#endif /* GYMCOMMUNICATOR_H */

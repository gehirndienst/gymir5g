/*
 * StreamState.h
 *
 * stream state: data encoding and processing
 *
 *  Created on: Aug 22, 2023
 *      Author: Nikita Smirnov
 */

#ifndef STREAMSTATE_H
#define STREAMSTATE_H

#include <iostream>
#include <vector>

#include <boost/circular_buffer.hpp>
#include <nlohmann/json.hpp>

class StreamState {
public:
    StreamState();
    ~StreamState();

    void set(const std::string& streamName, int historySize = 100);

    // get is quality changed in compare to the previous value
    bool getIsQualityChanged();
    // get amount of quality changes during the lookup period
    int getQualityChanges(int lookup);
    // is time to send a new Forward Error Correction redundant packet? Called through a manager
    bool isTimeToSendFec(int currentFecAfterPackets, bool isHigh);

    // updaters
    void updateElemSent(int elemSize, time_t queueingTime, time_t encodingTime);

    // serializer
    nlohmann::json toJson();

    void clearBuffers();

public:
    std::string streamName; // may be needed
    int elemsSent;

    double lastAction;
    int actionsTaken;

    bool isQualityChanged;

    // redundancy (FEC)
    int fecCounter;
    int firstFecSequenceNumber;

    int historySize;
    std::vector<int> lastElems;
    std::vector<time_t> lastEncodingTimes;
    std::vector<time_t> lastQueueingTimes;
    boost::circular_buffer<double> lastQualities;
};

#endif
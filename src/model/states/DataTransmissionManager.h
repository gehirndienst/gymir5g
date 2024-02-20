/*
 * DataTransmissionManager.h
 *
 * a class that manages data transmisson statistics, allowing periodic measurements
 *
 * Created on: June 27, 2023
 *      Author: Nikita Smirnov
 */

#ifndef DATATRANSMISSIONMANAGER_H
#define DATATRANSMISSIONMANAGER_H

#include <algorithm>
#include <memory>
#include <stdexcept>

#include <omnetpp.h>

#include "DataTransmissionState.h"

class DataTransmissionManager {
public:
    DataTransmissionManager();
    ~DataTransmissionManager() = default;

    DataTransmissionManager(const DataTransmissionManager& other);
    DataTransmissionManager& operator=(const DataTransmissionManager& other);

    // set period for periodic measurements
    void setPeriod(double currentTime);
    // set prev state to the current one and reset the period
    void update();

    // class getter
    DataTransmissionState& getState();
    double getLastTime() const;
    double getPeriodDuration() const;
    bool isEmpty() const;

    // DataTransmissionState glob and per getters
    int getTxPackets () const;
    int getTxPacketsPer() const;

    int getRxPackets() const;
    int getRxPacketsPer() const;

    int getLostPackets() const;
    int getLostPacketsPer() const;
    double getPacketLossRate() const;
    double getPacketLossRatePer() const;

    int getOutOfOrderPackets() const;
    int getOutOfOrderPacketsPer() const;
    double getOutOfOrderRate() const;
    double getOutOfOrderRatePer() const;

    double getTxBytes() const;
    double getTxBytesPer() const;
    double getRxBytes() const;
    double getRxBytesPer() const;
    double getTxMbits() const;
    double getTxMbitsPer() const;
    double getRxMbits() const;
    double getRxMbitsPer() const;

    double getTxGoodputAv() const;
    double getTxGoodputAvPer() const;
    double getRxGoodputAv() const;
    double getRxGoodputAvPer() const;

    int getAckPackets() const;
    int getAckPacketsPer() const;
    double getDelayAv() const;
    double getDelayAvPer() const;
    double getDelayJitterAv() const;
    double getDelayJitterAvPer() const;
    double getRttAv() const;
    double getRttAvPer() const;
    double getRttJitterAv() const;
    double getRttJitterAvPer() const;

    double getInstabilityRate() const;
    double getInstabilityRatePer() const;

    double getTxDuration() const;
    double getRxDuration() const;

private:
    DataTransmissionState state_;
    DataTransmissionState prev_;
    double periodDuration_;
    double lastTime_;
};

#endif

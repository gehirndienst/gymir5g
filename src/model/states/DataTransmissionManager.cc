# include "DataTransmissionManager.h"

DataTransmissionManager::DataTransmissionManager()
    : state_(DataTransmissionState())
    , prev_(DataTransmissionState())
    , periodDuration_(0.0)
    , lastTime_(0.0)
{}

// copy constructor
DataTransmissionManager::DataTransmissionManager(const DataTransmissionManager& other)
    : state_(other.state_)
    , prev_(other.prev_)
    , periodDuration_(other.periodDuration_)
    , lastTime_(other.lastTime_)
{}

// assignment operator
DataTransmissionManager& DataTransmissionManager::operator=(const DataTransmissionManager& other) {
    if (this != &other) {
        state_ = other.state_;
        prev_ = other.prev_;
        periodDuration_ = other.periodDuration_;
        lastTime_ = other.lastTime_;
    }
    return *this;
}

// class methods
void DataTransmissionManager::setPeriod(double currentTime) {
    periodDuration_ = currentTime - lastTime_;
    if (periodDuration_ <= 0.0) {
        throw std::runtime_error("[DataTransmissionManager::setPeriod] time difference <=0! td: " +
                                 std::to_string(periodDuration_));
    }
}

void DataTransmissionManager::update() {
    lastTime_ += periodDuration_;
    periodDuration_ = 0;
    prev_ = state_;
}

DataTransmissionState& DataTransmissionManager::getState() { return state_; }
double DataTransmissionManager::getLastTime() const { return lastTime_; }
double DataTransmissionManager::getPeriodDuration() const { return periodDuration_; }
bool DataTransmissionManager::isEmpty() const {
    return state_.timeFirstPacketAck == 0.0
           && state_.timeFirstPacketArrival == 0.0
           && state_.timeFirstPacketSent == 0.0;
}

// dts getters
int DataTransmissionManager::getTxPackets() const { return state_.numTxPackets; }
int DataTransmissionManager::getTxPacketsPer() const {
    return state_.numTxPackets - prev_.numTxPackets;
}
int DataTransmissionManager::getRxPackets() const { return state_.numRxPackets; }
int DataTransmissionManager::getRxPacketsPer() const {
    return state_.numRxPackets - prev_.numRxPackets;
}
int DataTransmissionManager::getLostPackets() const { return state_.numLostPackets; }
int DataTransmissionManager::getLostPacketsPer() const {
    return state_.numLostPackets - prev_.numLostPackets;
}
double DataTransmissionManager::getPacketLossRate() const { return state_.lossRate; }
double DataTransmissionManager::getPacketLossRatePer() const {
    int packetsReceivedPer = getRxPacketsPer();
    int packetsLostPer = getLostPacketsPer();
    return packetsReceivedPer + packetsLostPer > 0
           ? (double)packetsLostPer / (double)(packetsReceivedPer + packetsLostPer)
           : 0.0;
}
int DataTransmissionManager::getOutOfOrderPackets() const { return state_.numOutOfOrderPackets; }
int DataTransmissionManager::getOutOfOrderPacketsPer() const {
    return state_.numOutOfOrderPackets - prev_.numOutOfOrderPackets;
}
double DataTransmissionManager::getOutOfOrderRate() const { return state_.outOfOrderRate; }
double DataTransmissionManager::getOutOfOrderRatePer() const {
    int packetsOOOPer = getOutOfOrderPacketsPer();
    int packetsReceivedPer = getLostPacketsPer();
    return packetsReceivedPer > 0
           ? (double)packetsOOOPer / (double)packetsReceivedPer
           : 0.0;
}
double DataTransmissionManager::getTxBytes() const { return state_.numTxBytes; }
double DataTransmissionManager::getTxBytesPer() const {
    return state_.numTxBytes - prev_.numTxBytes;
}
double DataTransmissionManager::getRxBytes() const { return state_.numRxBytes; }
double DataTransmissionManager::getRxBytesPer() const {
    return state_.numRxBytes - prev_.numRxBytes;
}
double DataTransmissionManager::getTxMbits() const { return state_.numTxBytes * 0.000008; }
double DataTransmissionManager::getTxMbitsPer() const {
    return (state_.numTxBytes - prev_.numTxBytes) * 0.000008;
}
double DataTransmissionManager::getRxMbits() const { return state_.numRxBytes * 0.000008; }
double DataTransmissionManager::getRxMbitsPer() const {
    return (state_.numRxBytes - prev_.numRxBytes) * 0.000008;
}
double DataTransmissionManager::getTxGoodputAv() const { return state_.avTxGoodput; }
double DataTransmissionManager::getTxGoodputAvPer() const {
    return periodDuration_ > 0 ? getTxMbitsPer() / periodDuration_ : 0.0;
}
double DataTransmissionManager::getRxGoodputAv() const { return state_.avRxGoodput; }
double DataTransmissionManager::getRxGoodputAvPer() const {
    return periodDuration_ > 0 ? getRxMbitsPer() / periodDuration_ : 0.0;
}
int DataTransmissionManager::getAckPackets() const { return state_.numAckPackets; }
int DataTransmissionManager::getAckPacketsPer() const {
    return state_.numAckPackets - prev_.numAckPackets;
}
double DataTransmissionManager::getDelayAv() const { return state_.avDelay; }
double DataTransmissionManager::getDelayAvPer() const {
    int packetsReceivedPer = getRxPacketsPer();
    return packetsReceivedPer > 0
           ? (state_.delaySum - prev_.delaySum) / (double)packetsReceivedPer
           : 0.0;
}
double DataTransmissionManager::getDelayJitterAv() const { return state_.avDelayJitter; }
double DataTransmissionManager::getDelayJitterAvPer() const {
    int packetsReceivedPer = getRxPacketsPer();
    return packetsReceivedPer > 1
           ? (state_.delayJitterSum - prev_.delayJitterSum) / ((double)packetsReceivedPer - 1)
           : 0.0;
}
double DataTransmissionManager::getRttAv() const { return state_.avRtt; }
double DataTransmissionManager::getRttAvPer() const {
    int packetsAcknowledgedPer = getAckPacketsPer();
    return packetsAcknowledgedPer > 0
           ? (state_.rttSum - prev_.rttSum) / (double)packetsAcknowledgedPer
           : 0.0;
}
double DataTransmissionManager::getRttJitterAv() const { return state_.avRttJitter; }
double DataTransmissionManager::getRttJitterAvPer() const {
    int packetsAcknowledgedPer = getAckPacketsPer();
    return packetsAcknowledgedPer > 1
           ? (state_.rttJitterSum - prev_.rttJitterSum) / ((double)packetsAcknowledgedPer - 1)
           : 0.0;
}
double DataTransmissionManager::getInstabilityRate() const { return state_.instabilityRate; }
double DataTransmissionManager::getInstabilityRatePer() const {
    return periodDuration_ > 0
           ? (state_.instabilityDuration - state_.instabilityDuration) / periodDuration_
           : 0.0;
}
double DataTransmissionManager::getTxDuration() const { return state_.txDuration; }
double DataTransmissionManager::getRxDuration() const { return state_.rxDuration; }
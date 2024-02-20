#include "SignalListener.h"

SignalListener::SignalListener() {}

SignalListener::~SignalListener() {}

void SignalListener::receiveSignal(cComponent* src, simsignal_t id, cObject* object, cObject* details) {
    if (srcName_.empty()) {
        auto cMod = dynamic_cast<cModule*>(src);
        srcName_ = cMod->getFullPath();
    }
    objValue_ = object;
    counter_++;
}

void SignalListener::receiveSignal(cComponent* src, simsignal_t id, double d, cObject* details) {
    if (srcName_.empty()) {
        auto cMod = dynamic_cast<cModule*>(src);
        srcName_ = cMod->getFullPath();
    }
    doubleValue_ = d;
    counter_ ++;
    sumValue_ += d;
    avgValue_ = sumValue_ / counter_;
    maxValue_ = counter_ > 10 ? std::max(maxValue_, doubleValue_) : doubleValue_;
    minValue_ = counter_ > 10 ? std::min(minValue_, doubleValue_) : doubleValue_;
}

void SignalListener::receiveSignal(cComponent* src, simsignal_t id, intval_t i, cObject* details) {
    if (srcName_.empty()) {
        auto cMod = dynamic_cast<cModule*>(src);
        srcName_ = cMod->getFullPath();
    }
    intValue_ = i;
    counter_ ++;
    sumValue_ += (double)i;
    avgValue_ = sumValue_ / counter_;
    maxValue_ = counter_ > 10 ? std::max(maxValue_, double(intValue_)) : intValue_;
    minValue_ = counter_ > 10 ? std::min(minValue_, double(intValue_)) : intValue_;
}

void SignalListener::receiveSignal(cComponent* source, simsignal_t signalID, uintval_t i, cObject* details) {
    receiveSignal(source, signalID, (intval_t )i, details);
}

void SignalListener::receiveSignal(cComponent* source, simsignal_t signalID, const SimTime& t, cObject* details) {
    receiveSignal(source, signalID, t.dbl(), details);
}

int SignalListener::getCounter() {
    return counter_;
}

std::string SignalListener::getSrcName() {
    return srcName_;
}

int SignalListener::getIntValue() {
    int tmpValue = intValue_;
    intValue_ = 0;
    return tmpValue;
}

double SignalListener::getDoubleValue() {
    int tmpValue = doubleValue_;
    doubleValue_ = 0;
    return tmpValue;
}

cObject* SignalListener::getObjValue() {
    cObject* tmpPtr = objValue_;
    objValue_ = nullptr;
    return tmpPtr;
}

double SignalListener::getAvgValue() {
    return avgValue_;
}

double SignalListener::getMinValue() {
    return minValue_;
}

double SignalListener::getMaxValue() {
    return maxValue_;
}
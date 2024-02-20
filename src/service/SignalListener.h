/*
 * SignalListener.h
 *
 * A generic listener for the signals emitted by OMNeT++ modules
 *
 * Created on: Aug 22, 2022
 *      Author: Nikita Smirnov
 */

#ifndef SIGNALLISTENER_H
#define SIGNALLISTENER_H

#include <inet/common/packet/Packet.h>
#include <omnetpp/clistener.h>
#include <omnetpp/cmodule.h>

using namespace inet;
using namespace omnetpp;

class SignalListener: public cListener {
public:
    SignalListener();
    virtual ~SignalListener();

    int getCounter();
    std::string getSrcName();
    double getDoubleValue();
    int getIntValue();
    cObject* getObjValue();
    double getAvgValue();
    double getMinValue();
    double getMaxValue();

protected:
    void receiveSignal(cComponent* src, simsignal_t id, cObject* object, cObject* details) override;
    void receiveSignal(cComponent* src, simsignal_t id, double d, cObject* details) override;
    void receiveSignal(cComponent* source, simsignal_t signalID, intval_t i, cObject* details) override;
    void receiveSignal(cComponent* source, simsignal_t signalID, uintval_t i, cObject* details) override;
    void receiveSignal(cComponent* source, simsignal_t signalID, const SimTime& t, cObject* details) override;

private:
    int counter_;
    std::string srcName_;
    
    // vals
    int intValue_;
    double doubleValue_;
    cObject* objValue_;

    double sumValue_;
    double avgValue_;
    double minValue_;
    double maxValue_;
};

#endif /* SIGNALLISTENER_H */

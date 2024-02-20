/*
 * NRChannelModelWithFeedback.h
 *
 * this class duplicates NRChannelModel_3GPP38_901 channel model from simu5g with one addition: it sends UL/DL SINR/RSRP signals in ::isError method to the upper PHY layer.
 * The corresponding PHY Layer must be of type NRPhyUeWithFeedback.
 *
 *  Created on: May 1, 2023
 *      Author: Nikita Smirnov
 */

#ifndef NRCHANNELMODELWITHFEEDBACK_H
#define NRCHANNELMODELWITHFEEDBACK_H

#include <stdexcept>

#include "common/LteCommon.h"
#include "stack/phy/layer/NRPhyUe.h"
#include "stack/phy/ChannelModel/NRChannelModel_3GPP38_901.h"
#include "omnetpp.h"

struct SignalFeedback {
    bool isNew = false;
    int direction = -1;
    double carrierFrequency = 0.0;
    int numUsedRbs = 0;
    double sinr = 0.0;
    double rsrp = 0.0;
    double bler = 0.0;
    double harqper = 0.0;
};

class NRChannelModelWithFeedback : public NRChannelModel_3GPP38_901 {
public:
    NRChannelModelWithFeedback() = default;
    virtual ~NRChannelModelWithFeedback() = default;

    SignalFeedback* getSignalFeedback();
    void ackSignalFeedback();
protected:
    virtual void initialize(int stage) override; 
    virtual void finish() override;
    
    bool isError(LteAirFrame *frame, UserControlInfo* lInfo) override;
private:
    SignalFeedback* signalFeedback_;
};

#endif

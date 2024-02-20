/*
 * NRPhyUeWithFeedback.h
 *
 * this class duplicates NRPhyUe model from simu5g with one addition: it keeps the PhyDataState object, which is updated with DL/UL Tx params and signals from the channel model
 *
 *  Created on: May 1, 2023
 *      Author: Nikita Smirnov
 */

#ifndef NRPHYWITHFEEDBACK_H_
#define NRPHYWITHFEEDBACK_H_

#include <numeric>
#include <stdexcept>

#include "common/LteCommon.h"
#include "stack/phy/layer/NRPhyUe.h"
#include "omnetpp.h"

#include "domain/CSVLogger.h"
#include "model/states/PhyDataState.h"
#include "omnet/stack/phy/channelModel/NRChannelModelWithFeedback.h"
#include "PhyData.h"

class NRPhyUeWithFeedback : public NRPhyUe {
public:
    NRPhyUeWithFeedback();
    virtual ~NRPhyUeWithFeedback();

    const std::unordered_map<double, PhyDataState>& getPhyDataStateMap();
protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage* msg) override;
    virtual void handleAirFrame(cMessage* msg) override; // DL
    virtual void handleUpperMessage(cMessage* msg) override; // UL
private:
    std::unique_ptr<PhyData> packPhyData(UserControlInfo* lteInfo, LteAirFrame* frame);
    void logPhyDataState();

protected:
    double feedbackStartTime;
    double logUpdatePeriod;
    cMessage* logUpdateTimer;
    int verbose;
private:
    std::unordered_map<double, PhyDataState> phyDataStatesMap_;
};

#endif

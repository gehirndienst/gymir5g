/*
 * UnitState.h
 *
 * unit (device) states that are input to adaptive streaming algortihms
 * has all other states as members
 *
 *  Created on: Aug 23, 2023
 *      Author: Nikita Smirnov
 */

#ifndef UNITSTATE_H
#define UNITSTATE_H

#include "domain/CSVLogger.h"
#include "model/states/DataTransmissionState.h"
#include "model/states/PhyDataState.h"
#include "model/states/States.h"
#include "model/states/StreamState.h"
#include "omnet/stack/phy/NRPhyUeWithFeedback.h"

// TODO:

class UnitState {
public:
    UnitState();
    ~UnitState() = default;

    void set(const std::string& streamName, int historySize = 100, int verbose = 0);

    // TODO: add PhyDataManager that will manage a map of PhyDataStates for several CCs
    //std::optional<PhyDataState&> getPhyState(NRPhyUeWithFeedback* sphy);

    StreamState& getStreamState();
    DataTransmissionState& getTransmissionState();

    /* mutual exclusion: all getters must be called in a following block
    {
        std::unique_lock<std::mutex> lock(unitState.getStateMutex());
        auto& state = unitState.get**State();
        state.method(..);
        ..
    }
    */
    //std::mutex& getStateMutex(); // FIXME: leave mutex by Stream?

private:
    // PhyDataState phyState_;
    StreamState streamState_;
    DataTransmissionState transmissionState_;

    //mutable std::mutex stateMutex_;
};

#endif
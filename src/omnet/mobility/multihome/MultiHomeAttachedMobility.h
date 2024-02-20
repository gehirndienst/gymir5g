/*
 * MultiHomeAttachedMobility.h
 *
 * Attached mobility, where the main mobility is set dynamically
 *
 * Created on: Sep 13, 2023
 *      Author: Nikita Smirnov
 */


#ifndef MULTIHOMEATTACHEDMOBILITY_H
#define MULTIHOMEATTACHEDMOBILITY_H

#include "inet/common/INETDefs.h"
#include "inet/mobility/base/MobilityBase.h"
#include "inet/mobility/static/StationaryMobility.h"

using namespace inet;

class MultiHomeAttachedMobility : public MobilityBase, public cListener {
  protected:
    simtime_t initTime;
    double maxWaitingTimeToStart;

    // now it is a mutable and reassignable pointer
    IMobility* mobility;

    Coord positionOffset = Coord::NIL;
    Quaternion orientationOffset = Quaternion::NIL;
    bool isZeroOffset = false;
    Coord lastVelocity;
    Quaternion lastAngularPosition;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleSelfMessage(cMessage *msg) override;

  private:
    void findMainMobility();

  public:
    virtual const Coord& getCurrentPosition() override;
    virtual const Coord& getCurrentVelocity() override;
    virtual const Coord& getCurrentAcceleration() override;

    virtual const Quaternion& getCurrentAngularPosition() override;
    virtual const Quaternion& getCurrentAngularVelocity() override;
    virtual const Quaternion& getCurrentAngularAcceleration() override;

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

#endif


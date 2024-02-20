/*
 * DockingMobility.h
 *
 * docking mobility to model docking scenarios for the ship
 *
 * Created on: Nov 17, 2023
 *      Author: Nikita Smirnov
 */


#ifndef DOCKINGMOBILITY_H
#define DOCKINGMOBILITY_H

#include <fstream>
#include <iostream>

#include "inet/common/INETDefs.h"
#include "inet/mobility/base/MovingMobilityBase.h"

#include "nlohmann/json.hpp"

#include "WaypointType.h"

using namespace inet;
using namespace omnetpp;

struct Waypoint {
    double x;
    double y;
    double speed;
    WaypointType type;
    bool isPassed;
};

class DockingMobility : public MovingMobilityBase, public cListener {
protected:
    virtual void initialize(int stage) override;
    virtual void setInitialPosition() override;
    virtual void move() override;

    virtual void fillWaypoints(nlohmann::json& waypointsJson);

    bool isDocking();

protected:
    double heading;
    double waypointProximity;

    double dockingStopPeriod;
    simtime_t dockingStartTime;

    std::vector<Waypoint> waypoints;
    int lastWaypointIndex;
};

#endif


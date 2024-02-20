package gymir5g.omnet.mobility.docking;

//
// docking mobility model
//
// Created on: Nov 17, 2023
//     Author: Nikita Smirnov
//

import inet.mobility.base.MovingMobilityBase;

simple DockingMobility extends MovingMobilityBase
{
    parameters:
        @class(DockingMobility);

        string waypointsFile; // objects[{name,x,y,speed,type}...]

        double waypointProximity @unit(m) = default(5m);
        double dockingStopPeriod @unit(s) = default(30s);

        double initialX @unit(m) = default(uniform(constraintAreaMinX, constraintAreaMaxX));
        double initialY @unit(m) = default(uniform(constraintAreaMinY, constraintAreaMaxY));
        double initialZ @unit(m) = default(nanToZero(uniform(constraintAreaMinZ, constraintAreaMaxZ)));
        bool initFromDisplayString = default(true);
}

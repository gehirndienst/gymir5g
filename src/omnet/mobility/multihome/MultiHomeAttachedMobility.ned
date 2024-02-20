package gymir5g.omnet.mobility.multihome;

//
// multi home mobility attached to a dynamically generated main mobility
//
// Created on: Sep 13, 2023
//     Author: Nikita Smirnov
//

import inet.mobility.base.MobilityBase;

simple MultiHomeAttachedMobility extends MobilityBase
{
    parameters:
        @class(MultiHomeAttachedMobility);
        
        double maxWaitingTimeToStart @unit(s) = default(0.5s);
        
        // copied parameters from AttachedMobility
        string mobilityModule;
        double offsetX @unit(m) = default(0m);
        double offsetY @unit(m) = default(0m);
        double offsetZ @unit(m) = default(0m);
        double offsetHeading @unit(deg) = default(0deg);
        double offsetElevation @unit(deg) = default(0deg);
        double offsetBank @unit(deg) = default(0deg);
}

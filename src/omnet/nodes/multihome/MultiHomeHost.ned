package gymir5g.omnet.nodes.multihome;

import inet.applications.contract.IApp;
import inet.mobility.contract.IMobility;
import gymir5g.omnet.apps.multihome.IMultiHomeApp;
//
// multi home hosts
//
// Created on: Aug 4, 2023
//     Author: Nikita Smirnov
//

module MultiHomeHost {
    parameters:
        @networkNode();
        int numApps = default(1);
        mobility.typename = default("StationaryMobility");
        
    submodules:
        app[numApps]: <> like IApp {
            @display("p=374.47998,72.479996,row,140");
        }
        mobility: <default("")> like IMobility if typename != "" {
            @display("p=125,160;is=s");
        }
}


// with IMultiHomeApp interface backward compatibility for legacy multihome scenarios
module MultiHomeHost2 {
    parameters:
        @networkNode();
        int numApps = default(1);
        mobility.typename = default("StationaryMobility");
        
    submodules:
        app[numApps]: <> like IMultiHomeApp {
            @display("p=374.47998,72.479996,row,140");
        }
        mobility: <default("")> like IMobility if typename != "" {
            @display("p=125,160;is=s");
        }
}
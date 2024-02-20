package gymir5g.simulations.anleger;

import simu5g.nodes.NR.NRUe;
import gymir5g.simulations.anleger.Anleger_SA_Base;

//
// SA scenario for docking maneuver exploiting WebRTC like media streams
//
// Created on: Nov 21, 2023
// Author: Nikita Smirnov
// Organization: Intelligent Systems, CAU Kiel
// 

network Anleger_SA extends Anleger_SA_Base
{
    parameters:
        @display("bgb=7170.2715,5163.574");

    submodules:
        ship: NRUe {
            @display("p=2035.7981,1994.462;ls=red,3");
        }
}

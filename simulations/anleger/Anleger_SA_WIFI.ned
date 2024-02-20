package gymir5g.simulations.anleger;

import inet.node.ethernet.Eth100M;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;

import simu5g.nodes.NR.NRUe;

import gymir5g.omnet.nodes.multihome.MultiHomeHost;
import gymir5g.omnet.nodes.multihome.MultiHomeHost2;
import gymir5g.simulations.anleger.Anleger_SA_Base;

//
// SA scenario for docking maneuver with APs from Addix. Multihome functionalities are turned on
//
// Created on: Nov 22, 2023
// Author: Nikita Smirnov
// Organization: Intelligent Systems, CAU Kiel
// 

network Anleger_SA_WIFI extends Anleger_SA_Base
{
    parameters:
        @display("bgb=7170.2715,5163.574");

    submodules:
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=1519.098,403.026;is=s");
        }

        // REVENTLOUBRÜCKE

        // rev
        AP743E2B2C0BA0: AccessPoint {
            parameters:
                @display("p=1875.9750765560707,1504.3178344235566;is=s");
        }

        // rev
        AP187C0B182980: AccessPoint {
            parameters:
                @display("p=2177.32937395012,1128.6867882094157;is=s");
        }


        // rev
        AP187C0B1835C0: AccessPoint {
            parameters:
                @display("p=2093.9266466813674,1210.4032165377212;is=s");
        }

        // rev
        AP187C0B111340: AccessPoint {
            parameters:
                @display("p=1885.2461605358217,1614.120207409076;is=s");
        }

        // rev
        APF03E901ECA20: AccessPoint {
            parameters:
                @display("p=2037.4935698607983,1450.1530102274191;is=s");
        }

        // rev
        APF03E901EA200: AccessPoint {
            parameters:
                @display("p=1906.2699683274654,1216.5861923039702;is=s");
        }

        // rev
        AP24792A08DE30: AccessPoint {
            parameters:
                @display("p=1875.7142503919126,1540.1571812645343;is=s");
        }

        // rev
        AP8C7A1532E0F0: AccessPoint {
            parameters:
                @display("p=1927.2932900212286,1301.2679249184948;is=s");
        }

        // DIETRICHSDORF

        // di
        APF8E71E1FA780: AccessPoint {
            @display("p=3584.2644444374414,1692.0326723313701;is=s");
        }

        // di
        AP8C7A1531AC40: AccessPoint {
            parameters:
                @display("p=3675.862991698417,1739.8048961383101;is=s");
        }
        // di
        AP4CB1CD360060: AccessPoint {
            parameters:
                @display("p=3444.304923001658,1501.500246078409;is=s");
        }

        // di
        AP60D02C238DE0: AccessPoint {
            parameters:
                @display("p=3312.742136573284,1764.4420767013726;is=s");
        }

        // di
        APF03E90110140: AccessPoint {
            parameters:
                @display("p=3726.148453954472,2214.9525149023902;is=s");
        }

        // multihome ship: the usual one NRUe ship is not used, it is used in Anleger_NSA
        multiship: MultiHomeHost {
            @display("is=n;i=device/server;p=2011.731,1917.6572");
        }

        multiue: NRUe {
            @display("p=2141.987,1892.3297");
        }

        multiwh: WirelessHost {
            @display("r=,,#707070;p=1856.1475,1892.3297");
        }

    connections allowunconnected:
        // r
        AP743E2B2C0BA0.ethg++ <--> Eth100M <--> router.ethg++;
        AP187C0B182980.ethg++ <--> Eth100M <--> router.ethg++;
        AP187C0B1835C0.ethg++ <--> Eth100M <--> router.ethg++;
        AP187C0B111340.ethg++ <--> Eth100M <--> router.ethg++;
        APF03E901ECA20.ethg++ <--> Eth100M <--> router.ethg++;
        APF03E901EA200.ethg++ <--> Eth100M <--> router.ethg++;
        AP24792A08DE30.ethg++ <--> Eth100M <--> router.ethg++;
        AP8C7A1532E0F0.ethg++ <--> Eth100M <--> router.ethg++;
        // d
        APF8E71E1FA780.ethg++ <--> Eth100M <--> router.ethg++;
        AP8C7A1531AC40.ethg++ <--> Eth100M <--> router.ethg++;
        AP4CB1CD360060.ethg++ <--> Eth100M <--> router.ethg++;
        AP60D02C238DE0.ethg++ <--> Eth100M <--> router.ethg++;
        APF03E90110140.ethg++ <--> Eth100M <--> router.ethg++;

}

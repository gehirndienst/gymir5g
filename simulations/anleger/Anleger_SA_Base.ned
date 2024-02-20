package gymir5g.simulations.anleger;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.ethernet.Eth10G;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.visualizer*.common.IntegratedVisualizer;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;

import simu5g.common.binder.Binder;
import simu5g.common.carrierAggregation.CarrierAggregation;
import simu5g.nodes.backgroundCell.BackgroundCell;
import simu5g.nodes.eNodeB;
import simu5g.nodes.NR.gNodeB;
import simu5g.nodes.NR.NRUe;
import simu5g.nodes.Upf;
import simu5g.world.radio.LteChannelControl;

//
// Scenario for docking maneuver, exploits Reventloubrücke and Dietrichsdorf docking as well as sailing between them
// in the scope of F2 Schwentine ferry route in Kiel, Germany. 
// The main agent uses a custom docking mobility. 
// The base cellular connection here is assumed to be Vodafone 5G SA!
//
// Created on: Nov 21, 2023
// Author: Nikita Smirnov
// Organization: Intelligent Systems, CAU Kiel
// 

network Anleger_SA_Base
{
    parameters:
        @display("bgb=7170.2715,5163.574");

        // dl bg traffic ratio
        double dlBgTrafficRatio = default(0.75);
    submodules:
        // inet
        routingRecorder: RoutingTableRecorder {
            @display("p=5835.6113,2296.5955;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=5841.886,2723.2852");
            config = xmldoc("demo.xml");
        }
        router: Router {
            @display("p=4174.936,1632.7721;i=device/smallrouter;is=s");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=6778.7207,1044.1351;is=s");
        }

        // simu5g
        channelControl: LteChannelControl {
            @display("p=5841.886,2001.6774;is=s");
        }
        upf: Upf {
            @display("p=3616.9001,1891.1221;is=s");
        }
        iUpf: Upf {
            @display("p=3854.582,1891.1221;is=s");
        }
        binder: Binder {
            @display("p=5835.6113,3024.478;is=s");
        }
        carrierAggregation: CarrierAggregation {
            @display("p=5835.6113,3313.121;is=s");
        }

		// communication participants
        shore: StandardHost {
            @display("p=3735.614838820165,1497.1952661980686;is=s;i=device/server");
        }

        // enbs
        eNB23201: gNodeB {
            @display("p=2900.73951838478,2631.1304712972415;is=s");
        }

        eNB21430: gNodeB {
            @display("p=4312.803389692777,2126.4607051989806;is=s");
        }

        eNB23654: gNodeB {
            @display("p=4645.8561648811265,797.2911173884568;is=s");
        }

        eNB26135: gNodeB {
            @display("p=1930.6380343701271,1328.4680338443068;is=s");
        }

        eNB24141: gNodeB {
            @display("p=1636.2871083448222,1354.8879758731273;is=s");
        }

        eNB24143: gNodeB {
            @display("p=1492.3913746562134,1837.3371646755186;is=s");
        }

    connections allowunconnected:
        shore.pppg++ <--> Eth10G <--> router.pppg++;
        router.pppg++ <--> Eth10G <--> upf.filterGate;
        upf.pppg++ <--> Eth10G <--> iUpf.pppg++;

        iUpf.pppg++ <--> Eth10G <--> eNB23201.ppp;
        iUpf.pppg++ <--> Eth10G <--> eNB21430.ppp;
        iUpf.pppg++ <--> Eth10G <--> eNB23654.ppp;
        iUpf.pppg++ <--> Eth10G <--> eNB26135.ppp;
        iUpf.pppg++ <--> Eth10G <--> eNB24141.ppp;
        iUpf.pppg++ <--> Eth10G <--> eNB24143.ppp;

        eNB23201.x2++ <--> Eth10G <--> eNB21430.x2++;
        eNB23201.x2++ <--> Eth10G <--> eNB23654.x2++;
        eNB23201.x2++ <--> Eth10G <--> eNB26135.x2++;
        eNB23201.x2++ <--> Eth10G <--> eNB24141.x2++;
        eNB23201.x2++ <--> Eth10G <--> eNB24143.x2++;
        eNB21430.x2++ <--> Eth10G <--> eNB23654.x2++;
        eNB21430.x2++ <--> Eth10G <--> eNB26135.x2++;
        eNB21430.x2++ <--> Eth10G <--> eNB24141.x2++;
        eNB21430.x2++ <--> Eth10G <--> eNB24143.x2++;
        eNB23654.x2++ <--> Eth10G <--> eNB26135.x2++;
        eNB23654.x2++ <--> Eth10G <--> eNB24141.x2++;
        eNB23654.x2++ <--> Eth10G <--> eNB24143.x2++;
        eNB26135.x2++ <--> Eth10G <--> eNB24141.x2++;
        eNB26135.x2++ <--> Eth10G <--> eNB24143.x2++;
        eNB24141.x2++ <--> Eth10G <--> eNB24143.x2++;
}

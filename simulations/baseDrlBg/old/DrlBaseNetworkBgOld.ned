package gymir5g.simulations.baseDrlBg.old;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.ethernet.Eth10G;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;
import simu5g.common.binder.Binder;
import simu5g.common.carrierAggregation.CarrierAggregation;
import simu5g.nodes.backgroundCell.BackgroundCell;
import simu5g.nodes.eNodeB;
import simu5g.nodes.NR.NRUe;
import simu5g.nodes.NR.gNodeB;
import simu5g.nodes.PgwStandard;
import simu5g.world.radio.LteChannelControl;

network DrlBaseNetworkBgOld
{
    parameters:
        int numBgCells = default(3);
        @display("i=block/network2;bgb=991,800;");
    submodules:
        channelControl: LteChannelControl {
            @display("p=75.86875,22.3875;is=s");
        }
        routingRecorder: RoutingTableRecorder {
            @display("p=77.1125,79.6;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=77.1125,144.275");
        }
        binder: Binder {
            @display("p=77.1125,208.95;is=s");
        }
        carrierAggregation: CarrierAggregation {
            @display("p=77.1125,272.38126;is=s");
        }
        router: Router {
            @display("p=321,136;i=device/smallrouter");
        }
        pgw: PgwStandard {
            @display("p=527.35,116.9125;is=l");
        }
        eNodeB1: eNodeB {
            @display("p=320.8875,264.91876;is=vl");
        }
        eNodeB2: eNodeB {
            @display("p=773.6125,284.81876;is=vl");
        }
        gNodeB1: gNodeB {
            @display("p=320.8875,264.91876;is=vl");
        }
        gNodeB2: gNodeB {
            @display("p=773.6125,284.81876;is=vl");
        }
        ship: NRUe {
            @display("p=353.225,383.075;ls=red,3");
        }
        shore: StandardHost {
            @display("p=240.04375,88.30625;is=n;i=device/server");
        }
        bgCell[numBgCells]: BackgroundCell {
            @display("p=130.59375,468.89374;is=vl");
        }
        visualizer: IntegratedCanvasVisualizer {
            @display("p=924.10626,19.9;is=s");
        }
    connections:
        shore.pppg++ <--> Eth10G <--> router.pppg++;
        router.pppg++ <--> Eth10G <--> pgw.filterGate;
        pgw.pppg++ <--> Eth10G <--> eNodeB1.ppp;
        pgw.pppg++ <--> Eth10G <--> eNodeB2.ppp;
        //# X2 
        // m <-> s
        eNodeB1.x2++ <--> Eth10G <--> gNodeB1.x2++;
        eNodeB2.x2++ <--> Eth10G <--> gNodeB2.x2++;
		// m <-> m
        eNodeB1.x2++ <--> Eth10G <--> eNodeB2.x2++;
}

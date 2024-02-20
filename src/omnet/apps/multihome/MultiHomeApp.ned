package gymir5g.omnet.apps.multihome;

//
// multi home application for one cellular and (so far) one wireless interface
//
// Created on: Aug 4, 2023
//     Author: Nikita Smirnov
//

moduleinterface IMultiHomeApp {
	gates:
        input directIn @directIn; // redirect acks, reports or requests from cellular/wireless apps
}

simple MultiHomeUdpApp like IMultiHomeApp {
    parameters:
    	@class(MultiHomeUdpApp);
    	
    	double maxWaitingTimeToStart @unit(s) = default(0.5s);
        
		bool isNR @mutable = default(true);
		
		//# a feature of that design is that both ue and wh are on the same level of module hiereachy
		string cellularAppPath = default("multiue.app[0]");
		string wirelessAppPath = default("multiwh.app[0]"); // FiXME: add multiple here
		
        volatile int packetSize @unit(byte) = default(1000B);
        volatile double sendingPeriod @unit(s) = default(0.04s);
        bool isScheduleAfterAppsAreFound = default(true); // whether to schedule sending data right after the apps are found
        
        @signal[wlanPacketsSent](type ="long");
        @statistic[wlanPacketsSent](title="Number of sent WLAN pkts"; unit=""; source="wlanPacketsSent"; record=count);
    	@signal[ltePacketsSent](type ="long");
        @statistic[ltePacketsSent](title="Number of sent LTE pkts"; unit=""; source="ltePacketsSent"; record=count);
    	@signal[nrPacketsSent](type ="long");
        @statistic[nrPacketsSent](title="Number of sent NR pkts"; unit=""; source="nrPacketsSent"; record=count);
    	@signal[dontcarePacketsSent](type ="long");
        @statistic[dontcarePacketsSent](title="Number of sent DONTCARE pkts"; unit=""; source="dontcarePacketsSent"; record=count);
	gates:
        input directIn @directIn;
}
package gymir5g.omnet.apps.multihome;

import inet.applications.contract.IApp;
import inet.applications.udpapp.UdpSink;

//
// forward apps working in a combination with multi home apps. Installed on subinterfaces,
// they just forward the data received from dataIn gate via wrapping it into socket calls.
// They could be set for every standard module since they inherit IApp.
//
// Created on: Aug 4, 2023
//     Author: Nikita Smirnov
//

simple ForwardUdpApp like IApp {
    parameters:
    	@class(ForwardUdpApp);

        string multiHomeAppPath;

        string destAddress;
        int destPort;
        int localPort;

        string packetName = default("UDPData");
    gates:
        input socketIn;
        output socketOut;
        
        input dataIn @directIn; // input for sendDirect calls from multihome app
        input directIn @directIn; // input for sendDirect calls from external app (same as directIn by ship app)
}

simple ForwardShipApp extends ForwardUdpApp {
    parameters:
    	@class(ForwardShipApp);
        packetName = "ShipData";
}


// specific forward apps for shore that mimics inet interface to forward wlan data to ShoreApp
simple ForwardShoreApp extends UdpSink {
    parameters:
        @class(ForwardShoreApp);

        string logdir = default("null");
        double logUpdatePeriod @unit(s) = default(1s);
        bool verbose = default(false);
}
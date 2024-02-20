//
//
// Manages real and virtual streams, creates and sends datagrams with acquired stream data, manages transmission stats
// and communicates with Gymnasium for adaptive communication using Deep Reinforcement Learning
//
// Created on: July 21, 2022
//      Author: Nikita Smirnov
//

package gymir5g.apps.ship; 

import inet.applications.contract.IApp;

simple ShipApp like IApp 
{
    parameters:
        @class(ShipApp);

        // self
        double warmUp @unit(s) = default(uniform(0.001s, 0.01s));
        double maxWaitingTimeToStart @unit(s) = default(0.5s);
        double maxWaitingTimeForMobility @unit(s) = default(5s);
        bool isVeinsMobility = default(false);
        
        // connection
        string destAddress;
        int destPort;
        int localPortData;

        // sending options
        int tos = default(-1);
        bool isSendingSimultaneously = default(true); // whether to send all fragments of one stream element simultaneously or one-by-one with an equal delay in-between
        bool isUsingFutureData = default(true); // whether to use right-after encoding to speed up the data processing

        // multihoming
        bool isMultiHome = default(false); // whether to use multiple interfaces for sending
        string cellularAppPath = default("multiue.app[0]");
		string wirelessAppPath = default("multiwh.app[0]");

        // rtc
        double senderReportPeriod @mutable @unit(s) = default(1s); // sender report period for RTCP, 0 to turn off
        double maxRetransmitAge @unit(s) = default(1s); // how long to keep sent packets for retranmission requests, 0 to turn off

        bool isFec @mutable = default(false); // whether to use FEC (ONLY for SIM streams). NOTE: by default ULPFEC is simulated, meaning that for key frames the protection level is higher
        int fecAfterPackets @mutable = default(10); // how often to create a fec packet. NORE: it is assumed that RTP packets has a fixed upper-bound payload (e.g., 1200 bytes) and are more or less equal in size

        // streams
        string streamsConfigurationSchema = default("../../configurations/streams.json");
        
        // adaptive streaming
        string adaptiveAlgorithm = default(""); // see <common/AdaptiveAlgorithm.h>
        double stateUpdatePeriod @unit(s) = default(1s); // how often do we produce a new state (and send to the DRL model in case of AS)
        string hostAddress = default("127.0.0.1:5555"); // tcp zmq-based host address of the gym server
        int noReplyTimeout @unit(ms) = default(30000ms); // max time for a reply from the connected python-process

        // logging
        string logdir = default(""); // log directory to write, supplying "null" turns off any logging
        double logUpdatePeriod @unit(s) = default(1s); // NOTE: in case of AS it becomes automatically equal to stateUpdatePeriod
  
    gates:
        input socketIn @labels(UdpControlInfo/up) @loose;
        output socketOut @labels(UdpControlInfo/down) @loose;

        // to emulate lossless statistic as if AS state is calculated at the receiver side without changing the arch,
        // it is enough to send formed packets loslessly and w/o delay via sendDirect instead, in that case use this special gate.
        // To enable direct transmission of certain pakcets, enable existing "isDirect*" params at the receiver app side.
        input directIn @directIn;
}

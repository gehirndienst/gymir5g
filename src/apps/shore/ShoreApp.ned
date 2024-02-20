//
//
// Receives, decodes and processes datagrams with the data from different streams
//
// Created on: July 27, 2022
//      Author: Nikita Smirnov
//

package gymir5g.apps.shore; 

import inet.applications.contract.IApp;

simple ShoreApp like IApp 
{
    parameters:
        @class(ShoreApp);

        // self
        double warmUp @unit(s) = default(uniform(0.001s, 0.01s));
        double maxWaitingTimeToStart @unit(s) = default(0.5s);
        
        // connection
        string destAddress;
        int destPort;
        int localPortData;
        
        // sending options
        int tos = default(-1);

        // rtc
        bool isAck = default(false); // whether to send ACK for each packet
        bool isSack = default(false); // whether to send SACK for marked packets

        bool isNack @mutable = default(false); // whether to send NACK for each lost packet
        int maxNacks @mutable = default(2); // number of max possible consecutive NACKs for a single packet
        double maxNackDelay @mutable @unit(s) = default(0.05s); // max waiting time bw consecutive NACKs for a single packet
        double maxPlayoutDelay @mutable @unit(s) = default(0.1s); // max common waiting time for a missing RTP packet, meanwhile other packets wait and are not decoded. Could be smaller than maxNacks * maxNackDelay

        bool isFec = default(false); // whether to create a repair buffer and handle FEC, NOTE: turn off for now
        double repairPeriod @unit(s) = default(0.05s); // how often to check for being able to repair packets

        bool isFir @mutable = default(false); // whether to send FIR (Full Intra Request, here is the same as PLI) for resending the whole frame // FIXME: is not implemented so far
        
        double receiverReportPeriod @mutable @unit(s)= default(1s); // 0 to turn off
        double transportFeedbackPeriod @mutable @unit(s)= default(0.1s); // 0 to turn off
        bool isDirectReceiverReport = default(false); // whether to send RRs stats via sendDirect without delay and loss
        bool isDirectTransportFeedback = default(false); // whether to send TFs stats via sendDirect without delay and loss

        // streams decoding
        bool isDecode = default(false); // whether to decode received data (except sim streams)
        bool isView = default(false); // whether to play received frames for video and lidar streams

        // verbose mode: 0 -- stdout: off, logger: off, 1 -- stdout: off, logger: on, 2 -- stdout: on, logger: on
         // TODO: add logging at the receiver side?
        int verbose = default(1);

    gates:
        input socketIn @labels(UdpControlInfo/up);
        output socketOut @labels(UdpControlInfo/down);
}

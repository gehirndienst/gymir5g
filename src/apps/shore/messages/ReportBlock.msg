import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

//
// an auxilary class for Report Block chunks that consitute Receiver Report 
//

class ReportBlock extends inet::FieldsChunk {
    chunkLength = inet::B(36); // initially 24, increased due to extended RBs
    
    // ssrc
    string streamName;

     // packet loss rate bw two consecutive RRs
    double fractionLost;

    // packets expected minus the number of packets received
    int packetsLostCumulative;

    // extended highest sequence number received
    int maxSequenceNumber;

    // interarrival jitter.
    double jitter;

    // timestamp of the last SenderReport
    omnetpp::simtime_t lastSR;

    // delay since the last SenderReport
    omnetpp::simtime_t delaySinceLastSR;
    
    //////////////////// extended RR
    double fractionRate;
    double fractionFecRate;
    int packetsReceivedCumulative;
    int packetsOutOfOrderCumulative;
    int packetsPlayedCumulative;
    int packetsRetransmittedCumulative; // if NACK is on else 0
    int packetsRepairedCumulative; // if FEC is on else 0
    int packetsRepairedAndRetransmittedCumulative;
    long bytesReceivedCumulative;
    double fractionInterarrivalDelayMean;
    double fractionTransmissionDelayMean;
    double fractionRetransmissionDelayMean;
    double fractionPlayoutDelayMean; // if maxPlayoutDelay > 0 in shore app else 0
    double fractionStallingRate; // if playback delay > 300 ms
    int bandwidth; // from GCC
};

cplusplus(ReportBlock) {{
  public:
    virtual std::string str() const override;
    virtual void dump(std::ostream& os) const;
}}
//
// Generated file, do not edit! Created by opp_msgtool 6.0 from apps/shore/messages/ReportBlock.msg.
//

#ifndef __REPORTBLOCK_M_H
#define __REPORTBLOCK_M_H

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>

// opp_msgtool version check
#define MSGC_VERSION 0x0600
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of opp_msgtool: 'make clean' should help.
#endif

class ReportBlock;
#include "inet/common/INETDefs_m.h" // import inet.common.INETDefs

#include "inet/common/packet/chunk/Chunk_m.h" // import inet.common.packet.chunk.Chunk

/**
 * Class generated from <tt>apps/shore/messages/ReportBlock.msg:8</tt> by opp_msgtool.
 * <pre>
 * class ReportBlock extends inet::FieldsChunk
 * {
 *     chunkLength = inet::B(36); // initially 24, increased due to extended RBs
 * 
 *     // ssrc
 *     string streamName;
 * 
 *      // packet loss rate bw two consecutive RRs
 *     double fractionLost;
 * 
 *     // packets expected minus the number of packets received
 *     int packetsLostCumulative;
 * 
 *     // extended highest sequence number received
 *     int maxSequenceNumber;
 * 
 *     // interarrival jitter.
 *     double jitter;
 * 
 *     // timestamp of the last SenderReport
 *     omnetpp::simtime_t lastSR;
 * 
 *     // delay since the last SenderReport
 *     omnetpp::simtime_t delaySinceLastSR;
 * 
 *     //////////////////// extended RR
 *     double fractionRate;
 *     double fractionFecRate;
 *     int packetsReceivedCumulative;
 *     int packetsOutOfOrderCumulative;
 *     int packetsPlayedCumulative;
 *     int packetsRetransmittedCumulative; // if NACK is on else 0
 *     int packetsRepairedCumulative; // if FEC is on else 0
 *     int packetsRepairedAndRetransmittedCumulative;
 *     long bytesReceivedCumulative;
 *     double fractionInterarrivalDelayMean;
 *     double fractionTransmissionDelayMean;
 *     double fractionRetransmissionDelayMean;
 *     double fractionPlayoutDelayMean; // if maxPlayoutDelay > 0 in shore app else 0
 *     double fractionStallingRate; // if playback delay > 300 ms
 *     int bandwidth; // from GCC
 * }
 * </pre>
 */
class ReportBlock : public ::inet::FieldsChunk
{
  protected:
    omnetpp::opp_string streamName;
    double fractionLost = 0;
    int packetsLostCumulative = 0;
    int maxSequenceNumber = 0;
    double jitter = 0;
    omnetpp::simtime_t lastSR = SIMTIME_ZERO;
    omnetpp::simtime_t delaySinceLastSR = SIMTIME_ZERO;
    double fractionRate = 0;
    double fractionFecRate = 0;
    int packetsReceivedCumulative = 0;
    int packetsOutOfOrderCumulative = 0;
    int packetsPlayedCumulative = 0;
    int packetsRetransmittedCumulative = 0;
    int packetsRepairedCumulative = 0;
    int packetsRepairedAndRetransmittedCumulative = 0;
    long bytesReceivedCumulative = 0;
    double fractionInterarrivalDelayMean = 0;
    double fractionTransmissionDelayMean = 0;
    double fractionRetransmissionDelayMean = 0;
    double fractionPlayoutDelayMean = 0;
    double fractionStallingRate = 0;
    int bandwidth = 0;

  private:
    void copy(const ReportBlock& other);

  protected:
    bool operator==(const ReportBlock&) = delete;

  public:
    ReportBlock();
    ReportBlock(const ReportBlock& other);
    virtual ~ReportBlock();
    ReportBlock& operator=(const ReportBlock& other);
    virtual ReportBlock *dup() const override {return new ReportBlock(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    virtual const char * getStreamName() const;
    virtual void setStreamName(const char * streamName);

    virtual double getFractionLost() const;
    virtual void setFractionLost(double fractionLost);

    virtual int getPacketsLostCumulative() const;
    virtual void setPacketsLostCumulative(int packetsLostCumulative);

    virtual int getMaxSequenceNumber() const;
    virtual void setMaxSequenceNumber(int maxSequenceNumber);

    virtual double getJitter() const;
    virtual void setJitter(double jitter);

    virtual omnetpp::simtime_t getLastSR() const;
    virtual void setLastSR(omnetpp::simtime_t lastSR);

    virtual omnetpp::simtime_t getDelaySinceLastSR() const;
    virtual void setDelaySinceLastSR(omnetpp::simtime_t delaySinceLastSR);

    virtual double getFractionRate() const;
    virtual void setFractionRate(double fractionRate);

    virtual double getFractionFecRate() const;
    virtual void setFractionFecRate(double fractionFecRate);

    virtual int getPacketsReceivedCumulative() const;
    virtual void setPacketsReceivedCumulative(int packetsReceivedCumulative);

    virtual int getPacketsOutOfOrderCumulative() const;
    virtual void setPacketsOutOfOrderCumulative(int packetsOutOfOrderCumulative);

    virtual int getPacketsPlayedCumulative() const;
    virtual void setPacketsPlayedCumulative(int packetsPlayedCumulative);

    virtual int getPacketsRetransmittedCumulative() const;
    virtual void setPacketsRetransmittedCumulative(int packetsRetransmittedCumulative);

    virtual int getPacketsRepairedCumulative() const;
    virtual void setPacketsRepairedCumulative(int packetsRepairedCumulative);

    virtual int getPacketsRepairedAndRetransmittedCumulative() const;
    virtual void setPacketsRepairedAndRetransmittedCumulative(int packetsRepairedAndRetransmittedCumulative);

    virtual long getBytesReceivedCumulative() const;
    virtual void setBytesReceivedCumulative(long bytesReceivedCumulative);

    virtual double getFractionInterarrivalDelayMean() const;
    virtual void setFractionInterarrivalDelayMean(double fractionInterarrivalDelayMean);

    virtual double getFractionTransmissionDelayMean() const;
    virtual void setFractionTransmissionDelayMean(double fractionTransmissionDelayMean);

    virtual double getFractionRetransmissionDelayMean() const;
    virtual void setFractionRetransmissionDelayMean(double fractionRetransmissionDelayMean);

    virtual double getFractionPlayoutDelayMean() const;
    virtual void setFractionPlayoutDelayMean(double fractionPlayoutDelayMean);

    virtual double getFractionStallingRate() const;
    virtual void setFractionStallingRate(double fractionStallingRate);

    virtual int getBandwidth() const;
    virtual void setBandwidth(int bandwidth);


  public:
    virtual std::string str() const override;
    virtual void dump(std::ostream& os) const;
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const ReportBlock& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, ReportBlock& obj) {obj.parsimUnpack(b);}


namespace omnetpp {

template<> inline ReportBlock *fromAnyPtr(any_ptr ptr) { return check_and_cast<ReportBlock*>(ptr.get<cObject>()); }

}  // namespace omnetpp

#endif // ifndef __REPORTBLOCK_M_H


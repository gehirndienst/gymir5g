//
// Generated file, do not edit! Created by opp_msgtool 6.0 from apps/ship/messages/SenderReportPacket.msg.
//

#ifndef __SENDERREPORTPACKET_M_H
#define __SENDERREPORTPACKET_M_H

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>

// opp_msgtool version check
#define MSGC_VERSION 0x0600
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of opp_msgtool: 'make clean' should help.
#endif

class SenderReportPacket;
#include "inet/common/INETDefs_m.h" // import inet.common.INETDefs

#include "inet/common/packet/Packet_m.h" // import inet.common.packet.Packet

/**
 * Class generated from <tt>apps/ship/messages/SenderReportPacket.msg:8</tt> by opp_msgtool.
 * <pre>
 * //
 * // an RTCP-like SenderReport (PT 200).
 * // some real parameters are excluded but the header size is kept 
 * //
 * class SenderReportPacket extends inet::FieldsChunk
 * {
 *     chunkLength = inet::B(36); // fixed according to RFC
 * 
 *     string streamName; // ssrc    
 *     int senderPacketCount;
 *     long senderOctetCount;
 *     simtime_t timestamp;
 * 
 *     // auxilary for multihome scenarios
 *     int networkType;
 * }
 * </pre>
 */
class SenderReportPacket : public ::inet::FieldsChunk
{
  protected:
    omnetpp::opp_string streamName;
    int senderPacketCount = 0;
    long senderOctetCount = 0;
    omnetpp::simtime_t timestamp = SIMTIME_ZERO;
    int networkType = 0;

  private:
    void copy(const SenderReportPacket& other);

  protected:
    bool operator==(const SenderReportPacket&) = delete;

  public:
    SenderReportPacket();
    SenderReportPacket(const SenderReportPacket& other);
    virtual ~SenderReportPacket();
    SenderReportPacket& operator=(const SenderReportPacket& other);
    virtual SenderReportPacket *dup() const override {return new SenderReportPacket(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    virtual const char * getStreamName() const;
    virtual void setStreamName(const char * streamName);

    virtual int getSenderPacketCount() const;
    virtual void setSenderPacketCount(int senderPacketCount);

    virtual long getSenderOctetCount() const;
    virtual void setSenderOctetCount(long senderOctetCount);

    virtual omnetpp::simtime_t getTimestamp() const;
    virtual void setTimestamp(omnetpp::simtime_t timestamp);

    virtual int getNetworkType() const;
    virtual void setNetworkType(int networkType);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const SenderReportPacket& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, SenderReportPacket& obj) {obj.parsimUnpack(b);}


namespace omnetpp {

template<> inline SenderReportPacket *fromAnyPtr(any_ptr ptr) { return check_and_cast<SenderReportPacket*>(ptr.get<cObject>()); }

}  // namespace omnetpp

#endif // ifndef __SENDERREPORTPACKET_M_H


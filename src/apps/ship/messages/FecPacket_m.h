//
// Generated file, do not edit! Created by opp_msgtool 6.0 from apps/ship/messages/FecPacket.msg.
//

#ifndef __FECPACKET_M_H
#define __FECPACKET_M_H

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>

// opp_msgtool version check
#define MSGC_VERSION 0x0600
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of opp_msgtool: 'make clean' should help.
#endif

class FecPacket;
#include "inet/common/INETDefs_m.h" // import inet.common.INETDefs

#include "inet/common/packet/chunk/Chunk_m.h" // import inet.common.packet.chunk.Chunk

/**
 * Class generated from <tt>apps/ship/messages/FecPacket.msg:8</tt> by opp_msgtool.
 * <pre>
 * class FecPacket extends inet::FieldsChunk
 * {
 *     chunkLength = inet::B(10);
 * 
 *     string streamName;
 *     int firstSequenceNumber;
 *     int lastSequenceNumber;
 *     int fecCount;
 * 
 *     // auxilary for multihome scenarios
 *     int networkType;
 * }
 * </pre>
 */
class FecPacket : public ::inet::FieldsChunk
{
  protected:
    omnetpp::opp_string streamName;
    int firstSequenceNumber = 0;
    int lastSequenceNumber = 0;
    int fecCount = 0;
    int networkType = 0;

  private:
    void copy(const FecPacket& other);

  protected:
    bool operator==(const FecPacket&) = delete;

  public:
    FecPacket();
    FecPacket(const FecPacket& other);
    virtual ~FecPacket();
    FecPacket& operator=(const FecPacket& other);
    virtual FecPacket *dup() const override {return new FecPacket(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    virtual const char * getStreamName() const;
    virtual void setStreamName(const char * streamName);

    virtual int getFirstSequenceNumber() const;
    virtual void setFirstSequenceNumber(int firstSequenceNumber);

    virtual int getLastSequenceNumber() const;
    virtual void setLastSequenceNumber(int lastSequenceNumber);

    virtual int getFecCount() const;
    virtual void setFecCount(int fecCount);

    virtual int getNetworkType() const;
    virtual void setNetworkType(int networkType);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const FecPacket& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, FecPacket& obj) {obj.parsimUnpack(b);}


namespace omnetpp {

template<> inline FecPacket *fromAnyPtr(any_ptr ptr) { return check_and_cast<FecPacket*>(ptr.get<cObject>()); }

}  // namespace omnetpp

#endif // ifndef __FECPACKET_M_H


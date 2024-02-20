//
// Generated file, do not edit! Created by opp_msgtool 6.0 from apps/shore/messages/TransportFeedbackPacket.msg.
//

#ifndef __TRANSPORTFEEDBACKPACKET_M_H
#define __TRANSPORTFEEDBACKPACKET_M_H

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>

// opp_msgtool version check
#define MSGC_VERSION 0x0600
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of opp_msgtool: 'make clean' should help.
#endif

class TransportFeedbackPacket;
#include "inet/common/INETDefs_m.h" // import inet.common.INETDefs

#include "inet/common/packet/chunk/Chunk_m.h" // import inet.common.packet.chunk.Chunk

#include "TransportFeedbackChunk_m.h" // import TransportFeedbackChunk

/**
 * Class generated from <tt>apps/shore/messages/TransportFeedbackPacket.msg:11</tt> by opp_msgtool.
 * <pre>
 * class TransportFeedbackPacket extends inet::FieldsChunk
 * {
 *     chunkLength = inet::B(18); // + PT 205 RTCP header
 * 
 *     int baseSequenceNumber;
 *     int packetStatusCount;
 *     int feedbackStatusCount;
 * 
 *     cArray feedbackChunks;
 * }
 * </pre>
 */
class TransportFeedbackPacket : public ::inet::FieldsChunk
{
  protected:
    int baseSequenceNumber = 0;
    int packetStatusCount = 0;
    int feedbackStatusCount = 0;
    omnetpp::cArray feedbackChunks;

  private:
    void copy(const TransportFeedbackPacket& other);

  protected:
    bool operator==(const TransportFeedbackPacket&) = delete;

  public:
    TransportFeedbackPacket();
    TransportFeedbackPacket(const TransportFeedbackPacket& other);
    virtual ~TransportFeedbackPacket();
    TransportFeedbackPacket& operator=(const TransportFeedbackPacket& other);
    virtual TransportFeedbackPacket *dup() const override {return new TransportFeedbackPacket(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    virtual int getBaseSequenceNumber() const;
    virtual void setBaseSequenceNumber(int baseSequenceNumber);

    virtual int getPacketStatusCount() const;
    virtual void setPacketStatusCount(int packetStatusCount);

    virtual int getFeedbackStatusCount() const;
    virtual void setFeedbackStatusCount(int feedbackStatusCount);

    virtual const omnetpp::cArray& getFeedbackChunks() const;
    virtual omnetpp::cArray& getFeedbackChunksForUpdate() { handleChange();return const_cast<omnetpp::cArray&>(const_cast<TransportFeedbackPacket*>(this)->getFeedbackChunks());}
    virtual void setFeedbackChunks(const omnetpp::cArray& feedbackChunks);


  public:
    /** adds a chunk (seq number + delta) to this transport feedback */
    virtual void addFeedbackChunk(TransportFeedbackChunk* chunk);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const TransportFeedbackPacket& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, TransportFeedbackPacket& obj) {obj.parsimUnpack(b);}


namespace omnetpp {

template<> inline TransportFeedbackPacket *fromAnyPtr(any_ptr ptr) { return check_and_cast<TransportFeedbackPacket*>(ptr.get<cObject>()); }

}  // namespace omnetpp

#endif // ifndef __TRANSPORTFEEDBACKPACKET_M_H


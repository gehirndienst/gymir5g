//
// Generated file, do not edit! Created by opp_msgtool 6.0 from apps/ship/messages/ShipOffsetMessage.msg.
//

#ifndef __SHIPOFFSETMESSAGE_M_H
#define __SHIPOFFSETMESSAGE_M_H

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>

// opp_msgtool version check
#define MSGC_VERSION 0x0600
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of opp_msgtool: 'make clean' should help.
#endif

class ShipOffsetMessage;
/**
 * Class generated from <tt>apps/ship/messages/ShipOffsetMessage.msg:4</tt> by opp_msgtool.
 * <pre>
 * message ShipOffsetMessage
 * {
 *     string streamName;
 *     int offset;
 * }
 * </pre>
 */
class ShipOffsetMessage : public ::omnetpp::cMessage
{
  protected:
    omnetpp::opp_string streamName;
    int offset = 0;

  private:
    void copy(const ShipOffsetMessage& other);

  protected:
    bool operator==(const ShipOffsetMessage&) = delete;

  public:
    ShipOffsetMessage(const char *name=nullptr, short kind=0);
    ShipOffsetMessage(const ShipOffsetMessage& other);
    virtual ~ShipOffsetMessage();
    ShipOffsetMessage& operator=(const ShipOffsetMessage& other);
    virtual ShipOffsetMessage *dup() const override {return new ShipOffsetMessage(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    virtual const char * getStreamName() const;
    virtual void setStreamName(const char * streamName);

    virtual int getOffset() const;
    virtual void setOffset(int offset);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const ShipOffsetMessage& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, ShipOffsetMessage& obj) {obj.parsimUnpack(b);}


namespace omnetpp {

template<> inline ShipOffsetMessage *fromAnyPtr(any_ptr ptr) { return check_and_cast<ShipOffsetMessage*>(ptr.get<cObject>()); }

}  // namespace omnetpp

#endif // ifndef __SHIPOFFSETMESSAGE_M_H


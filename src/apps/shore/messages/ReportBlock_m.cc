//
// Generated file, do not edit! Created by opp_msgtool 6.0 from apps/shore/messages/ReportBlock.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wshadow"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wc++98-compat"
#  pragma clang diagnostic ignored "-Wunreachable-code-break"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include <iostream>
#include <sstream>
#include <memory>
#include <type_traits>
#include "ReportBlock_m.h"

namespace omnetpp {

// Template pack/unpack rules. They are declared *after* a1l type-specific pack functions for multiple reasons.
// They are in the omnetpp namespace, to allow them to be found by argument-dependent lookup via the cCommBuffer argument

// Packing/unpacking an std::vector
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<T,A>& v)
{
    int n = v.size();
    doParsimPacking(buffer, n);
    for (int i = 0; i < n; i++)
        doParsimPacking(buffer, v[i]);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<T,A>& v)
{
    int n;
    doParsimUnpacking(buffer, n);
    v.resize(n);
    for (int i = 0; i < n; i++)
        doParsimUnpacking(buffer, v[i]);
}

// Packing/unpacking an std::list
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::list<T,A>& l)
{
    doParsimPacking(buffer, (int)l.size());
    for (typename std::list<T,A>::const_iterator it = l.begin(); it != l.end(); ++it)
        doParsimPacking(buffer, (T&)*it);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::list<T,A>& l)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        l.push_back(T());
        doParsimUnpacking(buffer, l.back());
    }
}

// Packing/unpacking an std::set
template<typename T, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::set<T,Tr,A>& s)
{
    doParsimPacking(buffer, (int)s.size());
    for (typename std::set<T,Tr,A>::const_iterator it = s.begin(); it != s.end(); ++it)
        doParsimPacking(buffer, *it);
}

template<typename T, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::set<T,Tr,A>& s)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        T x;
        doParsimUnpacking(buffer, x);
        s.insert(x);
    }
}

// Packing/unpacking an std::map
template<typename K, typename V, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::map<K,V,Tr,A>& m)
{
    doParsimPacking(buffer, (int)m.size());
    for (typename std::map<K,V,Tr,A>::const_iterator it = m.begin(); it != m.end(); ++it) {
        doParsimPacking(buffer, it->first);
        doParsimPacking(buffer, it->second);
    }
}

template<typename K, typename V, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::map<K,V,Tr,A>& m)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        K k; V v;
        doParsimUnpacking(buffer, k);
        doParsimUnpacking(buffer, v);
        m[k] = v;
    }
}

// Default pack/unpack function for arrays
template<typename T>
void doParsimArrayPacking(omnetpp::cCommBuffer *b, const T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimPacking(b, t[i]);
}

template<typename T>
void doParsimArrayUnpacking(omnetpp::cCommBuffer *b, T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimUnpacking(b, t[i]);
}

// Default rule to prevent compiler from choosing base class' doParsimPacking() function
template<typename T>
void doParsimPacking(omnetpp::cCommBuffer *, const T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimPacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

template<typename T>
void doParsimUnpacking(omnetpp::cCommBuffer *, T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimUnpacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

}  // namespace omnetpp

Register_Class(ReportBlock)

ReportBlock::ReportBlock() : ::inet::FieldsChunk()
{
    this->setChunkLength(inet::B(36));

}

ReportBlock::ReportBlock(const ReportBlock& other) : ::inet::FieldsChunk(other)
{
    copy(other);
}

ReportBlock::~ReportBlock()
{
}

ReportBlock& ReportBlock::operator=(const ReportBlock& other)
{
    if (this == &other) return *this;
    ::inet::FieldsChunk::operator=(other);
    copy(other);
    return *this;
}

void ReportBlock::copy(const ReportBlock& other)
{
    this->streamName = other.streamName;
    this->fractionLost = other.fractionLost;
    this->packetsLostCumulative = other.packetsLostCumulative;
    this->maxSequenceNumber = other.maxSequenceNumber;
    this->jitter = other.jitter;
    this->lastSR = other.lastSR;
    this->delaySinceLastSR = other.delaySinceLastSR;
    this->fractionRate = other.fractionRate;
    this->fractionFecRate = other.fractionFecRate;
    this->packetsReceivedCumulative = other.packetsReceivedCumulative;
    this->packetsOutOfOrderCumulative = other.packetsOutOfOrderCumulative;
    this->packetsPlayedCumulative = other.packetsPlayedCumulative;
    this->packetsRetransmittedCumulative = other.packetsRetransmittedCumulative;
    this->packetsRepairedCumulative = other.packetsRepairedCumulative;
    this->packetsRepairedAndRetransmittedCumulative = other.packetsRepairedAndRetransmittedCumulative;
    this->bytesReceivedCumulative = other.bytesReceivedCumulative;
    this->fractionInterarrivalDelayMean = other.fractionInterarrivalDelayMean;
    this->fractionTransmissionDelayMean = other.fractionTransmissionDelayMean;
    this->fractionRetransmissionDelayMean = other.fractionRetransmissionDelayMean;
    this->fractionPlayoutDelayMean = other.fractionPlayoutDelayMean;
    this->fractionStallingRate = other.fractionStallingRate;
    this->bandwidth = other.bandwidth;
}

void ReportBlock::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::inet::FieldsChunk::parsimPack(b);
    doParsimPacking(b,this->streamName);
    doParsimPacking(b,this->fractionLost);
    doParsimPacking(b,this->packetsLostCumulative);
    doParsimPacking(b,this->maxSequenceNumber);
    doParsimPacking(b,this->jitter);
    doParsimPacking(b,this->lastSR);
    doParsimPacking(b,this->delaySinceLastSR);
    doParsimPacking(b,this->fractionRate);
    doParsimPacking(b,this->fractionFecRate);
    doParsimPacking(b,this->packetsReceivedCumulative);
    doParsimPacking(b,this->packetsOutOfOrderCumulative);
    doParsimPacking(b,this->packetsPlayedCumulative);
    doParsimPacking(b,this->packetsRetransmittedCumulative);
    doParsimPacking(b,this->packetsRepairedCumulative);
    doParsimPacking(b,this->packetsRepairedAndRetransmittedCumulative);
    doParsimPacking(b,this->bytesReceivedCumulative);
    doParsimPacking(b,this->fractionInterarrivalDelayMean);
    doParsimPacking(b,this->fractionTransmissionDelayMean);
    doParsimPacking(b,this->fractionRetransmissionDelayMean);
    doParsimPacking(b,this->fractionPlayoutDelayMean);
    doParsimPacking(b,this->fractionStallingRate);
    doParsimPacking(b,this->bandwidth);
}

void ReportBlock::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::inet::FieldsChunk::parsimUnpack(b);
    doParsimUnpacking(b,this->streamName);
    doParsimUnpacking(b,this->fractionLost);
    doParsimUnpacking(b,this->packetsLostCumulative);
    doParsimUnpacking(b,this->maxSequenceNumber);
    doParsimUnpacking(b,this->jitter);
    doParsimUnpacking(b,this->lastSR);
    doParsimUnpacking(b,this->delaySinceLastSR);
    doParsimUnpacking(b,this->fractionRate);
    doParsimUnpacking(b,this->fractionFecRate);
    doParsimUnpacking(b,this->packetsReceivedCumulative);
    doParsimUnpacking(b,this->packetsOutOfOrderCumulative);
    doParsimUnpacking(b,this->packetsPlayedCumulative);
    doParsimUnpacking(b,this->packetsRetransmittedCumulative);
    doParsimUnpacking(b,this->packetsRepairedCumulative);
    doParsimUnpacking(b,this->packetsRepairedAndRetransmittedCumulative);
    doParsimUnpacking(b,this->bytesReceivedCumulative);
    doParsimUnpacking(b,this->fractionInterarrivalDelayMean);
    doParsimUnpacking(b,this->fractionTransmissionDelayMean);
    doParsimUnpacking(b,this->fractionRetransmissionDelayMean);
    doParsimUnpacking(b,this->fractionPlayoutDelayMean);
    doParsimUnpacking(b,this->fractionStallingRate);
    doParsimUnpacking(b,this->bandwidth);
}

const char * ReportBlock::getStreamName() const
{
    return this->streamName.c_str();
}

void ReportBlock::setStreamName(const char * streamName)
{
    handleChange();
    this->streamName = streamName;
}

double ReportBlock::getFractionLost() const
{
    return this->fractionLost;
}

void ReportBlock::setFractionLost(double fractionLost)
{
    handleChange();
    this->fractionLost = fractionLost;
}

int ReportBlock::getPacketsLostCumulative() const
{
    return this->packetsLostCumulative;
}

void ReportBlock::setPacketsLostCumulative(int packetsLostCumulative)
{
    handleChange();
    this->packetsLostCumulative = packetsLostCumulative;
}

int ReportBlock::getMaxSequenceNumber() const
{
    return this->maxSequenceNumber;
}

void ReportBlock::setMaxSequenceNumber(int maxSequenceNumber)
{
    handleChange();
    this->maxSequenceNumber = maxSequenceNumber;
}

double ReportBlock::getJitter() const
{
    return this->jitter;
}

void ReportBlock::setJitter(double jitter)
{
    handleChange();
    this->jitter = jitter;
}

omnetpp::simtime_t ReportBlock::getLastSR() const
{
    return this->lastSR;
}

void ReportBlock::setLastSR(omnetpp::simtime_t lastSR)
{
    handleChange();
    this->lastSR = lastSR;
}

omnetpp::simtime_t ReportBlock::getDelaySinceLastSR() const
{
    return this->delaySinceLastSR;
}

void ReportBlock::setDelaySinceLastSR(omnetpp::simtime_t delaySinceLastSR)
{
    handleChange();
    this->delaySinceLastSR = delaySinceLastSR;
}

double ReportBlock::getFractionRate() const
{
    return this->fractionRate;
}

void ReportBlock::setFractionRate(double fractionRate)
{
    handleChange();
    this->fractionRate = fractionRate;
}

double ReportBlock::getFractionFecRate() const
{
    return this->fractionFecRate;
}

void ReportBlock::setFractionFecRate(double fractionFecRate)
{
    handleChange();
    this->fractionFecRate = fractionFecRate;
}

int ReportBlock::getPacketsReceivedCumulative() const
{
    return this->packetsReceivedCumulative;
}

void ReportBlock::setPacketsReceivedCumulative(int packetsReceivedCumulative)
{
    handleChange();
    this->packetsReceivedCumulative = packetsReceivedCumulative;
}

int ReportBlock::getPacketsOutOfOrderCumulative() const
{
    return this->packetsOutOfOrderCumulative;
}

void ReportBlock::setPacketsOutOfOrderCumulative(int packetsOutOfOrderCumulative)
{
    handleChange();
    this->packetsOutOfOrderCumulative = packetsOutOfOrderCumulative;
}

int ReportBlock::getPacketsPlayedCumulative() const
{
    return this->packetsPlayedCumulative;
}

void ReportBlock::setPacketsPlayedCumulative(int packetsPlayedCumulative)
{
    handleChange();
    this->packetsPlayedCumulative = packetsPlayedCumulative;
}

int ReportBlock::getPacketsRetransmittedCumulative() const
{
    return this->packetsRetransmittedCumulative;
}

void ReportBlock::setPacketsRetransmittedCumulative(int packetsRetransmittedCumulative)
{
    handleChange();
    this->packetsRetransmittedCumulative = packetsRetransmittedCumulative;
}

int ReportBlock::getPacketsRepairedCumulative() const
{
    return this->packetsRepairedCumulative;
}

void ReportBlock::setPacketsRepairedCumulative(int packetsRepairedCumulative)
{
    handleChange();
    this->packetsRepairedCumulative = packetsRepairedCumulative;
}

int ReportBlock::getPacketsRepairedAndRetransmittedCumulative() const
{
    return this->packetsRepairedAndRetransmittedCumulative;
}

void ReportBlock::setPacketsRepairedAndRetransmittedCumulative(int packetsRepairedAndRetransmittedCumulative)
{
    handleChange();
    this->packetsRepairedAndRetransmittedCumulative = packetsRepairedAndRetransmittedCumulative;
}

long ReportBlock::getBytesReceivedCumulative() const
{
    return this->bytesReceivedCumulative;
}

void ReportBlock::setBytesReceivedCumulative(long bytesReceivedCumulative)
{
    handleChange();
    this->bytesReceivedCumulative = bytesReceivedCumulative;
}

double ReportBlock::getFractionInterarrivalDelayMean() const
{
    return this->fractionInterarrivalDelayMean;
}

void ReportBlock::setFractionInterarrivalDelayMean(double fractionInterarrivalDelayMean)
{
    handleChange();
    this->fractionInterarrivalDelayMean = fractionInterarrivalDelayMean;
}

double ReportBlock::getFractionTransmissionDelayMean() const
{
    return this->fractionTransmissionDelayMean;
}

void ReportBlock::setFractionTransmissionDelayMean(double fractionTransmissionDelayMean)
{
    handleChange();
    this->fractionTransmissionDelayMean = fractionTransmissionDelayMean;
}

double ReportBlock::getFractionRetransmissionDelayMean() const
{
    return this->fractionRetransmissionDelayMean;
}

void ReportBlock::setFractionRetransmissionDelayMean(double fractionRetransmissionDelayMean)
{
    handleChange();
    this->fractionRetransmissionDelayMean = fractionRetransmissionDelayMean;
}

double ReportBlock::getFractionPlayoutDelayMean() const
{
    return this->fractionPlayoutDelayMean;
}

void ReportBlock::setFractionPlayoutDelayMean(double fractionPlayoutDelayMean)
{
    handleChange();
    this->fractionPlayoutDelayMean = fractionPlayoutDelayMean;
}

double ReportBlock::getFractionStallingRate() const
{
    return this->fractionStallingRate;
}

void ReportBlock::setFractionStallingRate(double fractionStallingRate)
{
    handleChange();
    this->fractionStallingRate = fractionStallingRate;
}

int ReportBlock::getBandwidth() const
{
    return this->bandwidth;
}

void ReportBlock::setBandwidth(int bandwidth)
{
    handleChange();
    this->bandwidth = bandwidth;
}

class ReportBlockDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_streamName,
        FIELD_fractionLost,
        FIELD_packetsLostCumulative,
        FIELD_maxSequenceNumber,
        FIELD_jitter,
        FIELD_lastSR,
        FIELD_delaySinceLastSR,
        FIELD_fractionRate,
        FIELD_fractionFecRate,
        FIELD_packetsReceivedCumulative,
        FIELD_packetsOutOfOrderCumulative,
        FIELD_packetsPlayedCumulative,
        FIELD_packetsRetransmittedCumulative,
        FIELD_packetsRepairedCumulative,
        FIELD_packetsRepairedAndRetransmittedCumulative,
        FIELD_bytesReceivedCumulative,
        FIELD_fractionInterarrivalDelayMean,
        FIELD_fractionTransmissionDelayMean,
        FIELD_fractionRetransmissionDelayMean,
        FIELD_fractionPlayoutDelayMean,
        FIELD_fractionStallingRate,
        FIELD_bandwidth,
    };
  public:
    ReportBlockDescriptor();
    virtual ~ReportBlockDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(ReportBlockDescriptor)

ReportBlockDescriptor::ReportBlockDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(ReportBlock)), "inet::FieldsChunk")
{
    propertyNames = nullptr;
}

ReportBlockDescriptor::~ReportBlockDescriptor()
{
    delete[] propertyNames;
}

bool ReportBlockDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<ReportBlock *>(obj)!=nullptr;
}

const char **ReportBlockDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *ReportBlockDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int ReportBlockDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 22+base->getFieldCount() : 22;
}

unsigned int ReportBlockDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_streamName
        FD_ISEDITABLE,    // FIELD_fractionLost
        FD_ISEDITABLE,    // FIELD_packetsLostCumulative
        FD_ISEDITABLE,    // FIELD_maxSequenceNumber
        FD_ISEDITABLE,    // FIELD_jitter
        FD_ISEDITABLE,    // FIELD_lastSR
        FD_ISEDITABLE,    // FIELD_delaySinceLastSR
        FD_ISEDITABLE,    // FIELD_fractionRate
        FD_ISEDITABLE,    // FIELD_fractionFecRate
        FD_ISEDITABLE,    // FIELD_packetsReceivedCumulative
        FD_ISEDITABLE,    // FIELD_packetsOutOfOrderCumulative
        FD_ISEDITABLE,    // FIELD_packetsPlayedCumulative
        FD_ISEDITABLE,    // FIELD_packetsRetransmittedCumulative
        FD_ISEDITABLE,    // FIELD_packetsRepairedCumulative
        FD_ISEDITABLE,    // FIELD_packetsRepairedAndRetransmittedCumulative
        FD_ISEDITABLE,    // FIELD_bytesReceivedCumulative
        FD_ISEDITABLE,    // FIELD_fractionInterarrivalDelayMean
        FD_ISEDITABLE,    // FIELD_fractionTransmissionDelayMean
        FD_ISEDITABLE,    // FIELD_fractionRetransmissionDelayMean
        FD_ISEDITABLE,    // FIELD_fractionPlayoutDelayMean
        FD_ISEDITABLE,    // FIELD_fractionStallingRate
        FD_ISEDITABLE,    // FIELD_bandwidth
    };
    return (field >= 0 && field < 22) ? fieldTypeFlags[field] : 0;
}

const char *ReportBlockDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "streamName",
        "fractionLost",
        "packetsLostCumulative",
        "maxSequenceNumber",
        "jitter",
        "lastSR",
        "delaySinceLastSR",
        "fractionRate",
        "fractionFecRate",
        "packetsReceivedCumulative",
        "packetsOutOfOrderCumulative",
        "packetsPlayedCumulative",
        "packetsRetransmittedCumulative",
        "packetsRepairedCumulative",
        "packetsRepairedAndRetransmittedCumulative",
        "bytesReceivedCumulative",
        "fractionInterarrivalDelayMean",
        "fractionTransmissionDelayMean",
        "fractionRetransmissionDelayMean",
        "fractionPlayoutDelayMean",
        "fractionStallingRate",
        "bandwidth",
    };
    return (field >= 0 && field < 22) ? fieldNames[field] : nullptr;
}

int ReportBlockDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "streamName") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "fractionLost") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "packetsLostCumulative") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "maxSequenceNumber") == 0) return baseIndex + 3;
    if (strcmp(fieldName, "jitter") == 0) return baseIndex + 4;
    if (strcmp(fieldName, "lastSR") == 0) return baseIndex + 5;
    if (strcmp(fieldName, "delaySinceLastSR") == 0) return baseIndex + 6;
    if (strcmp(fieldName, "fractionRate") == 0) return baseIndex + 7;
    if (strcmp(fieldName, "fractionFecRate") == 0) return baseIndex + 8;
    if (strcmp(fieldName, "packetsReceivedCumulative") == 0) return baseIndex + 9;
    if (strcmp(fieldName, "packetsOutOfOrderCumulative") == 0) return baseIndex + 10;
    if (strcmp(fieldName, "packetsPlayedCumulative") == 0) return baseIndex + 11;
    if (strcmp(fieldName, "packetsRetransmittedCumulative") == 0) return baseIndex + 12;
    if (strcmp(fieldName, "packetsRepairedCumulative") == 0) return baseIndex + 13;
    if (strcmp(fieldName, "packetsRepairedAndRetransmittedCumulative") == 0) return baseIndex + 14;
    if (strcmp(fieldName, "bytesReceivedCumulative") == 0) return baseIndex + 15;
    if (strcmp(fieldName, "fractionInterarrivalDelayMean") == 0) return baseIndex + 16;
    if (strcmp(fieldName, "fractionTransmissionDelayMean") == 0) return baseIndex + 17;
    if (strcmp(fieldName, "fractionRetransmissionDelayMean") == 0) return baseIndex + 18;
    if (strcmp(fieldName, "fractionPlayoutDelayMean") == 0) return baseIndex + 19;
    if (strcmp(fieldName, "fractionStallingRate") == 0) return baseIndex + 20;
    if (strcmp(fieldName, "bandwidth") == 0) return baseIndex + 21;
    return base ? base->findField(fieldName) : -1;
}

const char *ReportBlockDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "string",    // FIELD_streamName
        "double",    // FIELD_fractionLost
        "int",    // FIELD_packetsLostCumulative
        "int",    // FIELD_maxSequenceNumber
        "double",    // FIELD_jitter
        "omnetpp::simtime_t",    // FIELD_lastSR
        "omnetpp::simtime_t",    // FIELD_delaySinceLastSR
        "double",    // FIELD_fractionRate
        "double",    // FIELD_fractionFecRate
        "int",    // FIELD_packetsReceivedCumulative
        "int",    // FIELD_packetsOutOfOrderCumulative
        "int",    // FIELD_packetsPlayedCumulative
        "int",    // FIELD_packetsRetransmittedCumulative
        "int",    // FIELD_packetsRepairedCumulative
        "int",    // FIELD_packetsRepairedAndRetransmittedCumulative
        "long",    // FIELD_bytesReceivedCumulative
        "double",    // FIELD_fractionInterarrivalDelayMean
        "double",    // FIELD_fractionTransmissionDelayMean
        "double",    // FIELD_fractionRetransmissionDelayMean
        "double",    // FIELD_fractionPlayoutDelayMean
        "double",    // FIELD_fractionStallingRate
        "int",    // FIELD_bandwidth
    };
    return (field >= 0 && field < 22) ? fieldTypeStrings[field] : nullptr;
}

const char **ReportBlockDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *ReportBlockDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int ReportBlockDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    ReportBlock *pp = omnetpp::fromAnyPtr<ReportBlock>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void ReportBlockDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    ReportBlock *pp = omnetpp::fromAnyPtr<ReportBlock>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'ReportBlock'", field);
    }
}

const char *ReportBlockDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    ReportBlock *pp = omnetpp::fromAnyPtr<ReportBlock>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string ReportBlockDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    ReportBlock *pp = omnetpp::fromAnyPtr<ReportBlock>(object); (void)pp;
    switch (field) {
        case FIELD_streamName: return oppstring2string(pp->getStreamName());
        case FIELD_fractionLost: return double2string(pp->getFractionLost());
        case FIELD_packetsLostCumulative: return long2string(pp->getPacketsLostCumulative());
        case FIELD_maxSequenceNumber: return long2string(pp->getMaxSequenceNumber());
        case FIELD_jitter: return double2string(pp->getJitter());
        case FIELD_lastSR: return simtime2string(pp->getLastSR());
        case FIELD_delaySinceLastSR: return simtime2string(pp->getDelaySinceLastSR());
        case FIELD_fractionRate: return double2string(pp->getFractionRate());
        case FIELD_fractionFecRate: return double2string(pp->getFractionFecRate());
        case FIELD_packetsReceivedCumulative: return long2string(pp->getPacketsReceivedCumulative());
        case FIELD_packetsOutOfOrderCumulative: return long2string(pp->getPacketsOutOfOrderCumulative());
        case FIELD_packetsPlayedCumulative: return long2string(pp->getPacketsPlayedCumulative());
        case FIELD_packetsRetransmittedCumulative: return long2string(pp->getPacketsRetransmittedCumulative());
        case FIELD_packetsRepairedCumulative: return long2string(pp->getPacketsRepairedCumulative());
        case FIELD_packetsRepairedAndRetransmittedCumulative: return long2string(pp->getPacketsRepairedAndRetransmittedCumulative());
        case FIELD_bytesReceivedCumulative: return long2string(pp->getBytesReceivedCumulative());
        case FIELD_fractionInterarrivalDelayMean: return double2string(pp->getFractionInterarrivalDelayMean());
        case FIELD_fractionTransmissionDelayMean: return double2string(pp->getFractionTransmissionDelayMean());
        case FIELD_fractionRetransmissionDelayMean: return double2string(pp->getFractionRetransmissionDelayMean());
        case FIELD_fractionPlayoutDelayMean: return double2string(pp->getFractionPlayoutDelayMean());
        case FIELD_fractionStallingRate: return double2string(pp->getFractionStallingRate());
        case FIELD_bandwidth: return long2string(pp->getBandwidth());
        default: return "";
    }
}

void ReportBlockDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    ReportBlock *pp = omnetpp::fromAnyPtr<ReportBlock>(object); (void)pp;
    switch (field) {
        case FIELD_streamName: pp->setStreamName((value)); break;
        case FIELD_fractionLost: pp->setFractionLost(string2double(value)); break;
        case FIELD_packetsLostCumulative: pp->setPacketsLostCumulative(string2long(value)); break;
        case FIELD_maxSequenceNumber: pp->setMaxSequenceNumber(string2long(value)); break;
        case FIELD_jitter: pp->setJitter(string2double(value)); break;
        case FIELD_lastSR: pp->setLastSR(string2simtime(value)); break;
        case FIELD_delaySinceLastSR: pp->setDelaySinceLastSR(string2simtime(value)); break;
        case FIELD_fractionRate: pp->setFractionRate(string2double(value)); break;
        case FIELD_fractionFecRate: pp->setFractionFecRate(string2double(value)); break;
        case FIELD_packetsReceivedCumulative: pp->setPacketsReceivedCumulative(string2long(value)); break;
        case FIELD_packetsOutOfOrderCumulative: pp->setPacketsOutOfOrderCumulative(string2long(value)); break;
        case FIELD_packetsPlayedCumulative: pp->setPacketsPlayedCumulative(string2long(value)); break;
        case FIELD_packetsRetransmittedCumulative: pp->setPacketsRetransmittedCumulative(string2long(value)); break;
        case FIELD_packetsRepairedCumulative: pp->setPacketsRepairedCumulative(string2long(value)); break;
        case FIELD_packetsRepairedAndRetransmittedCumulative: pp->setPacketsRepairedAndRetransmittedCumulative(string2long(value)); break;
        case FIELD_bytesReceivedCumulative: pp->setBytesReceivedCumulative(string2long(value)); break;
        case FIELD_fractionInterarrivalDelayMean: pp->setFractionInterarrivalDelayMean(string2double(value)); break;
        case FIELD_fractionTransmissionDelayMean: pp->setFractionTransmissionDelayMean(string2double(value)); break;
        case FIELD_fractionRetransmissionDelayMean: pp->setFractionRetransmissionDelayMean(string2double(value)); break;
        case FIELD_fractionPlayoutDelayMean: pp->setFractionPlayoutDelayMean(string2double(value)); break;
        case FIELD_fractionStallingRate: pp->setFractionStallingRate(string2double(value)); break;
        case FIELD_bandwidth: pp->setBandwidth(string2long(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'ReportBlock'", field);
    }
}

omnetpp::cValue ReportBlockDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    ReportBlock *pp = omnetpp::fromAnyPtr<ReportBlock>(object); (void)pp;
    switch (field) {
        case FIELD_streamName: return pp->getStreamName();
        case FIELD_fractionLost: return pp->getFractionLost();
        case FIELD_packetsLostCumulative: return pp->getPacketsLostCumulative();
        case FIELD_maxSequenceNumber: return pp->getMaxSequenceNumber();
        case FIELD_jitter: return pp->getJitter();
        case FIELD_lastSR: return pp->getLastSR().dbl();
        case FIELD_delaySinceLastSR: return pp->getDelaySinceLastSR().dbl();
        case FIELD_fractionRate: return pp->getFractionRate();
        case FIELD_fractionFecRate: return pp->getFractionFecRate();
        case FIELD_packetsReceivedCumulative: return pp->getPacketsReceivedCumulative();
        case FIELD_packetsOutOfOrderCumulative: return pp->getPacketsOutOfOrderCumulative();
        case FIELD_packetsPlayedCumulative: return pp->getPacketsPlayedCumulative();
        case FIELD_packetsRetransmittedCumulative: return pp->getPacketsRetransmittedCumulative();
        case FIELD_packetsRepairedCumulative: return pp->getPacketsRepairedCumulative();
        case FIELD_packetsRepairedAndRetransmittedCumulative: return pp->getPacketsRepairedAndRetransmittedCumulative();
        case FIELD_bytesReceivedCumulative: return (omnetpp::intval_t)(pp->getBytesReceivedCumulative());
        case FIELD_fractionInterarrivalDelayMean: return pp->getFractionInterarrivalDelayMean();
        case FIELD_fractionTransmissionDelayMean: return pp->getFractionTransmissionDelayMean();
        case FIELD_fractionRetransmissionDelayMean: return pp->getFractionRetransmissionDelayMean();
        case FIELD_fractionPlayoutDelayMean: return pp->getFractionPlayoutDelayMean();
        case FIELD_fractionStallingRate: return pp->getFractionStallingRate();
        case FIELD_bandwidth: return pp->getBandwidth();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'ReportBlock' as cValue -- field index out of range?", field);
    }
}

void ReportBlockDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    ReportBlock *pp = omnetpp::fromAnyPtr<ReportBlock>(object); (void)pp;
    switch (field) {
        case FIELD_streamName: pp->setStreamName(value.stringValue()); break;
        case FIELD_fractionLost: pp->setFractionLost(value.doubleValue()); break;
        case FIELD_packetsLostCumulative: pp->setPacketsLostCumulative(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_maxSequenceNumber: pp->setMaxSequenceNumber(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_jitter: pp->setJitter(value.doubleValue()); break;
        case FIELD_lastSR: pp->setLastSR(value.doubleValue()); break;
        case FIELD_delaySinceLastSR: pp->setDelaySinceLastSR(value.doubleValue()); break;
        case FIELD_fractionRate: pp->setFractionRate(value.doubleValue()); break;
        case FIELD_fractionFecRate: pp->setFractionFecRate(value.doubleValue()); break;
        case FIELD_packetsReceivedCumulative: pp->setPacketsReceivedCumulative(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_packetsOutOfOrderCumulative: pp->setPacketsOutOfOrderCumulative(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_packetsPlayedCumulative: pp->setPacketsPlayedCumulative(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_packetsRetransmittedCumulative: pp->setPacketsRetransmittedCumulative(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_packetsRepairedCumulative: pp->setPacketsRepairedCumulative(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_packetsRepairedAndRetransmittedCumulative: pp->setPacketsRepairedAndRetransmittedCumulative(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_bytesReceivedCumulative: pp->setBytesReceivedCumulative(omnetpp::checked_int_cast<long>(value.intValue())); break;
        case FIELD_fractionInterarrivalDelayMean: pp->setFractionInterarrivalDelayMean(value.doubleValue()); break;
        case FIELD_fractionTransmissionDelayMean: pp->setFractionTransmissionDelayMean(value.doubleValue()); break;
        case FIELD_fractionRetransmissionDelayMean: pp->setFractionRetransmissionDelayMean(value.doubleValue()); break;
        case FIELD_fractionPlayoutDelayMean: pp->setFractionPlayoutDelayMean(value.doubleValue()); break;
        case FIELD_fractionStallingRate: pp->setFractionStallingRate(value.doubleValue()); break;
        case FIELD_bandwidth: pp->setBandwidth(omnetpp::checked_int_cast<int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'ReportBlock'", field);
    }
}

const char *ReportBlockDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr ReportBlockDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    ReportBlock *pp = omnetpp::fromAnyPtr<ReportBlock>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void ReportBlockDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    ReportBlock *pp = omnetpp::fromAnyPtr<ReportBlock>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'ReportBlock'", field);
    }
}

namespace omnetpp {

}  // namespace omnetpp


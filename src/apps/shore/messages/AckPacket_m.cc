//
// Generated file, do not edit! Created by opp_msgtool 6.0 from apps/shore/messages/AckPacket.msg.
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
#include "AckPacket_m.h"

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

Register_Class(AckPacket)

AckPacket::AckPacket() : ::inet::FieldsChunk()
{
    this->setChunkLength(inet::B(1));

}

AckPacket::AckPacket(const AckPacket& other) : ::inet::FieldsChunk(other)
{
    copy(other);
}

AckPacket::~AckPacket()
{
}

AckPacket& AckPacket::operator=(const AckPacket& other)
{
    if (this == &other) return *this;
    ::inet::FieldsChunk::operator=(other);
    copy(other);
    return *this;
}

void AckPacket::copy(const AckPacket& other)
{
    this->sequenceNumber = other.sequenceNumber;
    this->streamName = other.streamName;
    this->payloadSize = other.payloadSize;
    this->senderTimestamp = other.senderTimestamp;
    this->arrivalTimestamp = other.arrivalTimestamp;
}

void AckPacket::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::inet::FieldsChunk::parsimPack(b);
    doParsimPacking(b,this->sequenceNumber);
    doParsimPacking(b,this->streamName);
    doParsimPacking(b,this->payloadSize);
    doParsimPacking(b,this->senderTimestamp);
    doParsimPacking(b,this->arrivalTimestamp);
}

void AckPacket::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::inet::FieldsChunk::parsimUnpack(b);
    doParsimUnpacking(b,this->sequenceNumber);
    doParsimUnpacking(b,this->streamName);
    doParsimUnpacking(b,this->payloadSize);
    doParsimUnpacking(b,this->senderTimestamp);
    doParsimUnpacking(b,this->arrivalTimestamp);
}

int AckPacket::getSequenceNumber() const
{
    return this->sequenceNumber;
}

void AckPacket::setSequenceNumber(int sequenceNumber)
{
    handleChange();
    this->sequenceNumber = sequenceNumber;
}

const char * AckPacket::getStreamName() const
{
    return this->streamName.c_str();
}

void AckPacket::setStreamName(const char * streamName)
{
    handleChange();
    this->streamName = streamName;
}

int AckPacket::getPayloadSize() const
{
    return this->payloadSize;
}

void AckPacket::setPayloadSize(int payloadSize)
{
    handleChange();
    this->payloadSize = payloadSize;
}

omnetpp::simtime_t AckPacket::getSenderTimestamp() const
{
    return this->senderTimestamp;
}

void AckPacket::setSenderTimestamp(omnetpp::simtime_t senderTimestamp)
{
    handleChange();
    this->senderTimestamp = senderTimestamp;
}

omnetpp::simtime_t AckPacket::getArrivalTimestamp() const
{
    return this->arrivalTimestamp;
}

void AckPacket::setArrivalTimestamp(omnetpp::simtime_t arrivalTimestamp)
{
    handleChange();
    this->arrivalTimestamp = arrivalTimestamp;
}

class AckPacketDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_sequenceNumber,
        FIELD_streamName,
        FIELD_payloadSize,
        FIELD_senderTimestamp,
        FIELD_arrivalTimestamp,
    };
  public:
    AckPacketDescriptor();
    virtual ~AckPacketDescriptor();

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

Register_ClassDescriptor(AckPacketDescriptor)

AckPacketDescriptor::AckPacketDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(AckPacket)), "inet::FieldsChunk")
{
    propertyNames = nullptr;
}

AckPacketDescriptor::~AckPacketDescriptor()
{
    delete[] propertyNames;
}

bool AckPacketDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<AckPacket *>(obj)!=nullptr;
}

const char **AckPacketDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *AckPacketDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int AckPacketDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 5+base->getFieldCount() : 5;
}

unsigned int AckPacketDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_sequenceNumber
        FD_ISEDITABLE,    // FIELD_streamName
        FD_ISEDITABLE,    // FIELD_payloadSize
        FD_ISEDITABLE,    // FIELD_senderTimestamp
        FD_ISEDITABLE,    // FIELD_arrivalTimestamp
    };
    return (field >= 0 && field < 5) ? fieldTypeFlags[field] : 0;
}

const char *AckPacketDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "sequenceNumber",
        "streamName",
        "payloadSize",
        "senderTimestamp",
        "arrivalTimestamp",
    };
    return (field >= 0 && field < 5) ? fieldNames[field] : nullptr;
}

int AckPacketDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "sequenceNumber") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "streamName") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "payloadSize") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "senderTimestamp") == 0) return baseIndex + 3;
    if (strcmp(fieldName, "arrivalTimestamp") == 0) return baseIndex + 4;
    return base ? base->findField(fieldName) : -1;
}

const char *AckPacketDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",    // FIELD_sequenceNumber
        "string",    // FIELD_streamName
        "int",    // FIELD_payloadSize
        "omnetpp::simtime_t",    // FIELD_senderTimestamp
        "omnetpp::simtime_t",    // FIELD_arrivalTimestamp
    };
    return (field >= 0 && field < 5) ? fieldTypeStrings[field] : nullptr;
}

const char **AckPacketDescriptor::getFieldPropertyNames(int field) const
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

const char *AckPacketDescriptor::getFieldProperty(int field, const char *propertyName) const
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

int AckPacketDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    AckPacket *pp = omnetpp::fromAnyPtr<AckPacket>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void AckPacketDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    AckPacket *pp = omnetpp::fromAnyPtr<AckPacket>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'AckPacket'", field);
    }
}

const char *AckPacketDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    AckPacket *pp = omnetpp::fromAnyPtr<AckPacket>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string AckPacketDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    AckPacket *pp = omnetpp::fromAnyPtr<AckPacket>(object); (void)pp;
    switch (field) {
        case FIELD_sequenceNumber: return long2string(pp->getSequenceNumber());
        case FIELD_streamName: return oppstring2string(pp->getStreamName());
        case FIELD_payloadSize: return long2string(pp->getPayloadSize());
        case FIELD_senderTimestamp: return simtime2string(pp->getSenderTimestamp());
        case FIELD_arrivalTimestamp: return simtime2string(pp->getArrivalTimestamp());
        default: return "";
    }
}

void AckPacketDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    AckPacket *pp = omnetpp::fromAnyPtr<AckPacket>(object); (void)pp;
    switch (field) {
        case FIELD_sequenceNumber: pp->setSequenceNumber(string2long(value)); break;
        case FIELD_streamName: pp->setStreamName((value)); break;
        case FIELD_payloadSize: pp->setPayloadSize(string2long(value)); break;
        case FIELD_senderTimestamp: pp->setSenderTimestamp(string2simtime(value)); break;
        case FIELD_arrivalTimestamp: pp->setArrivalTimestamp(string2simtime(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'AckPacket'", field);
    }
}

omnetpp::cValue AckPacketDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    AckPacket *pp = omnetpp::fromAnyPtr<AckPacket>(object); (void)pp;
    switch (field) {
        case FIELD_sequenceNumber: return pp->getSequenceNumber();
        case FIELD_streamName: return pp->getStreamName();
        case FIELD_payloadSize: return pp->getPayloadSize();
        case FIELD_senderTimestamp: return pp->getSenderTimestamp().dbl();
        case FIELD_arrivalTimestamp: return pp->getArrivalTimestamp().dbl();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'AckPacket' as cValue -- field index out of range?", field);
    }
}

void AckPacketDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    AckPacket *pp = omnetpp::fromAnyPtr<AckPacket>(object); (void)pp;
    switch (field) {
        case FIELD_sequenceNumber: pp->setSequenceNumber(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_streamName: pp->setStreamName(value.stringValue()); break;
        case FIELD_payloadSize: pp->setPayloadSize(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_senderTimestamp: pp->setSenderTimestamp(value.doubleValue()); break;
        case FIELD_arrivalTimestamp: pp->setArrivalTimestamp(value.doubleValue()); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'AckPacket'", field);
    }
}

const char *AckPacketDescriptor::getFieldStructName(int field) const
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

omnetpp::any_ptr AckPacketDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    AckPacket *pp = omnetpp::fromAnyPtr<AckPacket>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void AckPacketDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    AckPacket *pp = omnetpp::fromAnyPtr<AckPacket>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'AckPacket'", field);
    }
}

namespace omnetpp {

}  // namespace omnetpp


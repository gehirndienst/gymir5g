//
// Generated file, do not edit! Created by opp_msgtool 6.0 from apps/ship/messages/ShipPacket.msg.
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
#include "ShipPacket_m.h"

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

Register_Class(ShipPacket)

ShipPacket::ShipPacket() : ::inet::FieldsChunk()
{
    this->setChunkLength(inet::B(14));

}

ShipPacket::ShipPacket(const ShipPacket& other) : ::inet::FieldsChunk(other)
{
    copy(other);
}

ShipPacket::~ShipPacket()
{
}

ShipPacket& ShipPacket::operator=(const ShipPacket& other)
{
    if (this == &other) return *this;
    ::inet::FieldsChunk::operator=(other);
    copy(other);
    return *this;
}

void ShipPacket::copy(const ShipPacket& other)
{
    this->transportWideSequenceNumber = other.transportWideSequenceNumber;
    this->sequenceNumber = other.sequenceNumber;
    this->streamName = other.streamName;
    this->streamType = other.streamType;
    this->elemNumber = other.elemNumber;
    this->fragmentOffset = other.fragmentOffset;
    this->isLastFragment_ = other.isLastFragment_;
    this->payloadSize = other.payloadSize;
    this->payloadTimestamp = other.payloadTimestamp;
    this->isMarked_ = other.isMarked_;
    this->networkType = other.networkType;
}

void ShipPacket::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::inet::FieldsChunk::parsimPack(b);
    doParsimPacking(b,this->transportWideSequenceNumber);
    doParsimPacking(b,this->sequenceNumber);
    doParsimPacking(b,this->streamName);
    doParsimPacking(b,this->streamType);
    doParsimPacking(b,this->elemNumber);
    doParsimPacking(b,this->fragmentOffset);
    doParsimPacking(b,this->isLastFragment_);
    doParsimPacking(b,this->payloadSize);
    doParsimPacking(b,this->payloadTimestamp);
    doParsimPacking(b,this->isMarked_);
    doParsimPacking(b,this->networkType);
}

void ShipPacket::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::inet::FieldsChunk::parsimUnpack(b);
    doParsimUnpacking(b,this->transportWideSequenceNumber);
    doParsimUnpacking(b,this->sequenceNumber);
    doParsimUnpacking(b,this->streamName);
    doParsimUnpacking(b,this->streamType);
    doParsimUnpacking(b,this->elemNumber);
    doParsimUnpacking(b,this->fragmentOffset);
    doParsimUnpacking(b,this->isLastFragment_);
    doParsimUnpacking(b,this->payloadSize);
    doParsimUnpacking(b,this->payloadTimestamp);
    doParsimUnpacking(b,this->isMarked_);
    doParsimUnpacking(b,this->networkType);
}

int ShipPacket::getTransportWideSequenceNumber() const
{
    return this->transportWideSequenceNumber;
}

void ShipPacket::setTransportWideSequenceNumber(int transportWideSequenceNumber)
{
    handleChange();
    this->transportWideSequenceNumber = transportWideSequenceNumber;
}

int ShipPacket::getSequenceNumber() const
{
    return this->sequenceNumber;
}

void ShipPacket::setSequenceNumber(int sequenceNumber)
{
    handleChange();
    this->sequenceNumber = sequenceNumber;
}

const char * ShipPacket::getStreamName() const
{
    return this->streamName.c_str();
}

void ShipPacket::setStreamName(const char * streamName)
{
    handleChange();
    this->streamName = streamName;
}

const char * ShipPacket::getStreamType() const
{
    return this->streamType.c_str();
}

void ShipPacket::setStreamType(const char * streamType)
{
    handleChange();
    this->streamType = streamType;
}

int ShipPacket::getElemNumber() const
{
    return this->elemNumber;
}

void ShipPacket::setElemNumber(int elemNumber)
{
    handleChange();
    this->elemNumber = elemNumber;
}

int ShipPacket::getFragmentOffset() const
{
    return this->fragmentOffset;
}

void ShipPacket::setFragmentOffset(int fragmentOffset)
{
    handleChange();
    this->fragmentOffset = fragmentOffset;
}

bool ShipPacket::isLastFragment() const
{
    return this->isLastFragment_;
}

void ShipPacket::setIsLastFragment(bool isLastFragment)
{
    handleChange();
    this->isLastFragment_ = isLastFragment;
}

int ShipPacket::getPayloadSize() const
{
    return this->payloadSize;
}

void ShipPacket::setPayloadSize(int payloadSize)
{
    handleChange();
    this->payloadSize = payloadSize;
}

omnetpp::simtime_t ShipPacket::getPayloadTimestamp() const
{
    return this->payloadTimestamp;
}

void ShipPacket::setPayloadTimestamp(omnetpp::simtime_t payloadTimestamp)
{
    handleChange();
    this->payloadTimestamp = payloadTimestamp;
}

bool ShipPacket::isMarked() const
{
    return this->isMarked_;
}

void ShipPacket::setIsMarked(bool isMarked)
{
    handleChange();
    this->isMarked_ = isMarked;
}

int ShipPacket::getNetworkType() const
{
    return this->networkType;
}

void ShipPacket::setNetworkType(int networkType)
{
    handleChange();
    this->networkType = networkType;
}

class ShipPacketDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_transportWideSequenceNumber,
        FIELD_sequenceNumber,
        FIELD_streamName,
        FIELD_streamType,
        FIELD_elemNumber,
        FIELD_fragmentOffset,
        FIELD_isLastFragment,
        FIELD_payloadSize,
        FIELD_payloadTimestamp,
        FIELD_isMarked,
        FIELD_networkType,
    };
  public:
    ShipPacketDescriptor();
    virtual ~ShipPacketDescriptor();

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

Register_ClassDescriptor(ShipPacketDescriptor)

ShipPacketDescriptor::ShipPacketDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(ShipPacket)), "inet::FieldsChunk")
{
    propertyNames = nullptr;
}

ShipPacketDescriptor::~ShipPacketDescriptor()
{
    delete[] propertyNames;
}

bool ShipPacketDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<ShipPacket *>(obj)!=nullptr;
}

const char **ShipPacketDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *ShipPacketDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int ShipPacketDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 11+base->getFieldCount() : 11;
}

unsigned int ShipPacketDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_transportWideSequenceNumber
        FD_ISEDITABLE,    // FIELD_sequenceNumber
        FD_ISEDITABLE,    // FIELD_streamName
        FD_ISEDITABLE,    // FIELD_streamType
        FD_ISEDITABLE,    // FIELD_elemNumber
        FD_ISEDITABLE,    // FIELD_fragmentOffset
        FD_ISEDITABLE,    // FIELD_isLastFragment
        FD_ISEDITABLE,    // FIELD_payloadSize
        FD_ISEDITABLE,    // FIELD_payloadTimestamp
        FD_ISEDITABLE,    // FIELD_isMarked
        FD_ISEDITABLE,    // FIELD_networkType
    };
    return (field >= 0 && field < 11) ? fieldTypeFlags[field] : 0;
}

const char *ShipPacketDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "transportWideSequenceNumber",
        "sequenceNumber",
        "streamName",
        "streamType",
        "elemNumber",
        "fragmentOffset",
        "isLastFragment",
        "payloadSize",
        "payloadTimestamp",
        "isMarked",
        "networkType",
    };
    return (field >= 0 && field < 11) ? fieldNames[field] : nullptr;
}

int ShipPacketDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "transportWideSequenceNumber") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "sequenceNumber") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "streamName") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "streamType") == 0) return baseIndex + 3;
    if (strcmp(fieldName, "elemNumber") == 0) return baseIndex + 4;
    if (strcmp(fieldName, "fragmentOffset") == 0) return baseIndex + 5;
    if (strcmp(fieldName, "isLastFragment") == 0) return baseIndex + 6;
    if (strcmp(fieldName, "payloadSize") == 0) return baseIndex + 7;
    if (strcmp(fieldName, "payloadTimestamp") == 0) return baseIndex + 8;
    if (strcmp(fieldName, "isMarked") == 0) return baseIndex + 9;
    if (strcmp(fieldName, "networkType") == 0) return baseIndex + 10;
    return base ? base->findField(fieldName) : -1;
}

const char *ShipPacketDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",    // FIELD_transportWideSequenceNumber
        "int",    // FIELD_sequenceNumber
        "string",    // FIELD_streamName
        "string",    // FIELD_streamType
        "int",    // FIELD_elemNumber
        "int",    // FIELD_fragmentOffset
        "bool",    // FIELD_isLastFragment
        "int",    // FIELD_payloadSize
        "omnetpp::simtime_t",    // FIELD_payloadTimestamp
        "bool",    // FIELD_isMarked
        "int",    // FIELD_networkType
    };
    return (field >= 0 && field < 11) ? fieldTypeStrings[field] : nullptr;
}

const char **ShipPacketDescriptor::getFieldPropertyNames(int field) const
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

const char *ShipPacketDescriptor::getFieldProperty(int field, const char *propertyName) const
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

int ShipPacketDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    ShipPacket *pp = omnetpp::fromAnyPtr<ShipPacket>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void ShipPacketDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    ShipPacket *pp = omnetpp::fromAnyPtr<ShipPacket>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'ShipPacket'", field);
    }
}

const char *ShipPacketDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    ShipPacket *pp = omnetpp::fromAnyPtr<ShipPacket>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string ShipPacketDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    ShipPacket *pp = omnetpp::fromAnyPtr<ShipPacket>(object); (void)pp;
    switch (field) {
        case FIELD_transportWideSequenceNumber: return long2string(pp->getTransportWideSequenceNumber());
        case FIELD_sequenceNumber: return long2string(pp->getSequenceNumber());
        case FIELD_streamName: return oppstring2string(pp->getStreamName());
        case FIELD_streamType: return oppstring2string(pp->getStreamType());
        case FIELD_elemNumber: return long2string(pp->getElemNumber());
        case FIELD_fragmentOffset: return long2string(pp->getFragmentOffset());
        case FIELD_isLastFragment: return bool2string(pp->isLastFragment());
        case FIELD_payloadSize: return long2string(pp->getPayloadSize());
        case FIELD_payloadTimestamp: return simtime2string(pp->getPayloadTimestamp());
        case FIELD_isMarked: return bool2string(pp->isMarked());
        case FIELD_networkType: return long2string(pp->getNetworkType());
        default: return "";
    }
}

void ShipPacketDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    ShipPacket *pp = omnetpp::fromAnyPtr<ShipPacket>(object); (void)pp;
    switch (field) {
        case FIELD_transportWideSequenceNumber: pp->setTransportWideSequenceNumber(string2long(value)); break;
        case FIELD_sequenceNumber: pp->setSequenceNumber(string2long(value)); break;
        case FIELD_streamName: pp->setStreamName((value)); break;
        case FIELD_streamType: pp->setStreamType((value)); break;
        case FIELD_elemNumber: pp->setElemNumber(string2long(value)); break;
        case FIELD_fragmentOffset: pp->setFragmentOffset(string2long(value)); break;
        case FIELD_isLastFragment: pp->setIsLastFragment(string2bool(value)); break;
        case FIELD_payloadSize: pp->setPayloadSize(string2long(value)); break;
        case FIELD_payloadTimestamp: pp->setPayloadTimestamp(string2simtime(value)); break;
        case FIELD_isMarked: pp->setIsMarked(string2bool(value)); break;
        case FIELD_networkType: pp->setNetworkType(string2long(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'ShipPacket'", field);
    }
}

omnetpp::cValue ShipPacketDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    ShipPacket *pp = omnetpp::fromAnyPtr<ShipPacket>(object); (void)pp;
    switch (field) {
        case FIELD_transportWideSequenceNumber: return pp->getTransportWideSequenceNumber();
        case FIELD_sequenceNumber: return pp->getSequenceNumber();
        case FIELD_streamName: return pp->getStreamName();
        case FIELD_streamType: return pp->getStreamType();
        case FIELD_elemNumber: return pp->getElemNumber();
        case FIELD_fragmentOffset: return pp->getFragmentOffset();
        case FIELD_isLastFragment: return pp->isLastFragment();
        case FIELD_payloadSize: return pp->getPayloadSize();
        case FIELD_payloadTimestamp: return pp->getPayloadTimestamp().dbl();
        case FIELD_isMarked: return pp->isMarked();
        case FIELD_networkType: return pp->getNetworkType();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'ShipPacket' as cValue -- field index out of range?", field);
    }
}

void ShipPacketDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    ShipPacket *pp = omnetpp::fromAnyPtr<ShipPacket>(object); (void)pp;
    switch (field) {
        case FIELD_transportWideSequenceNumber: pp->setTransportWideSequenceNumber(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_sequenceNumber: pp->setSequenceNumber(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_streamName: pp->setStreamName(value.stringValue()); break;
        case FIELD_streamType: pp->setStreamType(value.stringValue()); break;
        case FIELD_elemNumber: pp->setElemNumber(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_fragmentOffset: pp->setFragmentOffset(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_isLastFragment: pp->setIsLastFragment(value.boolValue()); break;
        case FIELD_payloadSize: pp->setPayloadSize(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_payloadTimestamp: pp->setPayloadTimestamp(value.doubleValue()); break;
        case FIELD_isMarked: pp->setIsMarked(value.boolValue()); break;
        case FIELD_networkType: pp->setNetworkType(omnetpp::checked_int_cast<int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'ShipPacket'", field);
    }
}

const char *ShipPacketDescriptor::getFieldStructName(int field) const
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

omnetpp::any_ptr ShipPacketDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    ShipPacket *pp = omnetpp::fromAnyPtr<ShipPacket>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void ShipPacketDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    ShipPacket *pp = omnetpp::fromAnyPtr<ShipPacket>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'ShipPacket'", field);
    }
}

namespace omnetpp {

}  // namespace omnetpp


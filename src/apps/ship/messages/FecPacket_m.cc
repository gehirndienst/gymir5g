//
// Generated file, do not edit! Created by opp_msgtool 6.0 from apps/ship/messages/FecPacket.msg.
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
#include "FecPacket_m.h"

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

Register_Class(FecPacket)

FecPacket::FecPacket() : ::inet::FieldsChunk()
{
    this->setChunkLength(inet::B(10));

}

FecPacket::FecPacket(const FecPacket& other) : ::inet::FieldsChunk(other)
{
    copy(other);
}

FecPacket::~FecPacket()
{
}

FecPacket& FecPacket::operator=(const FecPacket& other)
{
    if (this == &other) return *this;
    ::inet::FieldsChunk::operator=(other);
    copy(other);
    return *this;
}

void FecPacket::copy(const FecPacket& other)
{
    this->streamName = other.streamName;
    this->firstSequenceNumber = other.firstSequenceNumber;
    this->lastSequenceNumber = other.lastSequenceNumber;
    this->fecCount = other.fecCount;
    this->networkType = other.networkType;
}

void FecPacket::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::inet::FieldsChunk::parsimPack(b);
    doParsimPacking(b,this->streamName);
    doParsimPacking(b,this->firstSequenceNumber);
    doParsimPacking(b,this->lastSequenceNumber);
    doParsimPacking(b,this->fecCount);
    doParsimPacking(b,this->networkType);
}

void FecPacket::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::inet::FieldsChunk::parsimUnpack(b);
    doParsimUnpacking(b,this->streamName);
    doParsimUnpacking(b,this->firstSequenceNumber);
    doParsimUnpacking(b,this->lastSequenceNumber);
    doParsimUnpacking(b,this->fecCount);
    doParsimUnpacking(b,this->networkType);
}

const char * FecPacket::getStreamName() const
{
    return this->streamName.c_str();
}

void FecPacket::setStreamName(const char * streamName)
{
    handleChange();
    this->streamName = streamName;
}

int FecPacket::getFirstSequenceNumber() const
{
    return this->firstSequenceNumber;
}

void FecPacket::setFirstSequenceNumber(int firstSequenceNumber)
{
    handleChange();
    this->firstSequenceNumber = firstSequenceNumber;
}

int FecPacket::getLastSequenceNumber() const
{
    return this->lastSequenceNumber;
}

void FecPacket::setLastSequenceNumber(int lastSequenceNumber)
{
    handleChange();
    this->lastSequenceNumber = lastSequenceNumber;
}

int FecPacket::getFecCount() const
{
    return this->fecCount;
}

void FecPacket::setFecCount(int fecCount)
{
    handleChange();
    this->fecCount = fecCount;
}

int FecPacket::getNetworkType() const
{
    return this->networkType;
}

void FecPacket::setNetworkType(int networkType)
{
    handleChange();
    this->networkType = networkType;
}

class FecPacketDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_streamName,
        FIELD_firstSequenceNumber,
        FIELD_lastSequenceNumber,
        FIELD_fecCount,
        FIELD_networkType,
    };
  public:
    FecPacketDescriptor();
    virtual ~FecPacketDescriptor();

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

Register_ClassDescriptor(FecPacketDescriptor)

FecPacketDescriptor::FecPacketDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(FecPacket)), "inet::FieldsChunk")
{
    propertyNames = nullptr;
}

FecPacketDescriptor::~FecPacketDescriptor()
{
    delete[] propertyNames;
}

bool FecPacketDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<FecPacket *>(obj)!=nullptr;
}

const char **FecPacketDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *FecPacketDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int FecPacketDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 5+base->getFieldCount() : 5;
}

unsigned int FecPacketDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_streamName
        FD_ISEDITABLE,    // FIELD_firstSequenceNumber
        FD_ISEDITABLE,    // FIELD_lastSequenceNumber
        FD_ISEDITABLE,    // FIELD_fecCount
        FD_ISEDITABLE,    // FIELD_networkType
    };
    return (field >= 0 && field < 5) ? fieldTypeFlags[field] : 0;
}

const char *FecPacketDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "streamName",
        "firstSequenceNumber",
        "lastSequenceNumber",
        "fecCount",
        "networkType",
    };
    return (field >= 0 && field < 5) ? fieldNames[field] : nullptr;
}

int FecPacketDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "streamName") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "firstSequenceNumber") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "lastSequenceNumber") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "fecCount") == 0) return baseIndex + 3;
    if (strcmp(fieldName, "networkType") == 0) return baseIndex + 4;
    return base ? base->findField(fieldName) : -1;
}

const char *FecPacketDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "string",    // FIELD_streamName
        "int",    // FIELD_firstSequenceNumber
        "int",    // FIELD_lastSequenceNumber
        "int",    // FIELD_fecCount
        "int",    // FIELD_networkType
    };
    return (field >= 0 && field < 5) ? fieldTypeStrings[field] : nullptr;
}

const char **FecPacketDescriptor::getFieldPropertyNames(int field) const
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

const char *FecPacketDescriptor::getFieldProperty(int field, const char *propertyName) const
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

int FecPacketDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    FecPacket *pp = omnetpp::fromAnyPtr<FecPacket>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void FecPacketDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    FecPacket *pp = omnetpp::fromAnyPtr<FecPacket>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'FecPacket'", field);
    }
}

const char *FecPacketDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    FecPacket *pp = omnetpp::fromAnyPtr<FecPacket>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string FecPacketDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    FecPacket *pp = omnetpp::fromAnyPtr<FecPacket>(object); (void)pp;
    switch (field) {
        case FIELD_streamName: return oppstring2string(pp->getStreamName());
        case FIELD_firstSequenceNumber: return long2string(pp->getFirstSequenceNumber());
        case FIELD_lastSequenceNumber: return long2string(pp->getLastSequenceNumber());
        case FIELD_fecCount: return long2string(pp->getFecCount());
        case FIELD_networkType: return long2string(pp->getNetworkType());
        default: return "";
    }
}

void FecPacketDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    FecPacket *pp = omnetpp::fromAnyPtr<FecPacket>(object); (void)pp;
    switch (field) {
        case FIELD_streamName: pp->setStreamName((value)); break;
        case FIELD_firstSequenceNumber: pp->setFirstSequenceNumber(string2long(value)); break;
        case FIELD_lastSequenceNumber: pp->setLastSequenceNumber(string2long(value)); break;
        case FIELD_fecCount: pp->setFecCount(string2long(value)); break;
        case FIELD_networkType: pp->setNetworkType(string2long(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'FecPacket'", field);
    }
}

omnetpp::cValue FecPacketDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    FecPacket *pp = omnetpp::fromAnyPtr<FecPacket>(object); (void)pp;
    switch (field) {
        case FIELD_streamName: return pp->getStreamName();
        case FIELD_firstSequenceNumber: return pp->getFirstSequenceNumber();
        case FIELD_lastSequenceNumber: return pp->getLastSequenceNumber();
        case FIELD_fecCount: return pp->getFecCount();
        case FIELD_networkType: return pp->getNetworkType();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'FecPacket' as cValue -- field index out of range?", field);
    }
}

void FecPacketDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    FecPacket *pp = omnetpp::fromAnyPtr<FecPacket>(object); (void)pp;
    switch (field) {
        case FIELD_streamName: pp->setStreamName(value.stringValue()); break;
        case FIELD_firstSequenceNumber: pp->setFirstSequenceNumber(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_lastSequenceNumber: pp->setLastSequenceNumber(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_fecCount: pp->setFecCount(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_networkType: pp->setNetworkType(omnetpp::checked_int_cast<int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'FecPacket'", field);
    }
}

const char *FecPacketDescriptor::getFieldStructName(int field) const
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

omnetpp::any_ptr FecPacketDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    FecPacket *pp = omnetpp::fromAnyPtr<FecPacket>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void FecPacketDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    FecPacket *pp = omnetpp::fromAnyPtr<FecPacket>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'FecPacket'", field);
    }
}

namespace omnetpp {

}  // namespace omnetpp


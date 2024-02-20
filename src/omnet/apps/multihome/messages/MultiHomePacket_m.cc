//
// Generated file, do not edit! Created by opp_msgtool 6.0 from omnet/apps/multihome/MultiHomePacket.msg.
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
#include "MultiHomePacket_m.h"

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

Register_Class(MultiHomePacket)

MultiHomePacket::MultiHomePacket() : ::inet::FieldsChunk()
{
}

MultiHomePacket::MultiHomePacket(const MultiHomePacket& other) : ::inet::FieldsChunk(other)
{
    copy(other);
}

MultiHomePacket::~MultiHomePacket()
{
}

MultiHomePacket& MultiHomePacket::operator=(const MultiHomePacket& other)
{
    if (this == &other) return *this;
    ::inet::FieldsChunk::operator=(other);
    copy(other);
    return *this;
}

void MultiHomePacket::copy(const MultiHomePacket& other)
{
    this->id = other.id;
    this->networkType = other.networkType;
    this->destAddress = other.destAddress;
    this->destPort = other.destPort;
    this->payloadSize = other.payloadSize;
    this->sendingTime = other.sendingTime;
}

void MultiHomePacket::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::inet::FieldsChunk::parsimPack(b);
    doParsimPacking(b,this->id);
    doParsimPacking(b,this->networkType);
    doParsimPacking(b,this->destAddress);
    doParsimPacking(b,this->destPort);
    doParsimPacking(b,this->payloadSize);
    doParsimPacking(b,this->sendingTime);
}

void MultiHomePacket::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::inet::FieldsChunk::parsimUnpack(b);
    doParsimUnpacking(b,this->id);
    doParsimUnpacking(b,this->networkType);
    doParsimUnpacking(b,this->destAddress);
    doParsimUnpacking(b,this->destPort);
    doParsimUnpacking(b,this->payloadSize);
    doParsimUnpacking(b,this->sendingTime);
}

int MultiHomePacket::getId() const
{
    return this->id;
}

void MultiHomePacket::setId(int id)
{
    handleChange();
    this->id = id;
}

int MultiHomePacket::getNetworkType() const
{
    return this->networkType;
}

void MultiHomePacket::setNetworkType(int networkType)
{
    handleChange();
    this->networkType = networkType;
}

const char * MultiHomePacket::getDestAddress() const
{
    return this->destAddress.c_str();
}

void MultiHomePacket::setDestAddress(const char * destAddress)
{
    handleChange();
    this->destAddress = destAddress;
}

int MultiHomePacket::getDestPort() const
{
    return this->destPort;
}

void MultiHomePacket::setDestPort(int destPort)
{
    handleChange();
    this->destPort = destPort;
}

int MultiHomePacket::getPayloadSize() const
{
    return this->payloadSize;
}

void MultiHomePacket::setPayloadSize(int payloadSize)
{
    handleChange();
    this->payloadSize = payloadSize;
}

omnetpp::simtime_t MultiHomePacket::getSendingTime() const
{
    return this->sendingTime;
}

void MultiHomePacket::setSendingTime(omnetpp::simtime_t sendingTime)
{
    handleChange();
    this->sendingTime = sendingTime;
}

class MultiHomePacketDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_id,
        FIELD_networkType,
        FIELD_destAddress,
        FIELD_destPort,
        FIELD_payloadSize,
        FIELD_sendingTime,
    };
  public:
    MultiHomePacketDescriptor();
    virtual ~MultiHomePacketDescriptor();

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

Register_ClassDescriptor(MultiHomePacketDescriptor)

MultiHomePacketDescriptor::MultiHomePacketDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(MultiHomePacket)), "inet::FieldsChunk")
{
    propertyNames = nullptr;
}

MultiHomePacketDescriptor::~MultiHomePacketDescriptor()
{
    delete[] propertyNames;
}

bool MultiHomePacketDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<MultiHomePacket *>(obj)!=nullptr;
}

const char **MultiHomePacketDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *MultiHomePacketDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int MultiHomePacketDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 6+base->getFieldCount() : 6;
}

unsigned int MultiHomePacketDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_id
        FD_ISEDITABLE,    // FIELD_networkType
        FD_ISEDITABLE,    // FIELD_destAddress
        FD_ISEDITABLE,    // FIELD_destPort
        FD_ISEDITABLE,    // FIELD_payloadSize
        FD_ISEDITABLE,    // FIELD_sendingTime
    };
    return (field >= 0 && field < 6) ? fieldTypeFlags[field] : 0;
}

const char *MultiHomePacketDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "id",
        "networkType",
        "destAddress",
        "destPort",
        "payloadSize",
        "sendingTime",
    };
    return (field >= 0 && field < 6) ? fieldNames[field] : nullptr;
}

int MultiHomePacketDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "id") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "networkType") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "destAddress") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "destPort") == 0) return baseIndex + 3;
    if (strcmp(fieldName, "payloadSize") == 0) return baseIndex + 4;
    if (strcmp(fieldName, "sendingTime") == 0) return baseIndex + 5;
    return base ? base->findField(fieldName) : -1;
}

const char *MultiHomePacketDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",    // FIELD_id
        "int",    // FIELD_networkType
        "string",    // FIELD_destAddress
        "int",    // FIELD_destPort
        "int",    // FIELD_payloadSize
        "omnetpp::simtime_t",    // FIELD_sendingTime
    };
    return (field >= 0 && field < 6) ? fieldTypeStrings[field] : nullptr;
}

const char **MultiHomePacketDescriptor::getFieldPropertyNames(int field) const
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

const char *MultiHomePacketDescriptor::getFieldProperty(int field, const char *propertyName) const
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

int MultiHomePacketDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    MultiHomePacket *pp = omnetpp::fromAnyPtr<MultiHomePacket>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void MultiHomePacketDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    MultiHomePacket *pp = omnetpp::fromAnyPtr<MultiHomePacket>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'MultiHomePacket'", field);
    }
}

const char *MultiHomePacketDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    MultiHomePacket *pp = omnetpp::fromAnyPtr<MultiHomePacket>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string MultiHomePacketDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    MultiHomePacket *pp = omnetpp::fromAnyPtr<MultiHomePacket>(object); (void)pp;
    switch (field) {
        case FIELD_id: return long2string(pp->getId());
        case FIELD_networkType: return long2string(pp->getNetworkType());
        case FIELD_destAddress: return oppstring2string(pp->getDestAddress());
        case FIELD_destPort: return long2string(pp->getDestPort());
        case FIELD_payloadSize: return long2string(pp->getPayloadSize());
        case FIELD_sendingTime: return simtime2string(pp->getSendingTime());
        default: return "";
    }
}

void MultiHomePacketDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    MultiHomePacket *pp = omnetpp::fromAnyPtr<MultiHomePacket>(object); (void)pp;
    switch (field) {
        case FIELD_id: pp->setId(string2long(value)); break;
        case FIELD_networkType: pp->setNetworkType(string2long(value)); break;
        case FIELD_destAddress: pp->setDestAddress((value)); break;
        case FIELD_destPort: pp->setDestPort(string2long(value)); break;
        case FIELD_payloadSize: pp->setPayloadSize(string2long(value)); break;
        case FIELD_sendingTime: pp->setSendingTime(string2simtime(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'MultiHomePacket'", field);
    }
}

omnetpp::cValue MultiHomePacketDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    MultiHomePacket *pp = omnetpp::fromAnyPtr<MultiHomePacket>(object); (void)pp;
    switch (field) {
        case FIELD_id: return pp->getId();
        case FIELD_networkType: return pp->getNetworkType();
        case FIELD_destAddress: return pp->getDestAddress();
        case FIELD_destPort: return pp->getDestPort();
        case FIELD_payloadSize: return pp->getPayloadSize();
        case FIELD_sendingTime: return pp->getSendingTime().dbl();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'MultiHomePacket' as cValue -- field index out of range?", field);
    }
}

void MultiHomePacketDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    MultiHomePacket *pp = omnetpp::fromAnyPtr<MultiHomePacket>(object); (void)pp;
    switch (field) {
        case FIELD_id: pp->setId(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_networkType: pp->setNetworkType(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_destAddress: pp->setDestAddress(value.stringValue()); break;
        case FIELD_destPort: pp->setDestPort(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_payloadSize: pp->setPayloadSize(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_sendingTime: pp->setSendingTime(value.doubleValue()); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'MultiHomePacket'", field);
    }
}

const char *MultiHomePacketDescriptor::getFieldStructName(int field) const
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

omnetpp::any_ptr MultiHomePacketDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    MultiHomePacket *pp = omnetpp::fromAnyPtr<MultiHomePacket>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void MultiHomePacketDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    MultiHomePacket *pp = omnetpp::fromAnyPtr<MultiHomePacket>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'MultiHomePacket'", field);
    }
}

namespace omnetpp {

}  // namespace omnetpp


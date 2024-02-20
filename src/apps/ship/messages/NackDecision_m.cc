//
// Generated file, do not edit! Created by opp_msgtool 6.0 from apps/ship/messages/NackDecision.msg.
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
#include "NackDecision_m.h"

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

Register_Class(NackDecision)

NackDecision::NackDecision() : ::inet::FieldsChunk()
{
    this->setChunkLength(inet::B(2));

}

NackDecision::NackDecision(const NackDecision& other) : ::inet::FieldsChunk(other)
{
    copy(other);
}

NackDecision::~NackDecision()
{
}

NackDecision& NackDecision::operator=(const NackDecision& other)
{
    if (this == &other) return *this;
    ::inet::FieldsChunk::operator=(other);
    copy(other);
    return *this;
}

void NackDecision::copy(const NackDecision& other)
{
    this->decision = other.decision;
    this->reason = other.reason;
    this->timestamp = other.timestamp;
    this->streamName = other.streamName;
    this->sequenceNumber = other.sequenceNumber;
    this->networkType = other.networkType;
}

void NackDecision::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::inet::FieldsChunk::parsimPack(b);
    doParsimPacking(b,this->decision);
    doParsimPacking(b,this->reason);
    doParsimPacking(b,this->timestamp);
    doParsimPacking(b,this->streamName);
    doParsimPacking(b,this->sequenceNumber);
    doParsimPacking(b,this->networkType);
}

void NackDecision::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::inet::FieldsChunk::parsimUnpack(b);
    doParsimUnpacking(b,this->decision);
    doParsimUnpacking(b,this->reason);
    doParsimUnpacking(b,this->timestamp);
    doParsimUnpacking(b,this->streamName);
    doParsimUnpacking(b,this->sequenceNumber);
    doParsimUnpacking(b,this->networkType);
}

bool NackDecision::getDecision() const
{
    return this->decision;
}

void NackDecision::setDecision(bool decision)
{
    handleChange();
    this->decision = decision;
}

const char * NackDecision::getReason() const
{
    return this->reason.c_str();
}

void NackDecision::setReason(const char * reason)
{
    handleChange();
    this->reason = reason;
}

omnetpp::simtime_t NackDecision::getTimestamp() const
{
    return this->timestamp;
}

void NackDecision::setTimestamp(omnetpp::simtime_t timestamp)
{
    handleChange();
    this->timestamp = timestamp;
}

const char * NackDecision::getStreamName() const
{
    return this->streamName.c_str();
}

void NackDecision::setStreamName(const char * streamName)
{
    handleChange();
    this->streamName = streamName;
}

int NackDecision::getSequenceNumber() const
{
    return this->sequenceNumber;
}

void NackDecision::setSequenceNumber(int sequenceNumber)
{
    handleChange();
    this->sequenceNumber = sequenceNumber;
}

int NackDecision::getNetworkType() const
{
    return this->networkType;
}

void NackDecision::setNetworkType(int networkType)
{
    handleChange();
    this->networkType = networkType;
}

class NackDecisionDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_decision,
        FIELD_reason,
        FIELD_timestamp,
        FIELD_streamName,
        FIELD_sequenceNumber,
        FIELD_networkType,
    };
  public:
    NackDecisionDescriptor();
    virtual ~NackDecisionDescriptor();

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

Register_ClassDescriptor(NackDecisionDescriptor)

NackDecisionDescriptor::NackDecisionDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(NackDecision)), "inet::FieldsChunk")
{
    propertyNames = nullptr;
}

NackDecisionDescriptor::~NackDecisionDescriptor()
{
    delete[] propertyNames;
}

bool NackDecisionDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<NackDecision *>(obj)!=nullptr;
}

const char **NackDecisionDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *NackDecisionDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int NackDecisionDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 6+base->getFieldCount() : 6;
}

unsigned int NackDecisionDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_decision
        FD_ISEDITABLE,    // FIELD_reason
        FD_ISEDITABLE,    // FIELD_timestamp
        FD_ISEDITABLE,    // FIELD_streamName
        FD_ISEDITABLE,    // FIELD_sequenceNumber
        FD_ISEDITABLE,    // FIELD_networkType
    };
    return (field >= 0 && field < 6) ? fieldTypeFlags[field] : 0;
}

const char *NackDecisionDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "decision",
        "reason",
        "timestamp",
        "streamName",
        "sequenceNumber",
        "networkType",
    };
    return (field >= 0 && field < 6) ? fieldNames[field] : nullptr;
}

int NackDecisionDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "decision") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "reason") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "timestamp") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "streamName") == 0) return baseIndex + 3;
    if (strcmp(fieldName, "sequenceNumber") == 0) return baseIndex + 4;
    if (strcmp(fieldName, "networkType") == 0) return baseIndex + 5;
    return base ? base->findField(fieldName) : -1;
}

const char *NackDecisionDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "bool",    // FIELD_decision
        "string",    // FIELD_reason
        "omnetpp::simtime_t",    // FIELD_timestamp
        "string",    // FIELD_streamName
        "int",    // FIELD_sequenceNumber
        "int",    // FIELD_networkType
    };
    return (field >= 0 && field < 6) ? fieldTypeStrings[field] : nullptr;
}

const char **NackDecisionDescriptor::getFieldPropertyNames(int field) const
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

const char *NackDecisionDescriptor::getFieldProperty(int field, const char *propertyName) const
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

int NackDecisionDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    NackDecision *pp = omnetpp::fromAnyPtr<NackDecision>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void NackDecisionDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    NackDecision *pp = omnetpp::fromAnyPtr<NackDecision>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'NackDecision'", field);
    }
}

const char *NackDecisionDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    NackDecision *pp = omnetpp::fromAnyPtr<NackDecision>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string NackDecisionDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    NackDecision *pp = omnetpp::fromAnyPtr<NackDecision>(object); (void)pp;
    switch (field) {
        case FIELD_decision: return bool2string(pp->getDecision());
        case FIELD_reason: return oppstring2string(pp->getReason());
        case FIELD_timestamp: return simtime2string(pp->getTimestamp());
        case FIELD_streamName: return oppstring2string(pp->getStreamName());
        case FIELD_sequenceNumber: return long2string(pp->getSequenceNumber());
        case FIELD_networkType: return long2string(pp->getNetworkType());
        default: return "";
    }
}

void NackDecisionDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    NackDecision *pp = omnetpp::fromAnyPtr<NackDecision>(object); (void)pp;
    switch (field) {
        case FIELD_decision: pp->setDecision(string2bool(value)); break;
        case FIELD_reason: pp->setReason((value)); break;
        case FIELD_timestamp: pp->setTimestamp(string2simtime(value)); break;
        case FIELD_streamName: pp->setStreamName((value)); break;
        case FIELD_sequenceNumber: pp->setSequenceNumber(string2long(value)); break;
        case FIELD_networkType: pp->setNetworkType(string2long(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'NackDecision'", field);
    }
}

omnetpp::cValue NackDecisionDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    NackDecision *pp = omnetpp::fromAnyPtr<NackDecision>(object); (void)pp;
    switch (field) {
        case FIELD_decision: return pp->getDecision();
        case FIELD_reason: return pp->getReason();
        case FIELD_timestamp: return pp->getTimestamp().dbl();
        case FIELD_streamName: return pp->getStreamName();
        case FIELD_sequenceNumber: return pp->getSequenceNumber();
        case FIELD_networkType: return pp->getNetworkType();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'NackDecision' as cValue -- field index out of range?", field);
    }
}

void NackDecisionDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    NackDecision *pp = omnetpp::fromAnyPtr<NackDecision>(object); (void)pp;
    switch (field) {
        case FIELD_decision: pp->setDecision(value.boolValue()); break;
        case FIELD_reason: pp->setReason(value.stringValue()); break;
        case FIELD_timestamp: pp->setTimestamp(value.doubleValue()); break;
        case FIELD_streamName: pp->setStreamName(value.stringValue()); break;
        case FIELD_sequenceNumber: pp->setSequenceNumber(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_networkType: pp->setNetworkType(omnetpp::checked_int_cast<int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'NackDecision'", field);
    }
}

const char *NackDecisionDescriptor::getFieldStructName(int field) const
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

omnetpp::any_ptr NackDecisionDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    NackDecision *pp = omnetpp::fromAnyPtr<NackDecision>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void NackDecisionDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    NackDecision *pp = omnetpp::fromAnyPtr<NackDecision>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'NackDecision'", field);
    }
}

namespace omnetpp {

}  // namespace omnetpp


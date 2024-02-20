#include <algorithm>
#include <string>
#include <unordered_map>

#pragma once

enum class PacketType {
    // ack
    ACK,
    // special requests
    NACK,
    FIR,
    // feedback
    RECEIVERREPORT,
    SENDERREPORT,
    TRANSPORTFEEDBACK,
    // redundancy
    FEC,
    // data
    DATA,
    // default
    OTHER
};

namespace packettype {

    static const std::unordered_map<PacketType, std::string> packetTypeToStringMap{
        {PacketType::ACK, "ack"},
        {PacketType::NACK, "nack"},
        {PacketType::FIR, "fir"},
        {PacketType::RECEIVERREPORT, "receiverreport"},
        {PacketType::SENDERREPORT, "senderreport"},
        {PacketType::TRANSPORTFEEDBACK, "transportfeedback"},
        {PacketType::FEC, "fec"},
        {PacketType::DATA, "data"},
        {PacketType::OTHER, "other"}
    };

    static const std::unordered_map<std::string, PacketType> stringToPacketTypeMap{
        {"ack", PacketType::ACK},
        {"nack", PacketType::NACK},
        {"fir", PacketType::FIR},
        {"receiverreport", PacketType::RECEIVERREPORT},
        {"senderreport", PacketType::SENDERREPORT},
        {"transportfeedback", PacketType::TRANSPORTFEEDBACK},
        {"fec", PacketType::FEC},
        {"data", PacketType::DATA},
        {"other", PacketType::OTHER}
    };

    inline const std::string& toString(PacketType packetType) {
        auto it = packetTypeToStringMap.find(packetType);
        return (it != packetTypeToStringMap.end()) ? it->second : packetTypeToStringMap.at(PacketType::OTHER);
    }

    inline PacketType fromString(std::string& packetTypeStr) {
        std::transform(packetTypeStr.begin(), packetTypeStr.end(), packetTypeStr.begin(), ::tolower);
        auto it = stringToPacketTypeMap.find(packetTypeStr);
        return (it != stringToPacketTypeMap.end()) ? it->second : PacketType::OTHER;
    }
}
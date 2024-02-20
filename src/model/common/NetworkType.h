#include <algorithm>
#include <string>
#include <unordered_map>

#pragma once

enum class NetworkType {
    LTE,
    NR,
    WIFI,
    DONTCARE,
};

namespace networktype {

    static const std::unordered_map<NetworkType, std::string> networkTypeToStringMap{
        {NetworkType::LTE, "lte"},
        {NetworkType::NR, "nr"},
        {NetworkType::WIFI, "wifi"},
        {NetworkType::DONTCARE, "dontcare"}
    };

    static const std::unordered_map<std::string, NetworkType> stringToNetworkTypeMap{
        {"lte", NetworkType::LTE},
        {"nr", NetworkType::NR},
        {"wifi", NetworkType::WIFI},
        {"dontcare", NetworkType::DONTCARE}
    };

    inline const std::string& toString(NetworkType networkType) {
        auto it = networkTypeToStringMap.find(networkType);
        return (it != networkTypeToStringMap.end()) ? it->second : networkTypeToStringMap.at(NetworkType::DONTCARE);
    }

    inline NetworkType fromString(std::string& networkTypeStr) {
        std::transform(networkTypeStr.begin(), networkTypeStr.end(), networkTypeStr.begin(), ::tolower);
        auto it = stringToNetworkTypeMap.find(networkTypeStr);
        return (it != stringToNetworkTypeMap.end()) ? it->second : NetworkType::DONTCARE;
    }
}
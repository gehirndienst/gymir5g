#include <algorithm>
#include <string>
#include <unordered_map>

#pragma once

enum class AdaptiveAlgorithm {
    ABR_BASE,
    ABR_GCC,
    DRL_BASE,
    NO
};

namespace adaptivealgorithm {

    static const std::unordered_map<AdaptiveAlgorithm, std::string> adaptiveAlgorithmToStringMap{
        {AdaptiveAlgorithm::ABR_BASE, "abr_base"},
        {AdaptiveAlgorithm::ABR_GCC, "abr_gcc"},
        {AdaptiveAlgorithm::DRL_BASE, "drl_base"},
        {AdaptiveAlgorithm::NO, "no"}
    };

    static const std::unordered_map<std::string, AdaptiveAlgorithm> stringToAdaptiveAlgorithmMap{
        {"abr_base", AdaptiveAlgorithm::ABR_BASE},
        {"abr_gcc", AdaptiveAlgorithm::ABR_GCC},
        {"drl_base", AdaptiveAlgorithm::DRL_BASE},
        {"no", AdaptiveAlgorithm::NO}
    };

    inline const std::string& toString(AdaptiveAlgorithm algorithm) {
        auto it = adaptiveAlgorithmToStringMap.find(algorithm);
        return (it != adaptiveAlgorithmToStringMap.end()) ? it->second : adaptiveAlgorithmToStringMap.at(
                   AdaptiveAlgorithm::NO);
    }

    inline AdaptiveAlgorithm fromString(const std::string& algorithmStr) {
        std::string lowercaseStr = algorithmStr;
        std::transform(lowercaseStr.begin(), lowercaseStr.end(), lowercaseStr.begin(), ::tolower);

        auto it = stringToAdaptiveAlgorithmMap.find(lowercaseStr);
        if (it != stringToAdaptiveAlgorithmMap.end()) {
            return it->second;
        } else {
            return AdaptiveAlgorithm::NO;
        }
    }
}
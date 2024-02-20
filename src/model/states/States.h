/*
 * States.h
 *
 * enum with available states
 *
 * Created on: July 7, 2023
 *      Author: Nikita Smirnov
 */

#include <string>
#include <boost/algorithm/string.hpp>

enum StateType {
    PHY,
    STREAM,
    TRANSMISSION
};

inline StateType getStateType(std::string& stateName) {
    boost::algorithm::to_lower(stateName);
    if (stateName == "phy") {
        return PHY;
    } else if (stateName == "stream") {
        return STREAM;
    } else if (stateName == "transmission") {
        return TRANSMISSION;
    } else {
        throw std::invalid_argument(
            "States::getStateType wrong state type: " + stateName);
    }
}

inline std::string stateTypeToStr(StateType& stateType) {
    switch (stateType) {
        case PHY:
            return "phy";
        case STREAM:
            return "stream";
        case TRANSMISSION:
            return "transmission";
        default:
            return "";
    }
}
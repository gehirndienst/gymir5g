#include <string>
#include <stdexcept>

#pragma once

enum class WaypointType {
    NORMAL,
    DOCKING,
    DOCKING_NEARBY,
    FULL_GAS
};

WaypointType strToWaypointType(const std::string& str) {
    if (str == "normal") {
        return WaypointType::NORMAL;
    } else if (str == "docking") {
        return WaypointType::DOCKING;
    } else if (str == "docking_nearby") {
        return WaypointType::DOCKING_NEARBY;
    } else if (str == "full_gas") {
        return WaypointType::FULL_GAS;
    } else {
        throw std::runtime_error("unknown waypoint type: " + str);
    }
}
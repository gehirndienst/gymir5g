#include "DockingMobility.h"

Define_Module(DockingMobility);

void DockingMobility::initialize(int stage) {
    MovingMobilityBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        waypointProximity = par("waypointProximity");
        dockingStopPeriod = par("dockingStopPeriod");
        dockingStartTime = 0;
        lastWaypointIndex = 0;
        heading = 0;

        std::ifstream waypointsSchema(par("waypointsFile").stringValue());
        nlohmann::json waypointsJson = nlohmann::json::parse(waypointsSchema);
        fillWaypoints(waypointsJson);
    }
}

void DockingMobility::setInitialPosition() {
    lastPosition.x = waypoints[lastWaypointIndex].x;
    lastPosition.y = waypoints[lastWaypointIndex].y;
    lastVelocity.x = waypoints[lastWaypointIndex].speed * cos(M_PI * heading / 180);
    lastVelocity.y = waypoints[lastWaypointIndex].speed * sin(M_PI * heading / 180);
}

void DockingMobility::fillWaypoints(nlohmann::json& waypointsJson) {
    for (const auto& waypointJson : waypointsJson["objects"]) {
        Waypoint waypoint;
        waypoint.x = (double)waypointJson["x"];
        waypoint.y = (double)waypointJson["y"];
        waypoint.speed = (double)waypointJson["speed"];
        std::string type = std::string(waypointJson["type"]);
        waypoint.type = strToWaypointType(type);
        waypoint.isPassed = false;
        waypoints.push_back(waypoint);
    }
}

void DockingMobility::move() {
    if (isDocking()) {
        lastVelocity = Coord::ZERO;
        return;
    }
    Waypoint target = waypoints[lastWaypointIndex];
    double dx = target.x - lastPosition.x;
    double dy = target.y - lastPosition.y;
    if (dx * dx + dy * dy < waypointProximity * waypointProximity) {
        waypoints[lastWaypointIndex].isPassed = true;
        lastWaypointIndex = std::min(lastWaypointIndex + 1, (int)waypoints.size() - 1);
        if (waypoints[lastWaypointIndex].type == WaypointType::DOCKING) {
            dockingStartTime = simTime();
        }
    }
    double targetDirection = atan2(dy, dx) / M_PI * 180;
    double diff = targetDirection - heading;
    while (diff < -180)
        diff += 360;
    while (diff > 180)
        diff -= 360;
    double angularSpeed = diff * 5;
    double timeStep = (simTime() - lastUpdate).dbl();
    heading += angularSpeed * timeStep;

    Coord tempSpeed = Coord(cos(M_PI * heading / 180), sin(M_PI * heading / 180)) * waypoints[lastWaypointIndex].speed;
    Coord tempPosition = lastPosition + tempSpeed * timeStep;

    lastVelocity = tempPosition - lastPosition;
    lastPosition = tempPosition;
}

bool DockingMobility::isDocking() {
    return waypoints[lastWaypointIndex].type == WaypointType::DOCKING
           && dockingStartTime > 0.0
           && simTime() - dockingStartTime < dockingStopPeriod;
}

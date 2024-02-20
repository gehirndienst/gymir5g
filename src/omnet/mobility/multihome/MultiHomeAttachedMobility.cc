#include "MultiHomeAttachedMobility.h"

Define_Module(MultiHomeAttachedMobility);

void MultiHomeAttachedMobility::initialize(int stage) {
    MobilityBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        initTime = simTime();
        maxWaitingTimeToStart = par("maxWaitingTimeToStart").doubleValue();
        // replace original mobility.reference(this, "mobilityModule", true);
        // NOTE: unfortunately we have to leave this StationaryMobility object undisposed later because there are listeners
        // to this module and in case of reassigning the pointer and deleting the original one, they are crashed.
        mobility = new StationaryMobility();
        // then copy-paste without changes
        positionOffset.x = par("offsetX");
        positionOffset.y = par("offsetY");
        positionOffset.z = par("offsetZ");
        auto alpha = deg(par("offsetHeading"));
        auto offsetElevation = deg(par("offsetElevation"));
        auto beta = -offsetElevation;
        auto gamma = deg(par("offsetBank"));
        orientationOffset = Quaternion(EulerAngles(alpha, beta, gamma));
        isZeroOffset = positionOffset == Coord::ZERO;
        check_and_cast<cModule *>(mobility)->subscribe(IMobility::mobilityStateChangedSignal, this);
        WATCH(lastVelocity);
        WATCH(lastAngularPosition);
        // finally run a method which will look for a dynamic mobility and rewrite the pointer
        findMainMobility();
    }
}

void MultiHomeAttachedMobility::handleSelfMessage(cMessage* msg) {
    if (!strcmp(msg->getName(), "findMainMobility")) {
        delete msg;
        findMainMobility();
    }
}

void MultiHomeAttachedMobility::findMainMobility() {
    // find mobility from the main module that is dynamic
    cModule* mainMobilityModule = findModuleByPath(par("mobilityModule"));
    if (!mainMobilityModule) {
        if ((simTime() - initTime).dbl() > maxWaitingTimeToStart) {
            throw cRuntimeError("MultiHomeAttachedMobility::findMainMobility: can't find main mobility, aborting...");
        }
        scheduleAt(simTime() + 0.05, new cMessage("findMainMobility"));
    } else {
        // now we are ready to finish initialization that points to a valid mobility of a dynamic module
        mobility = check_and_cast<IMobility*>(mainMobilityModule);
        std::cout << "MultiHomeAttachedMobility::findMainMobility main mobility was found, go on..."
                  << std::endl;
    }
}

// other methods are simply copied from AttachedMobility
void MultiHomeAttachedMobility::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) {
    Enter_Method("%s", cComponent::getSignalName(signal));

    if (IMobility::mobilityStateChangedSignal == signal)
        emitMobilityStateChangedSignal();
}

const Coord& MultiHomeAttachedMobility::getCurrentPosition() {
    if (isZeroOffset)
        lastPosition = mobility->getCurrentPosition();
    else {
        RotationMatrix rotation(mobility->getCurrentAngularPosition().toEulerAngles());
        lastPosition = mobility->getCurrentPosition() + rotation.rotateVector(positionOffset);
    }
    return lastPosition;
}

const Coord& MultiHomeAttachedMobility::getCurrentVelocity() {
    if (isZeroOffset)
        lastVelocity = mobility->getCurrentVelocity();
    else {
        RotationMatrix rotation(mobility->getCurrentAngularPosition().toEulerAngles());
        Coord rotatedOffset = rotation.rotateVector(positionOffset);
        Quaternion quaternion(mobility->getCurrentAngularVelocity());
        Coord rotationAxis;
        double rotationAngle;
        quaternion.getRotationAxisAndAngle(rotationAxis, rotationAngle);
        auto additionalVelocity = rotationAngle == 0 ? Coord::ZERO : rotationAxis % rotatedOffset * rotationAngle;
        lastVelocity = mobility->getCurrentVelocity() + additionalVelocity;
    }
    return lastVelocity;
}

const Coord& MultiHomeAttachedMobility::getCurrentAcceleration() {
    if (isZeroOffset)
        return mobility->getCurrentAcceleration();
    else {
        return Coord::NIL;
    }
}

const Quaternion& MultiHomeAttachedMobility::getCurrentAngularPosition() {
    lastAngularPosition = mobility->getCurrentAngularPosition();
    lastAngularPosition *= Quaternion(orientationOffset);
    return lastAngularPosition;
}

const Quaternion& MultiHomeAttachedMobility::getCurrentAngularVelocity() {
    return mobility->getCurrentAngularVelocity();
}

const Quaternion& MultiHomeAttachedMobility::getCurrentAngularAcceleration() {
    return mobility->getCurrentAngularAcceleration();
}


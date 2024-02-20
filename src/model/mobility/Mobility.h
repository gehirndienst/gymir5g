/*
 * Mobility.h
 *
 * a class that helps to control vehicle info like coors, speed, etc.
 * it works with inet mobility classes and with TRaCI mobility from veins
 *
 *  Created on: Nov 10, 2023
 *      Author: Nikita Smirnov
 */

#ifndef MOBILITY_H
#define MOBILITY_H

#include <inet/mobility/contract/IMobility.h>
#include <veins_inet/veins_inet.h>
#include <veins_inet/VeinsInetMobility.h>

class Mobility {
public:
    Mobility() = default;
    ~Mobility() = default;

    void setInetMobility(inet::IMobility* inetMobility) {
        inetMobility_ = inetMobility;
        isVeins_ = false;
    }

    void setVeinsMobility(veins::VeinsInetMobility* veinsMobility) {
        veinsMobility_ = veinsMobility;
        isVeins_ = true;
    }

    double getSpeed() {
        if (isVeins_) {
            return veinsMobility_->getCurrentVelocity().length();
        } else {
            return inetMobility_->getCurrentVelocity().length();
        }
    }

    inet::Coord& getCoords() {
        if (isVeins_) {
            lastPosition_ = veinsMobility_->getCurrentPosition();
            return lastPosition_;
        } else {
            lastPosition_ = inetMobility_->getCurrentPosition();
            return lastPosition_;
        }
    }
private:
    bool isVeins_;
    inet::IMobility* inetMobility_;
    veins::VeinsInetMobility* veinsMobility_;

    inet::Coord lastPosition_;
};

#endif
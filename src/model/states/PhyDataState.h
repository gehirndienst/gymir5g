/*
 * PhyDataState.h
 *
 * 5G PHY data (UL/DL) state class to be used at the app
 *
 *  Created on: Apr 28, 2023
 *      Author: Nikita Smirnov
 */

#ifndef PHYDATASTATE_H
#define PHYDATASTATE_H

#include <vector>

#include "common/LteCommon.h"
#include "omnetpp.h"

#include "domain/CSVLogger.h"
#include "domain/Statistics.h"
#include "omnet/stack/phy/PhyData.h"

class PhyDataState {
public:
    PhyDataState(double carrierFrequency = 0.0, int verbose = 0);
    ~PhyDataState();

    void updateWithDlData(std::unique_ptr<PhyData> phyDataDl);
    void updateWithUlData(std::unique_ptr<PhyData> phyDataUl);

    double getCarrierFrequency() const;
    void setCarrierFrequency(double carrierFrequency);

    void print(std::ostream& os);
    void print();

    void log(double currentTime = 0.0);
    void clearCaches();

public:
    ElementCounter<int> cwDl;
    ElementCounter<int> cwUl;

    Statistics<int> cqiDl;
    Statistics<int> cqiUl;

    ElementCounter<uint16_t> riDl;
    ElementCounter<uint16_t> riUl;

    ElementCounter<uint16_t> pmiDl;
    ElementCounter<uint16_t> pmiUl;

    ElementCounter<std::string> modulationDl;
    ElementCounter<std::string> modulationUl;

    Statistics<int> numUsedRbs;
    Statistics<double> sinr;
    Statistics<double> rsrp;
    Statistics<double> rsrq;
    Statistics<double> rssi;
    Statistics<double> bler;
    Statistics<double> harqper;

    ElementCounter<std::string> enbs;
    ElementCounter<bool> nrs;

    inet::Coord ueCoord = inet::Coord::NIL;
    inet::Coord enbCoord = inet::Coord::NIL;

    double startTime = 0.0;
    double duration = 0.0;
    int fbkRcvdDl = 0;
    int fbkRcvdUl = 0;
    int fbkRcvdSgnl = 0;

    int verbose;

private:
    double carrierFrequency_;
    CSVLogger* logger_;
};

#endif
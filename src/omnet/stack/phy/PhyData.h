/*
 * PhyData.h
 *
 * OMNeT++ container for 5G PHY data
 *
 *  Created on: Apr 28, 2023
 *      Author: Nikita Smirnov
 */

#ifndef PHYDATA_H
#define PHYDATA_H

#include <vector>

#include "common/LteCommon.h"
#include "omnetpp.h"

class PhyData : public omnetpp::cObject {
public:
    // frame type
    std::string frameType;
    // carrier frequency
    double carrierFrequency;
    // num bands (resource blocks)
    int numBands;

    // user tx params (see UserTxParams.h)
    int cw; // codeword
    int cqi; // channel quality indicator from 1 to 15
    uint16_t ri; // rank indicator
    uint16_t pmi; // precoding matrix indicator
    std::string modulation; // QPSK, 16QAM, 64QAM, 256QAM

    // channel params
    int numUsedRbs;
    double sinr;
    double rsrp;
    double rsrq;
    double rssi;
    double bler;
    double harqper;

    // enb info
    std::string direction;
    uint16_t cellId;
    uint16_t enbId;
    std::string enbName;
    bool isNr; // for UL always true, for DL depends

    // coords
    inet::Coord ueCoord;
    inet::Coord enbCoord;

    // time
    omnetpp::simtime_t time;

public:
    PhyData()
        : frameType("")
        , carrierFrequency(0.0)
        , numBands(0)
        , cw(0)
        , cqi(0)
        , ri(0)
        , pmi(0)
        , modulation("")
        , numUsedRbs(0)
        , sinr(0.0)
        , rsrp(0.0)
        , rsrq(0.0)
        , rssi(0.0)
        , bler(0.0)
        , harqper(0.0)
        , direction("")
        , cellId(0)
        , enbId(0)
        , enbName("")
        , isNr(false)
        , ueCoord(inet::Coord::NIL)
        , enbCoord(inet::Coord::NIL)
        , time(0.0)
    {}
};

#endif
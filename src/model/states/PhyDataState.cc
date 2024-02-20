#include "PhyDataState.h"

PhyDataState::PhyDataState(double carrierFrequency, int verbose)
    : verbose(verbose)
    , carrierFrequency_(carrierFrequency)
    , logger_(nullptr) {}

PhyDataState::~PhyDataState() {
    if (logger_) delete logger_;
}

void PhyDataState::updateWithDlData(std::unique_ptr<PhyData> phyDataDl) {
    cwDl.add(phyDataDl->cw);
    cqiDl.add(phyDataDl->cqi);
    riDl.add(phyDataDl->ri);
    pmiDl.add(phyDataDl->pmi);
    modulationDl.add(phyDataDl->modulation);

    if (phyDataDl->numUsedRbs > 0) {
        numUsedRbs.add(phyDataDl->numUsedRbs);
        sinr.add(phyDataDl->sinr);
        rsrp.add(phyDataDl->rsrp);
        rsrq.add(phyDataDl->rsrq);
        rssi.add(phyDataDl->rssi);
        bler.add(phyDataDl->bler);
        harqper.add(phyDataDl->harqper);
        fbkRcvdSgnl++;
    }

    enbs.add(phyDataDl->enbName);
    nrs.add(phyDataDl->isNr);

    ueCoord = phyDataDl->ueCoord;
    enbCoord = phyDataDl->enbCoord;

    if (fbkRcvdDl == 0 && fbkRcvdUl == 0) {
        startTime = phyDataDl->time.dbl();
    } else {
        duration = phyDataDl->time.dbl() - startTime;
    }
    fbkRcvdDl++;

    if (verbose > 1) print();
}

void PhyDataState::updateWithUlData(std::unique_ptr<PhyData> phyDataUl) {
    cwUl.add(phyDataUl->cw);
    cqiUl.add(phyDataUl->cqi);
    riUl.add(phyDataUl->ri);
    pmiUl.add(phyDataUl->pmi);
    modulationUl.add(phyDataUl->modulation);

    if (fbkRcvdUl == 0 && fbkRcvdUl == 0) {
        startTime = phyDataUl->time.dbl();
    } else {
        duration = phyDataUl->time.dbl() - startTime;
    }
    fbkRcvdUl++;

    if (verbose > 1) print();
}

void PhyDataState::setCarrierFrequency(double carrierFrequency) {
    if (carrierFrequency_ > 0) {
        throw std::runtime_error("PhyDataState::setCarrierFrequency Carrier frequency is already set");
    } else {
        if (carrierFrequency > 0) {
            carrierFrequency_ = carrierFrequency;
        }
    }
}

double PhyDataState::getCarrierFrequency() const { return carrierFrequency_; }

void PhyDataState::print(std::ostream& os) {
    os << "--------------------------------------------------------" << std::endl;
    os << "\t" << "5G PHY data state for CF " << carrierFrequency_ << std::endl;
    os << "--------------------------------------------------------" << std::endl;
    os << "\t" << "GENERAL" << std::endl;
    os << "Time passed (sec) " << duration << std::endl;
    os << "Feedbacks received: UL " << fbkRcvdDl << " DL " << fbkRcvdDl << " SGNL " << fbkRcvdSgnl << std::endl;
    os << "UE Coord " << ueCoord << std::endl;
    os << "ENB Coord " << enbCoord << std::endl;
    os << "ENB name " << enbs.getLast() << " handovers " << enbs.getNumChangesHappened() << std::endl;
    os << "ENB NR? " << (nrs.getLast() ? "true" : "false") << std::endl;
    os << "--------------------------------------------------------" << std::endl;
    os << "\t" << "UL TX BLOCK" << std::endl;
    os << "cw last/mc/mc_ratio(%): " << cwUl.getLast() << " / " << cwUl.getMostCommon() << " / " <<
       cwUl.getMostCommonRatio() * 100.0 << std::endl;
    os << "RI last/mc/mc_ratio(%): " << riUl.getLast() << " / " << riUl.getMostCommon() << " / " <<
       riUl.getMostCommonRatio() * 100.0 << std::endl;
    os << "PMI last/mc/mc_ratio(%): " << pmiUl.getLast() << " / " << pmiUl.getMostCommon() << " / " <<
       pmiUl.getMostCommonRatio() * 100.0 << std::endl;
    os << "modulation last/mc/mc_ratio(%): " << modulationUl.getLast() << " / " << modulationUl.getMostCommon() << " / " <<
       modulationUl.getMostCommonRatio() * 100.0 << std::endl;
    os << "UL CQI last/min/avg/max: " << cqiUl.getLast() << " / " << cqiUl.getMin() << " / " << cqiUl.getMean() << " / " <<
       cqiUl.getMax() << std::endl;
    os << "--------------------------------------------------------" << std::endl;
    os << "\t" << "DL TX BLOCK" << std::endl;
    os << "cw last/mc/mc_ratio(%): " << cwDl.getLast() << " / " << cwDl.getMostCommon() << " / " <<
       cwDl.getMostCommonRatio() * 100.0 << std::endl;
    os << "RI last/mc/mc_ratio(%): " << riDl.getLast() << " / " << riDl.getMostCommon() << " / " <<
       riDl.getMostCommonRatio() * 100.0 << std::endl;
    os << "PMI last/mc/mc_ratio(%): " << pmiDl.getLast() << " / " << pmiDl.getMostCommon() << " / " <<
       pmiDl.getMostCommonRatio() * 100.0 << std::endl;
    os << "modulation last/mc/mc_ratio(%): " << modulationDl.getLast() << " / " << modulationDl.getMostCommon() << " / " <<
       modulationDl.getMostCommonRatio() * 100.0 << std::endl;
    os << "DL CQI last/min/avg/max: " << cqiDl.getLast() << " / " << cqiDl.getMin() << " / " << cqiDl.getMean() << " / " <<
       cqiDl.getMax() << std::endl;
    os << "--------------------------------------------------------" << std::endl;
    os << "\t" << "SIGNAL BLOCK" << std::endl;
    os << "NumUsedRbs last/min/avg/max: " << numUsedRbs.getLast() << " / " << numUsedRbs.getMin() << " / " <<
       numUsedRbs.getMean() << " / " << numUsedRbs.getMax() << std::endl;
    os << "SINR last/min/avg/max: " << sinr.getLast() << " / " << sinr.getMin() << " / " << sinr.getMean() << " / " <<
       sinr.getMax() << std::endl;
    os << "RSRP last/min/avg/max: " << rsrp.getLast() << " / " << rsrp.getMin() << " / " << rsrp.getMean() << " / " <<
       rsrp.getMax() << std::endl;
    os << "RSRQ last/min/avg/max: " << rsrq.getLast() << " / " << rsrq.getMin() << " / " << rsrq.getMean() << " / " <<
       rsrq.getMax() << std::endl;
    os << "RSSI last/min/avg/max: " << rssi.getLast() << " / " << rssi.getMin() << " / " << rssi.getMean() << " / " <<
       rssi.getMax() << std::endl;
    os << "BLER last/min/avg/max: " << bler.getLast() << " / " << bler.getMin() << " / " << bler.getMean() << " / " <<
       bler.getMax() << std::endl;
    os << "H-ARQ Reduced PER last/min/avg/max: " << harqper.getLast() << " / " << harqper.getMin() << " / " <<
       harqper.getMean() << " / " << harqper.getMax() << std::endl;
    os << "--------------------------------------------------------" << std::endl;
}

void PhyDataState::print() {
    print(std::cout);
}

void PhyDataState::log(double currentTime) {
    // init
    if (!logger_ && verbose >= 1) {
        std::string logFile = "phydata_cf_" + std::to_string(carrierFrequency_) + "_";
        logger_ = new CSVLogger(logFile);
    }
    if (!logger_->getIsHeaderWritten()) {
        // write headers
        *logger_ <<
                 "time" <<
                 "duration" <<
                 "ueCoord" <<
                 "enbCoord" <<
                 "enbName" <<
                 "isNr" <<
                 "cqiUl" <<
                 "modulationUl" <<
                 "numUsedRbs" <<
                 "rsrp" <<
                 "sinr" <<
                 "bler" <<
                 "harqper" << CSVLogger::endl;
    } else {
        // log
        *logger_ <<
                 currentTime <<
                 duration <<
                 std::to_string(ueCoord.getX()) + "," + std::to_string(ueCoord.getY()) + "," + std::to_string(
                     ueCoord.getZ()) <<
                 std::to_string(enbCoord.getX()) + "," + std::to_string(enbCoord.getY()) + "," + std::to_string(
                     enbCoord.getZ()) <<
                 enbs.getLast() <<
                 (nrs.getLast() ? "NR" : "LTE") <<
                 cqiUl.getLast() <<
                 modulationUl.getLast() <<
                 numUsedRbs.getLast() <<
                 rsrp.getLast() <<
                 sinr.getLast() <<
                 bler.getLast() <<
                 harqper.getLast() << CSVLogger::endl;
        clearCaches();
    }
}

void PhyDataState::clearCaches() {
    cwUl.clearCache();
    riUl.clearCache();
    pmiUl.clearCache();
    modulationUl.clearCache();
    cqiUl.clearCache();
    cwDl.clearCache();
    riDl.clearCache();
    pmiDl.clearCache();
    modulationDl.clearCache();
    cqiDl.clearCache();
    numUsedRbs.clearCache();
    sinr.clearCache();
    rsrp.clearCache();
    rsrq.clearCache();
    rssi.clearCache();
    bler.clearCache();
    harqper.clearCache();
}
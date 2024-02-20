#include "ReportBlock_m.h"

Register_Class(ReportBlock);

std::string ReportBlock::str() const {
    std::stringstream out;
    out << "ReportBlock stream name=" << getStreamName();
    return out.str();
}

void ReportBlock::dump(std::ostream& os) const {
    os << "ReportBlock:" << std::endl;
    os << "  stream name = " << getStreamName() << std::endl;
    os << "  fractionLost = " << (int)getFractionLost() << std::endl;
    os << "  packetsLostCumulative = " << getPacketsLostCumulative() << std::endl;
    os << "  extendedHighestSequenceNumber = " << getMaxSequenceNumber() << std::endl;
    os << "  jitter = " << getJitter() << std::endl;
    os << "  lastSR = " << getLastSR() << std::endl;
    os << "  delaySinceLastSR = " << getDelaySinceLastSR() << std::endl;

    // extended
    os << "  EXT: fractionRate = " << getFractionRate() << std::endl;
    os << "  EXT: fractionFecRate = " << getFractionFecRate() << std::endl;
    os << "  EXT: packetsOutOfOrderCumulative = " << getPacketsOutOfOrderCumulative() << std::endl;
    os << "  EXT: packetsPlayedCumulative = " << getPacketsPlayedCumulative() << std::endl;
    os << "  EXT: packetsRetransmittedCumulative = " << getPacketsRetransmittedCumulative() << std::endl;
    os << "  EXT: fractionRetransmissionDelayMean = " << getFractionRetransmissionDelayMean() << std::endl;
    os << "  EXT: packetsRepairedCumulative = " << getPacketsRepairedCumulative() << std::endl;
    os << "  EXT: packetsRepairedAndRetransmittedCumulative = " << getPacketsRepairedAndRetransmittedCumulative() <<
       std::endl;
    os << "  EXT: fractionPlayoutDelayMean = " << getFractionPlayoutDelayMean() << std::endl;
    os << "  EXT: fractionStallingRate = " << getFractionStallingRate() << std::endl;
}


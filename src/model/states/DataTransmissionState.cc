#include "DataTransmissionState.h"

DataTransmissionState::DataTransmissionState(int verbose)
    : verbose(verbose) {}

void DataTransmissionState::updateWithPacket(
    const inet::Ptr<const inet::Chunk>& packet, PacketType packetType, int twNumber) {
    switch (packetType) {
        case PacketType::ACK:
            onNewAckPacket(packet);
            break;
        case PacketType::NACK:
            onNewNackPacket(packet);
            break;
        case PacketType::DATA:
            onNewDataPacket(packet);
            break;
        case PacketType::FEC:
            onNewFecPacket(packet);
            break;
        case PacketType::RECEIVERREPORT:
            onNewReceiverReportPacket(packet, twNumber);
            break;
        case PacketType::TRANSPORTFEEDBACK:
            onNewTransportFeedbackPacket(packet, twNumber);
            break;
        default:
            break;
    }
}

void DataTransmissionState::onNewAckPacket(const inet::Ptr<const inet::Chunk>& packet) {
    inet::Ptr<const AckPacket> ackPacket = inet::dynamicPtrCast<const AckPacket>(packet);

    omnetpp::simtime_t senderTimestamp = ackPacket->getSenderTimestamp();
    omnetpp::simtime_t arrivalTimestamp = ackPacket->getArrivalTimestamp();
    omnetpp::simtime_t ackTimestamp = omnetpp::simTime();

    // update acks
    numAckPackets ++;

    // time in seconds
    if (numRxPackets == 0) {
        // first rx packet, save time
        timeFirstPacketArrival = arrivalTimestamp;
        timeFirstPacketAck = ackTimestamp;
    }
    timeCurrentPacketAck = ackTimestamp;
    timeCurrentPacketArrival = arrivalTimestamp;
    rxDuration = (timeCurrentPacketArrival - timeFirstPacketArrival).dbl();

    // packet delay/rtt in milliseconds
    double packetDelay = (arrivalTimestamp - senderTimestamp).dbl() * 1000;
    double packetRtt = (ackTimestamp - senderTimestamp).dbl() * 1000;

    // ok or out-of-order ack? reestimate in case of instability?
    int packetIdDifference = ackPacket->getSequenceNumber() - lastOkPacketId;
    if (packetIdDifference > 0) {
        lastOkPacketId = ackPacket->getSequenceNumber();
        numRxPackets ++;
        numRxBytes += ackPacket->getPayloadSize();
        numLostPackets += packetIdDifference - 1;
        lossRate = double(numLostPackets) / double(numRxPackets + numLostPackets);
    } else if (packetIdDifference < 0) {
        numRxPackets ++;
        numOutOfOrderPackets ++;
        numLostPackets --;
        numRxBytes += ackPacket->getPayloadSize();
        outOfOrderRate = double(numOutOfOrderPackets) / double(numRxPackets);
        lossRate = double(numLostPackets) / double(numRxPackets + numLostPackets);
    } else {
        throw std::runtime_error("Packets with the same id: stopping...");
    }

    // estimate delay and jitter in milliseconds
    delaySum += packetDelay;
    avDelay = numAckPackets > 0 ? delaySum / double(numAckPackets) : 0.0;
    // wait for 10 packets to have more or less realistic minmax values
    maxDelay = numRxPackets > 10 ? std::max(maxDelay, packetDelay) : std::numeric_limits<double>::min();
    minDelay = numRxPackets > 10 ? std::min(minDelay, packetDelay) : std::numeric_limits<double>::max();
    if (numAckPackets > 1) {
        delayJitter = fabs(packetDelay - delay);
        delayJitterSum += delayJitter;
        avDelayJitter = delayJitterSum / double(numAckPackets - 1);
        delay = packetDelay;
    }

    // rtt in milliseconds
    double rttEstimated = (1.0 - RTT_ALPHA) * rtt + RTT_ALPHA * packetRtt;
    rttSum += rttEstimated;
    avRtt = numAckPackets > 0 ? rttSum / double(numAckPackets) : 0.0;
    // wait for 10 packets to have more or less realistic minmax values
    maxRtt = numRxPackets > 10 ? std::max(maxRtt, rttEstimated) : std::numeric_limits<double>::min();
    minRtt = numRxPackets > 10 ? std::min(minRtt, rttEstimated) : std::numeric_limits<double>::max();
    if (numAckPackets > 1) {
        rttJitter = fabs(rttEstimated - rtt);
        rttJitterSum += rttJitter;
        avRttJitter = rttJitterSum / double(numAckPackets - 1);
        rtt = rttEstimated;
    }

    // goodput in mbit/s
    // payload in bytes -> *8 / 1000 / 1000 to megabits
    // duration in seconds ->  .
    avRxGoodput = rxDuration > 0.0 ? double(numRxBytes) * 8.0 / (rxDuration * 1000 * 1000) : 0.0;
    // wait for some packets to have more or less realistic minmax values
    maxRxGoodput = numRxPackets > 25 ? std::max(maxRxGoodput, avRxGoodput) : std::numeric_limits<double>::min();
    minRxGoodput = numRxPackets > 25 ? std::min(minRxGoodput, avRxGoodput) : std::numeric_limits<double>::max();

    // always print?
    if (verbose > 1)
        print();
}

void DataTransmissionState::onNewNackPacket(const inet::Ptr<const inet::Chunk>& packet) {
    numNackPackets ++;
}

void DataTransmissionState::onNewDataPacket(const inet::Ptr<const inet::Chunk>& packet) {
    inet::Ptr<const ShipPacket> dataPacket = inet::dynamicPtrCast<const ShipPacket>(packet);

    // update general
    numTxPackets ++;
    numTxBytes += dataPacket->getPayloadSize();

    // time
    if (numTxPackets == 1) {
        // first rx packet, save time
        timeFirstPacketSent = dataPacket->getPayloadTimestamp();
    }
    timeCurrentPacketSent = dataPacket->getPayloadTimestamp();
    txDuration = (timeCurrentPacketSent - timeFirstPacketSent).dbl();

    // tx goodput in mbit/s
    avTxGoodput = txDuration > 0.0 ? double(numTxBytes) * 8.0 / (txDuration * 1000 * 1000) : 0.0;
    // wait 1 second for enough packets sent
    if (txDuration >= 1.0) {
        maxTxGoodput = std::max(maxTxGoodput, avTxGoodput);
        minTxGoodput = std::min(minTxGoodput, avTxGoodput);
    }
}

void DataTransmissionState::onNewFecPacket(const inet::Ptr<const inet::Chunk>& packet) {
    numFecPackets ++;
    numFecBytes += inet::B(packet->getChunkLength()).get();
}

void DataTransmissionState::onNewReceiverReportPacket(const inet::Ptr<const inet::Chunk>& packet, int twNumber) {
    inet::Ptr<const ReportBlock> reportBlock = inet::dynamicPtrCast<const ReportBlock>(packet);

    // update packet counters
    if (twNumber > lastReceiverReportId) {
        lastReceiverReportId = twNumber;
        numReceiverReportPackets++;
    }
    numLostPackets = reportBlock->getPacketsLostCumulative();
    numRepairedPackets = reportBlock->getPacketsRepairedCumulative();
    numRxPackets = reportBlock->getPacketsReceivedCumulative();
    lossRate = static_cast<double>(numLostPackets) / (numRxPackets + numLostPackets);
    numOutOfOrderPackets = reportBlock->getPacketsOutOfOrderCumulative();
    outOfOrderRate = static_cast<double>(numOutOfOrderPackets) / numRxPackets;
    numPlayedPackets = reportBlock->getPacketsPlayedCumulative();
    playRate = static_cast<double>(numPlayedPackets) / (numRxPackets + numRepairedPackets);
    repairRate = static_cast<double>(numRepairedPackets) / (numRxPackets + numRepairedPackets);
    numRetransmittedPackets = reportBlock->getPacketsRetransmittedCumulative();
    retransmissionRate = static_cast<double>(numRetransmittedPackets) / numRxPackets;
    successNackRate = static_cast<double>(numRetransmittedPackets) / numNackPackets;
    numRepairedAndRetransmittedPackets = reportBlock->getPacketsRepairedAndRetransmittedCumulative();
    numRxBytes = reportBlock->getBytesReceivedCumulative();

    // update rates
    bandwidth = reportBlock->getBandwidth();
    rxGoodput = reportBlock->getFractionRate();
    minRxGoodput = std::min(minRxGoodput, rxGoodput);
    maxRxGoodput = std::max(maxRxGoodput, rxGoodput);
    rxFecGoodput = reportBlock->getFractionFecRate();
    minRxFecGoodput = std::min(minRxFecGoodput, rxFecGoodput);
    maxRxFecGoodput = std::max(maxRxFecGoodput, rxFecGoodput);

    // update RFC-defined fraction lost
    fractionLossRate = reportBlock->getFractionLost();

    // update RFC-defined jitter
    interarrivalJitter = reportBlock->getJitter() * 1000;

    // update retransmission and playout delays
    fractionAvInterarrivalDelay = !std::isnan(reportBlock->getFractionInterarrivalDelayMean())
                                  ? reportBlock->getFractionInterarrivalDelayMean() * 1000 : 0.0;
    fractionAvTransmissionDelay = !std::isnan(reportBlock->getFractionTransmissionDelayMean())
                                  ? reportBlock->getFractionTransmissionDelayMean() * 1000 : 0.0;
    fractionAvRetransmissionDelay = !std::isnan(reportBlock->getFractionRetransmissionDelayMean())
                                    ? reportBlock->getFractionRetransmissionDelayMean() * 1000 : 0.0;
    fractionAvPlayoutDelay = !std::isnan(reportBlock->getFractionPlayoutDelayMean())
                             ? reportBlock->getFractionPlayoutDelayMean() * 1000 : 0.0;
    fractionStallingRate = reportBlock->getFractionStallingRate();

    // update transmission delay
    delaySum += fractionAvTransmissionDelay;
    avDelay = numAckPackets > 0 ? delaySum / double(numAckPackets) : 0.0;
    // wait for 10 packets to have more or less realistic minmax values
    maxDelay = std::max(maxDelay, fractionAvTransmissionDelay);
    minDelay = std::min(minDelay, fractionAvTransmissionDelay);
    if (numAckPackets > 1) {
        delayJitter = fabs(fractionAvTransmissionDelay - delay);
        delayJitterSum += delayJitter;
        avDelayJitter = delayJitterSum / double(numAckPackets - 1);
    }
    delay = fractionAvTransmissionDelay;

    // update rtt
    double fractionRtt;
    bool isDirect = reportBlock->getLastSR().dbl() == 0.0 && reportBlock->getDelaySinceLastSR().dbl() == 0.0;
    if (isDirect) {
        // sample RTT by twice of fractionAvTransmissionDelay (already in ms)
        double rttSample = fractionAvTransmissionDelay * 2.0;
        if (rtt > 0.0) {
            fractionRtt = (1.0 - RTT_ALPHA) * rtt + RTT_ALPHA * rttSample;
        } else {
            fractionRtt = rttSample;
        }
    } else {
        // sample RTT as in real RR reports: rttSample = now - last_sr - delay_since_last_sr
        omnetpp::simtime_t rttSampleSimtime =
            omnetpp::simTime() - reportBlock->getLastSR() - reportBlock->getDelaySinceLastSR();
        double rttSample = rttSampleSimtime.dbl() * 1000;
        if (rtt > 0.0) {
            fractionRtt = (1.0 - RTT_ALPHA) * rtt + RTT_ALPHA * rttSample;
        } else {
            fractionRtt = rttSample;
        }
    }
    rttSum += fractionRtt;
    avRtt = numReceiverReportPackets > 0 ? rttSum / double(numReceiverReportPackets) : 0.0;
    maxRtt = std::max(maxRtt, fractionRtt);
    minRtt = std::min(minRtt, fractionRtt);
    if (numReceiverReportPackets > 1) {
        rttJitter = fabs(fractionRtt - rtt);
        rttJitterSum += rttJitter;
        avRttJitter = rttJitterSum / double(numReceiverReportPackets - 1);
    }
    rtt = fractionRtt;

    // update pli: simulate it very roughly based on fractionLossRate
    numPliPackets += std::floor(fractionLossRate * 8.67 + 0.1);

    // update timestamp
    timeCurrentReceiverReport = omnetpp::simTime();

    if (verbose > 1)
        print();

}

void DataTransmissionState::onNewTransportFeedbackPacket(const inet::Ptr<const inet::Chunk>& packet, int twNumber) {
    inet::Ptr<const TransportFeedbackChunk> transportFeedbackChunk =
        inet::dynamicPtrCast<const TransportFeedbackChunk>(packet);

    // increment new ack packets
    if (twNumber > lastTransportFeedbackId) {
        lastTransportFeedbackId = twNumber;
        numTransportFeedbacks++;
    }
    numAckPackets++;

    // push packet
    packets.emplace_back(twNumber, transportFeedbackChunk->getSequenceNumber());

    double interarrivalDelta = transportFeedbackChunk->getRecvDelta().dbl() * 1000;
    deltas.emplace_back(twNumber, interarrivalDelta);
    interarrivalDelaySum += interarrivalDelta;
    avInterarrivalDelay = numAckPackets > 0 ? interarrivalDelaySum / double(numAckPackets) : 0.0;

    // update timestamp
    timeCurrentTransportFeedback = omnetpp::simTime();
}

void DataTransmissionState::checkNetworkInstability() {
    // no instability, first registered
    if (numAckPackets > 0 && !isInstabilityNow
            && ((omnetpp::simTime() - timeCurrentPacketAck).dbl() * 1000 >= instableAfterNoReplyMs)) {
        isInstabilityNow = true;
        timeInstabilityStarts = omnetpp::simTime();
        // instability is registered, but still unproceeded because no packet difference info has come
    } else if (numAckPackets > 0 && isInstabilityNow
               && ((omnetpp::simTime() - timeCurrentPacketAck).dbl() * 1000 >= instableAfterNoReplyMs)) {
        instabilityDuration += (omnetpp::simTime() - timeInstabilityStarts).dbl();
        instabilityRate = instabilityDuration / (omnetpp::simTime() - timeFirstPacketAck).dbl();
        timeInstabilityStarts = omnetpp::simTime();
        // instability is over, rtt is further ok
    } else if (numAckPackets > 0 && isInstabilityNow
               && ((omnetpp::simTime() - timeCurrentPacketAck).dbl() * 1000 < instableAfterNoReplyMs)) {
        isInstabilityNow = false;
        instabilityDuration += (omnetpp::simTime() - timeInstabilityStarts).dbl();
        instabilityRate = instabilityDuration / (omnetpp::simTime() - timeFirstPacketAck).dbl();
        timeInstabilityStarts = 0;
    }
}

nlohmann::json DataTransmissionState::toJson() {
    nlohmann::json dtsJson;
    dtsJson["numTxPackets"] = numTxPackets;
    dtsJson["numTxBytes"] = numTxBytes;
    dtsJson["numRxPackets"] = numRxPackets;
    dtsJson["rxGoodput"] = rxGoodput;
    dtsJson["rxFecGoodput"] = rxFecGoodput;

    dtsJson["numOutOfOrderPackets"] = numOutOfOrderPackets;
    dtsJson["numLostPackets"] = numLostPackets;
    dtsJson["lossRate"] = lossRate;
    dtsJson["fractionLossRate"] = fractionLossRate;
    dtsJson["numPlayedPackets"] = numPlayedPackets;
    dtsJson["numRepairedPackets"] = numRepairedPackets;
    dtsJson["numRetransmittedPackets"] = numRetransmittedPackets;
    dtsJson["numRepairedAndRetransmittedPackets"] = numRepairedAndRetransmittedPackets;

    dtsJson["numAckPackets"] = numAckPackets;
    dtsJson["numNackPackets"] = numNackPackets;
    dtsJson["numPliPackets"] = numPliPackets;
    dtsJson["numFecPackets"] = numFecPackets;
    dtsJson["numFecBytes"] = numFecBytes;

    dtsJson["interarrivalJitter"] = interarrivalJitter;
    dtsJson["fractionAvTransmissionDelay"] = fractionAvTransmissionDelay;
    dtsJson["fractionAvRetransmissionDelay"] = fractionAvRetransmissionDelay;
    dtsJson["fractionAvPlayoutDelay"] = fractionAvPlayoutDelay;
    dtsJson["fractionStallingRate"] = fractionStallingRate;
    dtsJson["fractionRtt"] = rtt;
    dtsJson["rttJitter"] = rttJitter;

    // serialize vectors and erase them
    nlohmann::json deltasJson = nlohmann::json::array();
    for (const auto& deltaPair : deltas) {
        nlohmann::json pairJson;
        pairJson["tfnum"] = deltaPair.first;
        pairJson["delay"] = deltaPair.second;
        deltasJson.push_back(pairJson);
    }
    deltas.clear();
    packets.clear();
    dtsJson["deltas"] = deltasJson;

    dtsJson["numReceiverReportPackets"] = numReceiverReportPackets;
    dtsJson["numTransportFeedbacks"] = numTransportFeedbacks;
    dtsJson["lastReceiverReportId"] = lastReceiverReportId;
    dtsJson["lastTransportFeedbackId"] = lastTransportFeedbackId;
    dtsJson["timeCurrentReceiverReport"] = timeCurrentReceiverReport.dbl();
    dtsJson["timeCurrentTransportFeedback"] = timeCurrentTransportFeedback.dbl();
    return dtsJson;
}

void DataTransmissionState::print(std::ostream& os) {
    os << "--------------------------------------------------------" << std::endl;
    os << "\t" << "Transmission state " << std::endl;
    os << "--------------------------------------------------------" << std::endl;
    os << "Time passed (sec) " << txDuration << std::endl;
    os << "Rcvd: pckts " << numRxPackets << "   bytes: " << numRxBytes << std::endl;
    os << "Sent: pckts " << numTxPackets << "   bytes: " << numTxBytes << std::endl;
    os << "Lost: pckts " << numLostPackets << "    rate (%): "  << lossRate * 100 << std::endl;
    os << "Ooo: pckts " << numOutOfOrderPackets << "  rate (%): "  << outOfOrderRate * 100 << std::endl;
    os << "Played: pckts " << numPlayedPackets << "   rate (%): " << playRate * 100 << " per avg playout delay (ms) "
       << fractionAvPlayoutDelay << std::endl;
    os << "Repaired: pckts " << numRepairedPackets << "  rate (%): "  << repairRate * 100 << std::endl;
    os << "Retransmitted: pckts " << numRetransmittedPackets << "  rate (%): "  << retransmissionRate * 100
       << " succes NACK rate (%): "  << successNackRate * 100 << " per avg retr delay (ms) "
       << fractionAvRetransmissionDelay << std::endl;
    os << "Repaired AND retransmitted: pckts " << numRepairedAndRetransmittedPackets << std::endl;
    os << "Delay current (ms): " << delay << "   min/avg/max (ms): " << minDelay << " / " << avDelay << " / "
       << maxDelay << std::endl;
    os << "Delay Jitter current (ms): " << delayJitter << "   avg (ms) " << avDelayJitter << std::endl;
    os << "Rtt current (ms): " << rtt << "   min/avg/max (ms): " << minRtt << " / " << avRtt << " / "
       << maxRtt << std::endl;
    os << "Rx Goodput curent (Mbps): " << rxGoodput << "    /min/max " << minRxGoodput << " / " << maxRxGoodput
       << std::endl;
    os << "Rx FEC Goodput current (Mbps): " << rxFecGoodput << "    /min/max " << minRxFecGoodput << " / " <<
       maxRxFecGoodput << std::endl;
    os << "Instability duration: (sec)  " << instabilityDuration << "    rate (%): " << instabilityRate
       << std::endl;
    os << "--------------------------------------------------------" << std::endl;
}

void DataTransmissionState::print() {
    print(std::cout);
}


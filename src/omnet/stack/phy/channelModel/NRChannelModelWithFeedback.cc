#include "NRChannelModelWithFeedback.h"

Define_Module(NRChannelModelWithFeedback);

void NRChannelModelWithFeedback::initialize(int stage) {
    NRChannelModel_3GPP38_901::initialize(stage);

    signalFeedback_ = new SignalFeedback();
}

void NRChannelModelWithFeedback::finish() {
    NRChannelModel_3GPP38_901::finish();
    if (signalFeedback_) delete signalFeedback_;
}

bool NRChannelModelWithFeedback::isError(LteAirFrame *frame, UserControlInfo* lteInfo) {
    EV << "NRChannelModelWithFeedback::error" << endl;

    //get codeword
    unsigned char cw = lteInfo->getCw();
    //get number of codeword
    int size = lteInfo->getUserTxParams()->readCqiVector().size();

    //get position associated to the packet
    // Coord coord = lteInfo->getCoord();

    //if total number of codeword is equal to 1 the cw index should be only 0
    if (size == 1)
        cw = 0;

    //get cqi used to transmit this cw
    Cqi cqi = lteInfo->getUserTxParams()->readCqiVector()[cw];

    MacNodeId id;
    Direction dir = (Direction) lteInfo->getDirection();

    //Get MacNodeId of UE
    if (dir == DL)
        id = lteInfo->getDestId();
    else
        id = lteInfo->getSourceId();

    // Get Number of RTX
    unsigned char nTx = lteInfo->getTxNumber();

    //consistency check
    if (nTx == 0)
        throw cRuntimeError("Transmissions counter should not be 0");

    //Get txmode
    TxMode txmode = (TxMode) lteInfo->getTxMode();

    // If rank is 1 and we used SMUX to transmit we have to corrupt this packet
    if (txmode == CL_SPATIAL_MULTIPLEXING
            || txmode == OL_SPATIAL_MULTIPLEXING)
    {
        //compare lambda min (smaller eingenvalues of channel matrix) with the threshold used to compute the rank
        if (binder_->phyPisaData.getLambda(id, 1) < lambdaMinTh_)
            return false;
    }

    // Take sinr and rsrp
    std::vector<double> snrV;
    std::vector<double> rsrpV; // NOTE: add rsrp
    if (lteInfo->getDirection() == D2D || lteInfo->getDirection() == D2D_MULTI)
    {
        MacNodeId destId = lteInfo->getDestId();
        Coord destCoord = phy_->getCoord();
        MacNodeId enbId = binder_->getNextHop(lteInfo->getSourceId());
        snrV = getSINR_D2D(frame,lteInfo,destId,destCoord,enbId);
        rsrpV = getRSRP_D2D(frame,lteInfo,destId,destCoord); // NOTE: add rsrp
    }
    else
    {
        snrV = getSINR(frame, lteInfo);
        rsrpV = getRSRP(frame, lteInfo); // NOTE: add rsrp
    }

    //Get the resource Block id used to transmist this packet
    RbMap rbmap = lteInfo->getGrantedBlocks();

    //Get txmode
    unsigned int itxmode = txModeToIndex[txmode];

    double bler = 0;
    std::vector<double> totalbler;
    double finalSuccess = 1;
    RbMap::iterator it;
    std::map<Band, unsigned int>::iterator jt;

    // for statistic purposes
    double sumSnr = 0.0;
    double sumRsrp = 0.0; // NOTE: add rsrp
    int usedRBs = 0;

    //for each Remote unit used to transmit the packet
    for (it = rbmap.begin(); it != rbmap.end(); ++it)
    {
        //for each logical band used to transmit the packet
        for (jt = it->second.begin(); jt != it->second.end(); ++jt)
        {
            //this Rb is not allocated
            if (jt->second == 0)
                continue;

            //check the antenna used in Das
            if ((lteInfo->getTxMode() == CL_SPATIAL_MULTIPLEXING
                    || lteInfo->getTxMode() == OL_SPATIAL_MULTIPLEXING)
                    && rbmap.size() > 1)
                //we consider only the snr associated to the LB used
                if (it->first != lteInfo->getCw())
                    continue;

            //Get the Bler
            if (cqi == 0 || cqi > 15)
                throw cRuntimeError("A packet has been transmitted with a cqi equal to 0 or greater than 15 cqi:%d txmode:%d dir:%d rb:%d cw:%d rtx:%d", cqi,lteInfo->getTxMode(),dir,jt->second,cw,nTx);

            // for statistic purposes
            sumSnr += snrV[jt->first];
            sumRsrp += rsrpV[jt->first]; // NOTE: add rsrp
            usedRBs++;

            int snr = snrV[jt->first];//XXX because jt->first is a Band (=unsigned short)
            if (snr < binder_->phyPisaData.minSnr())
                return false;
            else if (snr > binder_->phyPisaData.maxSnr())
                bler = 0;
            else
                bler = binder_->phyPisaData.getBler(itxmode, cqi - 1, snr);

            EV << "\t bler computation: [itxMode=" << itxmode << "] - [cqi-1=" << cqi-1
                    << "] - [snr=" << snr << "]" << endl;

            double success = 1 - bler;
            //compute the success probability according to the number of RB used
            double successPacket = pow(success, (double)jt->second);
            // compute the success probability according to the number of LB used
            finalSuccess *= successPacket;

            EV << " LteRealisticChannelModel::error direction " << dirToA(dir)
                                << " node " << id << " remote unit " << dasToA((*it).first)
                                << " Band " << (*jt).first << " SNR " << snr << " CQI " << cqi
                                << " BLER " << bler << " success probability " << successPacket
                                << " total success probability " << finalSuccess << endl;
        }
    }
    //Compute total error probability
    double per = 1 - finalSuccess;
    //Harq Reduction
    double totalPer = per * pow(harqReduction_, nTx - 1);

    double er = uniform(0.0, 1.0);

    EV << " LteRealisticChannelModel::error direction " << dirToA(dir)
                        << " node " << id << " total ERROR probability  " << per
                        << " per with H-ARQ error reduction " << totalPer
                        << " - CQI[" << cqi << "]- random error extracted[" << er << "]" << endl;

    // emit SINR statistic
    if (collectSinrStatistics_ && usedRBs > 0)
    {
        if (dir == DL) // we are on the UE
            emit(rcvdSinrDl_, sumSnr / usedRBs);
        else
        {
            // we are on the BS, so we need to retrieve the channel model of the sender
            // XXX I know, there might be a faster way...
            LteChannelModel* ueChannelModel = check_and_cast<LtePhyUe*>(getPhyByMacNodeId(id))->getChannelModel(lteInfo->getCarrierFrequency());
            ueChannelModel->emit(rcvdSinrUl_, sumSnr / usedRBs);
        }
    }

    // NOTE: send signal values
    if (usedRBs > 0) {
        signalFeedback_->isNew = true;
        signalFeedback_->direction = (int)dir;
        signalFeedback_->carrierFrequency = lteInfo->getCarrierFrequency();
        signalFeedback_->numUsedRbs = usedRBs;
        signalFeedback_->sinr = sumSnr / usedRBs;
        signalFeedback_->rsrp = sumRsrp / usedRBs;
        signalFeedback_->bler = bler;
        signalFeedback_->harqper = totalPer;
    }

    if (er <= totalPer)
    {
        EV << "This is NOT your lucky day (" << er << " < " << totalPer
                << ") -> do not receive." << endl;

        // Signal too weak, we can't receive it
        return false;
    }
    // Signal is strong enough, receive this Signal
    EV << "This is your lucky day (" << er << " > " << totalPer
            << ") -> Receive AirFrame." << endl;

    return true;
}

SignalFeedback* NRChannelModelWithFeedback::getSignalFeedback() { return signalFeedback_; }

void NRChannelModelWithFeedback::ackSignalFeedback() { signalFeedback_->isNew = false; }
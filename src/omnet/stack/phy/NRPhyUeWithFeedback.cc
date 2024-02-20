#include "NRPhyUeWithFeedback.h"

Define_Module(NRPhyUeWithFeedback);

NRPhyUeWithFeedback::NRPhyUeWithFeedback() {
    handoverStarter_ = nullptr;
    handoverTrigger_ = nullptr;
    logUpdateTimer = nullptr;
}

NRPhyUeWithFeedback::~NRPhyUeWithFeedback() {
    cancelAndDelete(logUpdateTimer);
}

void NRPhyUeWithFeedback::initialize(int stage) {
    NRPhyUe::initialize(stage);
    feedbackStartTime = par("feedbackStartTime").doubleValue();
    logUpdatePeriod = par("logUpdatePeriod").doubleValue();
    verbose = par("verbose").intValue();
    if (logUpdatePeriod > 0) {
        if (verbose == 0) verbose = 1;
        logUpdateTimer = new cMessage("logUpdate");
        scheduleAt(simTime() + feedbackStartTime, logUpdateTimer);
    }
}

void NRPhyUeWithFeedback::handleMessage(cMessage* msg) {
    if (!strcmp(msg->getName(), "logUpdate")) {
        logPhyDataState();
    } else {
        NRPhyUe::handleMessage(msg);
    }
}

void NRPhyUeWithFeedback::handleUpperMessage(cMessage* msg) {
    cMessage* lteMsg = msg->dup();
    NRPhyUe::handleUpperMessage(msg);

    if (simTime() - feedbackStartTime >= 0) {
        auto lteInfo = check_and_cast<inet::Packet*>(lteMsg)->removeTag<UserControlInfo>();
        double carrierFrequency = lteInfo->getCarrierFrequency();
        if (carrierFrequency > 0 && phyDataStatesMap_.find(carrierFrequency) == phyDataStatesMap_.end()) {
            phyDataStatesMap_.insert(std::make_pair(carrierFrequency, PhyDataState(carrierFrequency, verbose)));
        }
        if (lteInfo->getFrameType() == DATAPKT && lteInfo->getUserTxParams()) {
            phyDataStatesMap_[carrierFrequency].updateWithUlData(packPhyData(lteInfo.get(), nullptr));
        }
        lteInfo.reset();
        delete lteMsg;
    }
}

void NRPhyUeWithFeedback::handleAirFrame(cMessage* msg) {
    cMessage* lteMsg = msg->dup();
    NRPhyUe::handleAirFrame(msg);

    if (simTime() - feedbackStartTime >= 0) {
        auto lteInfo = check_and_cast<UserControlInfo*>(lteMsg->removeControlInfo());
        double carrierFrequency = lteInfo->getCarrierFrequency();
        if (carrierFrequency > 0 && phyDataStatesMap_.find(carrierFrequency) == phyDataStatesMap_.end()) {
            phyDataStatesMap_.insert(std::make_pair(carrierFrequency, PhyDataState(carrierFrequency, verbose)));
        }
        if (lteInfo->getFrameType() == DATAPKT && lteInfo->getUserTxParams()) {
            auto frame = check_and_cast<LteAirFrame*>(lteMsg);
            phyDataStatesMap_[carrierFrequency].updateWithDlData(packPhyData(lteInfo, frame));
            delete frame;
        }
        delete lteInfo;
    }
}

std::unique_ptr<PhyData> NRPhyUeWithFeedback::packPhyData(UserControlInfo* lteInfo, LteAirFrame* frame) {
    std::unique_ptr<PhyData> data = std::make_unique<PhyData>();

    // general parameters
    Direction direction = (Direction)lteInfo->getDirection();
    LtePhyFrameType frameType = (LtePhyFrameType)lteInfo->getFrameType();
    data->direction = dirToA(direction);
    data->frameType = phyFrameTypeToA(frameType);
    data->carrierFrequency = lteInfo->getCarrierFrequency();

    // received sinr and rsrq from the UE channel model
    // NOTE: can be extended to UL as well if gNodeB's channel model is called. But this is unrealistic from UE's perspective.
    NRChannelModelWithFeedback* channelModel =
        check_and_cast<NRChannelModelWithFeedback*>(getChannelModel(data->carrierFrequency));
    data->numBands = channelModel->getNumBands();
    SignalFeedback* signalFeedback = channelModel->getSignalFeedback();
    if ((Direction)signalFeedback->direction == DL && signalFeedback->isNew) {
        data->numUsedRbs = signalFeedback->numUsedRbs;
        data->sinr = signalFeedback->sinr;
        data->rsrp = signalFeedback->rsrp;
        data->bler = signalFeedback->bler;
        data->harqper = signalFeedback->harqper;
        // https://arimas.com/2017/11/06/lte-rsrp-rsrq-rssi-calculator/
        // RSRP = RSSI - 10 * log(12 * N)
        // RSRQ = N * RSRP / RSSI
        data->rssi = data->rsrp + 10 * std::log10(12 * data->numBands);
        data->rsrq = data->rsrp * data->numBands  / data->rssi;
        channelModel->ackSignalFeedback();
    } else {
        data->numUsedRbs = 0;
    }

    // user tx DL/UL params
    data->cw = lteInfo->getUserTxParams()->readCqiVector().size() == 1 ? 0 : lteInfo->getCw();
    data->cqi = (int)lteInfo->getUserTxParams()->readCqiVector()[data->cw];
    data->ri = lteInfo->getUserTxParams()->readRank();
    data->pmi = lteInfo->getUserTxParams()->readPmi();
    data->modulation = modToA(lteInfo->getUserTxParams()->getCwModulation(data->cw));

    // enb info & coords
    if (direction == DL) {
        data->enbId = lteInfo->getSourceId();
        data->enbName = binder_->getModuleNameByMacNodeId(data->enbId);
        data->isNr = binder_->getBaseStationTypeById(data->enbId) == GNODEB;
        data->cellId = lteInfo->getSourceId();
        data->enbCoord = lteInfo->getCoord();
        data->ueCoord = getRadioPosition();
    }

    data->time = simTime();
    return data;
}

void NRPhyUeWithFeedback::logPhyDataState() {
    // dirty hack to surpress the exception because of the heap ownership of a message (this class is a derived one
    cancelEvent(logUpdateTimer);
    for (auto& it : phyDataStatesMap_) {
        it.second.log(simTime().dbl());
    }
    scheduleAt(simTime() + logUpdatePeriod, logUpdateTimer);
}

const std::unordered_map<double, PhyDataState>& NRPhyUeWithFeedback::getPhyDataStateMap() { return phyDataStatesMap_; }

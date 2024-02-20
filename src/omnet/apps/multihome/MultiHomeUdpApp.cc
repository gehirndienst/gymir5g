#include "MultiHomeUdpApp.h"

Define_Module(MultiHomeUdpApp);

simsignal_t MultiHomeUdpApp::ltePacketsSent = registerSignal("ltePacketsSent");
simsignal_t MultiHomeUdpApp::nrPacketsSent = registerSignal("nrPacketsSent");
simsignal_t MultiHomeUdpApp::wlanPacketsSent = registerSignal("wlanPacketsSent");
simsignal_t MultiHomeUdpApp::dontcarePacketsSent = registerSignal("dontcarePacketsSent");

MultiHomeUdpApp::MultiHomeUdpApp()
    : sendDataMsg(nullptr)
    , dontCareDist(std::uniform_int_distribution<int>(0, 100)) {}

MultiHomeUdpApp::~MultiHomeUdpApp() {
    cancelAndDelete(sendDataMsg);
}

void MultiHomeUdpApp::initialize(int stage) {
    cSimpleModule::initialize(stage);
    if (stage != INITSTAGE_APPLICATION_LAYER)
        return;

    initTime = simTime();
    maxWaitingTimeToStart = par("maxWaitingTimeToStart").doubleValue();

    isNR = par("isNR").boolValue();

    packetSize = par("packetSize");
    sendingPeriod = par("sendingPeriod");

    sendDataMsg = new cMessage("sendData");
    isScheduleAfterAppsAreFound = par("isScheduleAfterAppsAreFound").boolValue();
    packetsSent = 0;

    // find cellular and wireless apps
    findMultiHomeApps();
}

void MultiHomeUdpApp::findMultiHomeApps() {
    // find cellular app which module could be dynamic
    cellularApp = findModuleByPath(par("cellularAppPath"));
    if (!cellularApp) {
        // cellular module can be dynamic, e.g., simu5g.nodes.cars.NRCar
        if ((simTime() - initTime).dbl() > maxWaitingTimeToStart) {
            throw cRuntimeError("MultiHomeUdpApp::findMultiHomeApps: can't find cellular app, aborting...");
        }
        double offset = uniform(0.01, 0.05);
        scheduleAt(simTime() + offset, new cMessage("findApps"));
    } else {
        // if cellular app is found then proceed with wireless (it should not be dynamic) and start data sending
        std::cout << "MultiHomeApp::findMultiHomeApps found cellular app! proceeding..."
                  << std::endl;
        wirelessApp = findModuleByPath(par("wirelessAppPath"));
        if (!wirelessApp) {
            throw cRuntimeError("MultiHomeUdpApp::findMultiHomeApps: can't find wireless app, aborting...");
        }
        std::cout << "MultiHomeApp::findMultiHomeApps found wireless app! proceeding..."
                  << std::endl;
        if (isScheduleAfterAppsAreFound) {
            scheduleAt(simTime(), sendDataMsg);
        }
        std::cout << "MultiHomeUdpApp::findMultiHomeApps all apps are found, start sending the data..."
                  << std::endl;
    }
}

void MultiHomeUdpApp::handleMessage(cMessage* msg) {
    if (msg->isSelfMessage()) {
        if (!strcmp(msg->getName(), "findApps")) {
            delete msg;
            findMultiHomeApps();
        } else if (!strcmp(msg->getName(), "sendData")) {
            sendData();
        }
    } else {
        // NOTE: here data should arrive only via sendDirect from cellular or wireless stack
        if (msg->getArrivalGate() == gate("directIn")) {
            receiveData(msg);
        }
    }
}

void MultiHomeUdpApp::sendData() {
    MultiHomeMessage* msg = new MultiHomeMessage();
    msg->setId(++packetsSent);
    msg->setSendingTime(simTime());
    msg->setPayloadSize(packetSize);

    NetworkType nt = getNetworkType();
    msg->setNetworkType(static_cast<int>(nt));

    if (msg->getNetworkType() < 2) {
        // cellular
        sendDirect(msg, cellularApp, cellularApp->findGate("dataIn"));
    } else {
        // wireless
        sendDirect(msg, wirelessApp, wirelessApp->findGate("dataIn"));
    }

    scheduleAt(simTime() + sendingPeriod, sendDataMsg);
}

void MultiHomeUdpApp::receiveData(cMessage* msg) {
    // override if needed
    delete msg;
}

NetworkType MultiHomeUdpApp::getNetworkType() {
    // override if needed
    auto randomEngine = std::default_random_engine{std::random_device{}()};
    bool isDontCare = dontCareDist(randomEngine) == 0;
    NetworkType nt;
    if (isDontCare) {
        std::uniform_int_distribution<int> dis(0, 2);
        nt = static_cast<NetworkType>(dis(randomEngine));
        emit(dontcarePacketsSent, 1);
    } else {
        if (packetsSent % 2 == 0) {
            // cellular
            nt = isNR ? NetworkType::NR : NetworkType::LTE;
            isNR ? emit(nrPacketsSent, 1) : emit(ltePacketsSent, 1);
        } else {
            // wlan
            nt = NetworkType::WIFI;
            emit(wlanPacketsSent, 1);
        }
    }
    return nt;
}

void MultiHomeUdpApp::finish() {
}

#include "AbrGcc.h"


void AbrGcc::applyAction(std::unique_ptr<Stream>& stream, double actionVal) {
    StreamActions::setRateEstimated(stream, actionVal);
}

nlohmann::json& AbrGcc::makeState(UnitState& state) {
    // get gcc estimated bandwidth delivered with a report from the receiver
    int bandwidthKbps;
    int bandwidthGccBps = state.getTransmissionState().bandwidth;
    if (bandwidthGccBps == 0) {
        bandwidthKbps = 400; // FIXME: make it a parameter
    } else {
        bandwidthKbps = bandwidthGccBps / 1000;
    }
    adaptiveStreamingState["streamName"] = state.getStreamState().streamName;
    adaptiveStreamingState["action"] = (double)bandwidthKbps;
    return adaptiveStreamingState;
}
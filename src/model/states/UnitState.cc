#include "UnitState.h"

UnitState::UnitState() {
    streamState_ = StreamState();
    transmissionState_ = DataTransmissionState();
}

void UnitState::set(const std::string& streamName, int historySize, int verbose) {
    streamState_.set(streamName, historySize);
    transmissionState_.verbose = verbose;
}

StreamState& UnitState::getStreamState() {
    return streamState_;
}

DataTransmissionState& UnitState::getTransmissionState() {
    return transmissionState_;
}
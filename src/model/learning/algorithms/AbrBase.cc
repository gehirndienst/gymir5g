#include "AbrBase.h"

int AbrBase::chooseDiscreteAction(std::unique_ptr<Stream>& stream) {
    // tpt kbits
    double tptPer = stream->state.getTransmissionState().rxGoodput / 1000;

    // qc fore previous 10 steps (could be tuned)
    int qualityChanges = stream->state.getStreamState().getQualityChanges(10);

    // select stream quality: a quality based on the recorded periodic tpt
    int quality = 0;
    if (stream->type == VIDEO) {
        quality = getVideoQualityByBitrate(tptPer);
    } else if (stream->type == LIDAR) {
        quality = getPointCloudQualityByBitrate(tptPer);
    } else {
        quality = stream->dataGen.getQualityPresetIndexByRate(tptPer);
    }

    // if there is no change of quality try to push a bit by 1 and see what happens
    if (qualityChanges == 0 && quality != stream->quality.maxQuality) {
        quality ++;
    }

    return quality;
}

void AbrBase::applyAction(std::unique_ptr<Stream>& stream, int actionNum) {
    StreamActions::setQuality(stream, actionNum);
}

void AbrBase::applyAction(std::unique_ptr<Stream>& stream, double actionVal) {
    StreamActions::setRate(stream, actionVal);
}
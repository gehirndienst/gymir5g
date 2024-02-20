#include "StreamActions.h"

void StreamActions::setQuality(std::unique_ptr<Stream>& stream, int newQuality) {
    std::unique_lock<std::mutex>(stream->streamMutex);
    StreamState& streamState = stream->state.getStreamState();
    if (newQuality >= stream->quality.minQuality && newQuality <= stream->quality.maxQuality) {
        if (!stream->isRunning) {
            stream->resumeStream();
        }
        stream->quality.currQuality = newQuality;
        streamState.isQualityChanged = true;
    } else {
        std::cerr << "[StreamActions::setQuality] stream " << stream->name << ": you try to set a quality " << newQuality <<
                  " not from current quality interval "
                  << stream->quality.minQuality << "--" << stream->quality.maxQuality << std::endl;
        throw std::invalid_argument("StreamActions::setQuality: wrong quality");
    }
    streamState.lastQualities.push_back(stream->quality.currQuality);
    streamState.lastAction = newQuality;
    streamState.actionsTaken++;
}

void StreamActions::setRate(std::unique_ptr<Stream>& stream, double relativeRate,
                            bool isApplyTenPercentPolicy) {
    // NOTE: Currently works only for SIM Streams
    if (stream->type != SIM) {
        throw std::invalid_argument("StreamActions::setRate works only for SIM streams");
    }
    if (relativeRate < -1 || relativeRate > 1) {
        throw std::invalid_argument("StreamActions::setRate relative rate should be within [-1, 1]");
    }
    std::unique_lock<std::mutex>(stream->streamMutex);

    StreamState& streamState = stream->state.getStreamState();
    double currentRate = stream->dataGen.getRate();
    double newRate = stream->dataGen.getAbsoluteRate(relativeRate);
    if (isApplyTenPercentPolicy) {
        if (newRate < 0.9 * currentRate || newRate > 1.1 * currentRate) {
            // a policy is to change the rate only if it differs more than 5% in any direction
            stream->dataGen.updateRate(newRate);
            streamState.isQualityChanged = true;
        } else {
            streamState.lastQualities.push_back(relativeRate);
            streamState.lastAction = relativeRate;
            streamState.actionsTaken++;
            return;
        }
    } else {
        stream->dataGen.updateRate(newRate);
        streamState.isQualityChanged = true;
    }
    streamState.lastQualities.push_back(relativeRate);
    streamState.lastAction = relativeRate;
    streamState.actionsTaken++;
}

void StreamActions::setRateEstimated(std::unique_ptr<Stream>& stream, double rateKbps,
                                     bool isApplyTenPercentPolicy) {
    // NOTE: Currently works only for SIM Streams and ABR_GCC adaptive algorithm
    if (stream->type != SIM) {
        throw std::invalid_argument("StreamActions::setRateEstimated works only for SIM streams");
    }
    if (rateKbps < 0) {
        std::cerr << "StreamActions::setRateEstimated rate should be positive, but is " << std::to_string(
                      rateKbps) << std::endl;
        std::clamp(rateKbps, stream->dataGen.getMinRate(), stream->dataGen.getMaxRate());
    }
    std::unique_lock<std::mutex>(stream->streamMutex);
    StreamState& streamState = stream->state.getStreamState();
    double currentRate = stream->dataGen.getRate();
    double newRate = rateKbps;
    if (isApplyTenPercentPolicy) {
        if (newRate < 0.9 * currentRate || newRate > 1.1 * currentRate) {
            // a policy is to change the rate only if it differs more than 5% in any direction
            stream->dataGen.updateRate(newRate);
            streamState.isQualityChanged = true;
        } else {
            streamState.lastQualities.push_back(newRate);
            streamState.lastAction = newRate;
            streamState.actionsTaken++;
            return;
        }
    } else {
        stream->dataGen.updateRate(newRate);
        streamState.isQualityChanged = true;
    }
    streamState.lastQualities.push_back(newRate);
    streamState.lastAction = newRate;
    streamState.actionsTaken++;
}

void StreamActions::turnStreamOff(std::unique_ptr<Stream>& stream) {
    std::unique_lock<std::mutex>(stream->streamMutex);
    if (stream->isRunning) {
        stream->pauseStream();
    }
    stream->state.getStreamState().actionsTaken ++;
}

void StreamActions::continueWithSameQuality(std::unique_ptr<Stream>& stream) {
    std::unique_lock<std::mutex>(stream->streamMutex);
    StreamState& streamState = stream->state.getStreamState();
    if (!stream->isRunning) {
        std::cerr << "[StreamActions::continueWithSameQuality] stream " << stream->name << " isn't running now" << std::endl;
        return;
    }
    streamState.lastQualities.push_back(stream->quality.currQuality);
    streamState.actionsTaken ++;
}

void StreamActions::upgradeQuality(std::unique_ptr<Stream>& stream) {
    std::unique_lock<std::mutex>(stream->streamMutex);
    StreamState& streamState = stream->state.getStreamState();
    if (!stream->isRunning) {
        std::cerr << "[StreamActions::upgradeQuality] stream "  << stream->name << " isn't running now" << std::endl;
        return;
    }

    stream->quality.currQuality ++;
    streamState.isQualityChanged = true;
    streamState.lastQualities.push_back(stream->quality.currQuality);
    streamState.actionsTaken ++;
}

void StreamActions::upgradeQualityMax(std::unique_ptr<Stream>& stream) {
    std::unique_lock<std::mutex>(stream->streamMutex);
    StreamState& streamState = stream->state.getStreamState();
    if (!stream->isRunning) {
        std::cerr << "[StreamActions::upgradeQualityMax] stream "  << stream->name << " isn't running now" << std::endl;
        return;
    }
    stream->quality.currQuality = stream->quality.maxQuality;
    streamState.isQualityChanged = true;
    streamState.lastQualities.push_back(stream->quality.currQuality);
    streamState.actionsTaken ++;
}

void StreamActions::downgradeQuality(std::unique_ptr<Stream>& stream) {
    std::unique_lock<std::mutex>(stream->streamMutex);
    StreamState& streamState = stream->state.getStreamState();
    if (!stream->isRunning) {
        std::cerr << "[StreamActions::downgradeQuality] stream "  << stream->name << " isn't running now" << std::endl;
        return;
    }
    stream->quality.currQuality --;
    streamState.isQualityChanged = true;
    streamState.lastQualities.push_back(stream->quality.currQuality);
    streamState.actionsTaken ++;
}

void StreamActions::downgradeQualityMax(std::unique_ptr<Stream>& stream) {
    std::unique_lock<std::mutex>(stream->streamMutex);
    StreamState& streamState = stream->state.getStreamState();
    if (!stream->isRunning) {
        std::cerr << "[StreamActions::downgradeQualityMax] stream "  << stream->name << " isn't running now" << std::endl;
        return;
    }
    stream->quality.currQuality = stream->quality.minQuality;
    streamState.isQualityChanged = true;
    streamState.lastQualities.push_back(stream->quality.currQuality);
    streamState.actionsTaken ++;
}

void StreamActions::continueWithSameEncodingSpeed(std::unique_ptr<Stream>& stream) {
    std::unique_lock<std::mutex>(stream->streamMutex);
    StreamState& streamState = stream->state.getStreamState();
    if (!stream->isRunning) {
        std::cerr << "[StreamActions::continueWithSameEncodingSpeed] stream " << stream->name << " isn't running now" <<
                  std::endl;
        return;
    }
    streamState.lastQualities.push_back(stream->quality.currQuality);
    streamState.actionsTaken ++;
}

void StreamActions::setFastEncodingSpeed(std::unique_ptr<Stream>& stream) {
    std::unique_lock<std::mutex>(stream->streamMutex);
    StreamState& streamState = stream->state.getStreamState();
    if (!stream->isRunning) {
        std::cerr << "[StreamActions::setFastEncodingSpeed] stream "  << stream->name << " isn't running now" << std::endl;
        return;
    }
    stream->quality.currEncodingSpeed = stream->quality.maxEncodingSpeed;
    streamState.isQualityChanged = true;
    streamState.lastQualities.push_back(stream->quality.currQuality);
    streamState.actionsTaken ++;
}

// FIXME: so far we only switch between fast and medium encoding speeds, don't see much sense in other options
void StreamActions::setNonFastEncodingSpeed(std::unique_ptr<Stream>& stream) {
    std::unique_lock<std::mutex>(stream->streamMutex);
    StreamState& streamState = stream->state.getStreamState();
    if (!stream->isRunning) {
        std::cerr << "[StreamActions::setNonFastEncodingSpeed] stream "  << stream->name << " isn't running now" << std::endl;
        return;
    }
    stream->quality.currEncodingSpeed = (stream->quality.maxEncodingSpeed - stream->quality.minEncodingSpeed) / 2;
    streamState.isQualityChanged = true;
    streamState.lastQualities.push_back(stream->quality.currQuality);
    streamState.actionsTaken ++;
}

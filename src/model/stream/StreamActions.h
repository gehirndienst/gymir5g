
/*
 * StreamActions.h
 *
 * all stream actions. The logic should be defined in Stream class
 *
 *  Created on: Aug 21, 2023
 *      Author: Nikita Smirnov
 */

#ifndef ADAPTIVESTREAMING_H
#define ADAPTIVESTREAMING_H

#include <vector>

#include "nlohmann/json.hpp"

#include "model/presets/PointCloudCompressionPresets.h"
#include "model/presets/VideoQualityPresets.h"
#include "model/stream/Stream.h"

class StreamActions {
public:
    static void setQuality(std::unique_ptr<Stream>& stream, int newQuality);
    static void setRate(std::unique_ptr<Stream>& stream, double relativeRate, bool isApplyTenPercentPolicy = false);
    static void setRateEstimated(std::unique_ptr<Stream>& stream, double rateKbps, bool isApplyTenPercentPolicy = false);
    // not used now
    static void turnStreamOff(std::unique_ptr<Stream>& stream);
    static void continueWithSameQuality(std::unique_ptr<Stream>& stream);
    static void upgradeQuality(std::unique_ptr<Stream>& stream);
    static void upgradeQualityMax(std::unique_ptr<Stream>& stream);
    static void downgradeQuality(std::unique_ptr<Stream>& stream);
    static void downgradeQualityMax(std::unique_ptr<Stream>& stream);
    static void continueWithSameEncodingSpeed(std::unique_ptr<Stream>& stream);
    static void setNonFastEncodingSpeed(std::unique_ptr<Stream>& stream);
    static void setFastEncodingSpeed(std::unique_ptr<Stream>& stream);
};

#endif


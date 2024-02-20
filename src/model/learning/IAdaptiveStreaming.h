/*
 * IAdaptiveStreaming.h
 *
 *  interface for adaptive streaming algorithms
 *
 *  Created on: Aug 21, 2023
 *      Author: Nikita Smirnov
 */

#ifndef IADAPTIVESTREAMING_H
#define IADAPTIVESTREAMING_H

#include <iostream>
#include <memory>
#include <string>

#include <boost/algorithm/string.hpp>
#include <nlohmann/json.hpp>
#include <omnetpp.h>

#include "model/states/UnitState.h"
#include "model/stream/Stream.h"
#include "model/stream/StreamActions.h"

class IAdaptiveStreaming {
public:
    virtual ~IAdaptiveStreaming() {};

    virtual nlohmann::json& makeState(UnitState& state) = 0;

    virtual int chooseDiscreteAction(std::unique_ptr<Stream>& stream) = 0;
    virtual double chooseContinuousAction(std::unique_ptr<Stream>& stream) = 0;

    virtual void applyAction(std::unique_ptr<Stream>& stream, int action) = 0;
    virtual void applyAction(std::unique_ptr<Stream>& stream, double action) = 0;

protected:
    nlohmann::json adaptiveStreamingState;
};

#endif
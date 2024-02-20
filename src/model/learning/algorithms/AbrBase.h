/*
 * Abrbase.h
 *
 * base ABR algorithm without prediction and with median decision
 *
 *  Created on: Jul 8, 2023
 *      Author: Nikita Smirnov
 */

#ifndef ABRBASE_H
#define ABRBASE_H

#include "model/learning/IAdaptiveStreaming.h"

class AbrBase : public IAdaptiveStreaming {
public:
    virtual nlohmann::json& makeState(UnitState& state) override { return adaptiveStreamingState; };

    virtual int chooseDiscreteAction(std::unique_ptr<Stream>& stream) override;
    virtual double chooseContinuousAction(std::unique_ptr<Stream>& stream) override { return 0.0; }

    virtual void applyAction(std::unique_ptr<Stream>& stream, int actionNum) override;
    virtual void applyAction(std::unique_ptr<Stream>& stream, double actionVal) override;
};

#endif
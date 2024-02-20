/*
 * Abrbase.h
 *
 * base DRL that expects int for a stream quality and sends only transmission state
 *
 *  Created on: Jul 8, 2023
 *      Author: Nikita Smirnov
 */

#include "model/learning/IAdaptiveStreaming.h"

class DrlBase : public IAdaptiveStreaming {
public:
    virtual nlohmann::json& makeState(UnitState& state) override;

    virtual void applyAction(std::unique_ptr<Stream>& stream, int actionNum) override;
    virtual void applyAction(std::unique_ptr<Stream>& stream, double actionVal) override;

    // delivered externally
    virtual int chooseDiscreteAction(std::unique_ptr<Stream>& stream) override { return 0; }
    virtual double chooseContinuousAction(std::unique_ptr<Stream>& stream) override { return 0.0; }
};
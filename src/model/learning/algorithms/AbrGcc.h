/*
 * Abrbase.h
 *
 * GCC estimator
 *
 *  Created on: Oct 10, 2023
 *      Author: Nikita Smirnov
 */

#ifndef ABRGCC_H
#define ABRGCC_H

#include "model/learning/algorithms/AbrBase.h"

class AbrGcc : public AbrBase {
public:
    virtual nlohmann::json& makeState(UnitState& state) override;

    virtual int chooseDiscreteAction(std::unique_ptr<Stream>& stream) override { return 0; };
    virtual double chooseContinuousAction(std::unique_ptr<Stream>& stream) override { return 0.0; };

    virtual void applyAction(std::unique_ptr<Stream>& stream, int actionNum) override {};
    virtual void applyAction(std::unique_ptr<Stream>& stream, double actionVal) override;
};

#endif
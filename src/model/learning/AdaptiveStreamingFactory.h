/*
 * AdaptiveStreamingFactory.h
 *
 *  factory method to create adaptive streaming algorithm
 *
 *  Created on: Aug 21, 2023
 *      Author: Nikita Smirnov
 */

#ifndef ADAPTIVESTREAMINGFACTORY_H
#define ADAPTIVESTREAMINGFACTORY_H

#include "model/common/AdaptiveAlgorithm.h"
#include "model/learning/IAdaptiveStreaming.h"
#include "model/learning/algorithms/AbrBase.h"
#include "model/learning/algorithms/AbrGcc.h"
#include "model/learning/algorithms/DrlBase.h"

class AdaptiveStreamingFactory {
public:
    static std::unique_ptr<IAdaptiveStreaming> create(AdaptiveAlgorithm alg) {
        switch (alg) {
            case AdaptiveAlgorithm::ABR_BASE:
                return std::make_unique<AbrBase>();
            case AdaptiveAlgorithm::ABR_GCC:
                return std::make_unique<AbrGcc>();
            case AdaptiveAlgorithm::DRL_BASE:
                return std::make_unique<DrlBase>();
            default:
                return nullptr;
        }
    }
};

#endif
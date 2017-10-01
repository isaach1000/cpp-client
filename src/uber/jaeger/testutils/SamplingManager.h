/*
 * Copyright (c) 2017, Uber Technologies, Inc
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef UBER_JAEGER_TESTUTILS_SAMPLINGMANAGER_H
#define UBER_JAEGER_TESTUTILS_SAMPLINGMANAGER_H

#include <mutex>
#include <string>
#include <unordered_map>

#include "uber/jaeger/thrift-gen/SamplingManager.h"
#include "uber/jaeger/thrift-gen/sampling_types.h"

namespace uber {
namespace jaeger {
namespace testutils {

class SamplingManager : public thrift::sampling_manager::SamplingManagerIf {
  public:
    using Response = thrift::sampling_manager::SamplingStrategyResponse;

    void getSamplingStrategy(Response& response,
                             const std::string& service) override
    {
        using ProbabilisticSamplingStrategy
            = thrift::sampling_manager::ProbabilisticSamplingStrategy;
        using SamplingStrategyType
            = thrift::sampling_manager::SamplingStrategyType;

        std::lock_guard<std::mutex> lock(_mutex);
        auto responseItr = _sampling.find(service);
        if (responseItr != std::end(_sampling)) {
            response = responseItr->second;
            return;
        }

        constexpr auto kSamplingRate = 0.01;
        ProbabilisticSamplingStrategy probabilisticSampling;
        probabilisticSampling.__set_samplingRate(kSamplingRate);
        response.__set_strategyType(SamplingStrategyType::PROBABILISTIC);
        response.__set_probabilisticSampling(probabilisticSampling);
    }

    void addSamplingStrategy(const std::string& serviceName,
                             const Response& response)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _sampling[serviceName] = response;
    }

  private:
    using StrategyMap = std::unordered_map<std::string, Response>;

    StrategyMap _sampling;
    std::mutex _mutex;
};

}  // namespace testutils
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_TESTUTILS_SAMPLINGMANAGER_H

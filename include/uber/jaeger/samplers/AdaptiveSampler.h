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

#ifndef UBER_JAEGER_SAMPLERS_ADAPTIVESAMPLER_H
#define UBER_JAEGER_SAMPLERS_ADAPTIVESAMPLER_H

#include <string>
#include <unordered_map>

#include <boost/thread/shared_mutex.hpp>

#include "uber/jaeger/Constants.h"
#include "uber/jaeger/samplers/GuaranteedThroughputProbabilisticSampler.h"
#include "uber/jaeger/samplers/ProbabilisticSampler.h"
#include "uber/jaeger/samplers/Sampler.h"
#include "uber/jaeger/thrift_gen/sampling_types.h"

namespace uber {
namespace jaeger {
namespace samplers {

class AdaptiveSampler : public Sampler {
  public:
    using PerOperationSamplingStrategies
        = thrift::sampling_manager::PerOperationSamplingStrategies;
    using SamplerMap = std::
        unordered_map<std::string,
                      std::
                          shared_ptr<GuaranteedThroughputProbabilisticSampler>>;

    AdaptiveSampler(const PerOperationSamplingStrategies& strategies,
                    size_t maxOperations);

    ~AdaptiveSampler() { close(); }

    SamplingStatus isSampled(const TraceID& id,
                             const std::string& operation) override;

    void close() override;

    void update(const PerOperationSamplingStrategies& strategies);

    Type type() const override { return Type::kAdaptiveSampler; }

  private:
    SamplerMap _samplers;
    ProbabilisticSampler _defaultSampler;
    double _lowerBound;
    size_t _maxOperations;
    boost::upgrade_mutex _rwMutex;
};

}  // namespace samplers
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_SAMPLERS_ADAPTIVESAMPLER_H

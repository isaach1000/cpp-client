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

#ifndef UBER_JAEGER_SAMPLERS_GUARANTEEDTHROUGHPUTPROBABILISTICSAMPLER_H
#define UBER_JAEGER_SAMPLERS_GUARANTEEDTHROUGHPUTPROBABILISTICSAMPLER_H

#include "uber/jaeger/Constants.h"
#include "uber/jaeger/samplers/ProbabilisticSampler.h"
#include "uber/jaeger/samplers/RateLimitingSampler.h"
#include "uber/jaeger/samplers/Sampler.h"

namespace uber {
namespace jaeger {
namespace samplers {

class GuaranteedThroughputProbabilisticSampler : public Sampler {
  public:
    GuaranteedThroughputProbabilisticSampler(
        double lowerBound, double samplingRate)
        : _probabilisticSampler(samplingRate)
        , _samplingRate(_probabilisticSampler.samplingRate())
        , _lowerBoundSampler(lowerBound)
        , _lowerBound(lowerBound)
        , _tags({{kSamplerTypeTagKey, kSamplerTypeLowerBound},
                 {kSamplerParamTagKey, _samplingRate}})
    {
    }

    SamplingStatus isSampled(
        const TraceID& id, const std::string& operation) override
    {
        const auto samplingStatus =
            _probabilisticSampler.isSampled(id, operation);
        if (samplingStatus.isSampled()) {
            _lowerBoundSampler.isSampled(id, operation);
            return samplingStatus;
        }
        const auto sampled =
            _lowerBoundSampler.isSampled(id, operation).isSampled();
        return SamplingStatus(sampled, samplingStatus.tags());
    }

    void close() override
    {
        _probabilisticSampler.close();
        _lowerBoundSampler.close();
    }

  private:
    ProbabilisticSampler _probabilisticSampler;
    double _samplingRate;
    RateLimitingSampler _lowerBoundSampler;
    double _lowerBound;
    std::vector<Tag> _tags;
};

}  // namespace samplers
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_SAMPLERS_GUARANTEEDTHROUGHPUTPROBABILISTICSAMPLER_H

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

#ifndef UBER_JAEGER_SAMPLERS_PROBABILISTICSAMPLER_H
#define UBER_JAEGER_SAMPLERS_PROBABILISTICSAMPLER_H

#include "uber/jaeger/Constants.h"
#include "uber/jaeger/samplers/Sampler.h"

namespace uber {
namespace jaeger {
namespace samplers {

class ProbabilisticSampler : public Sampler {
  public:
    explicit ProbabilisticSampler(double samplingRate)
        : _samplingRate(std::max(0.0, std::min(samplingRate, 1.0)))
        , _samplingBoundary(
              static_cast<uint64_t>(kMaxRandomNumber * _samplingRate))
        , _tags({ { kSamplerTypeTagKey, kSamplerTypeProbabilistic },
                  { kSamplerParamTagKey, _samplingRate } })
    {
    }

    double samplingRate() const { return _samplingRate; }

    SamplingStatus isSampled(const TraceID& id,
                             const std::string& operation) override
    {
        return SamplingStatus(_samplingBoundary >= id.low(), _tags);
    }

    void close() override {}

    Type type() const override { return Type::kProbabilisticSampler; }

  private:
    static constexpr auto kMaxRandomNumber
        = std::numeric_limits<uint64_t>::max();

    double _samplingRate;
    uint64_t _samplingBoundary;
    std::vector<Tag> _tags;
};

}  // namespace samplers
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_SAMPLERS_PROBABILISTICSAMPLER_H

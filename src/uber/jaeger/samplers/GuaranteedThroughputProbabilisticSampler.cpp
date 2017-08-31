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

#include "uber/jaeger/samplers/GuaranteedThroughputProbabilisticSampler.h"

namespace uber {
namespace jaeger {
namespace samplers {

void GuaranteedThroughputProbabilisticSampler::update(
    double lowerBound, double samplingRate)
{
    if (_samplingRate != samplingRate) {
        _probabilisticSampler = ProbabilisticSampler(samplingRate);
        _samplingRate = _probabilisticSampler.samplingRate();
        _tags = {
            { kSamplerTypeTagKey, kSamplerTypeLowerBound },
            { kSamplerParamTagKey, _samplingRate }
        };
    }

    if (_lowerBound != lowerBound) {
        _lowerBoundSampler.reset(new RateLimitingSampler(lowerBound));
        _lowerBound = lowerBound;
    }
}

}  // namespace samplers
}  // namespace jaeger
}  // namespace uber

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

#include "uber/jaeger/samplers/AdaptiveSampler.h"

namespace uber {
namespace jaeger {
namespace samplers {
namespace {

AdaptiveSampler::SamplerMap samplersFromStrategies(
    const thrift::sampling_manager::PerOperationSamplingStrategies& strategies)
{
    AdaptiveSampler::SamplerMap samplers;
    for (auto&& strategy : strategies.perOperationStrategies) {
        samplers[strategy.operation] =
            std::make_shared<GuaranteedThroughputProbabilisticSampler>(
                strategies.defaultLowerBoundTracesPerSecond,
                strategy.probabilisticSampling.samplingRate);
    }
    return samplers;
}

}  // anonymous namespace

AdaptiveSampler::AdaptiveSampler(
    const thrift::sampling_manager::PerOperationSamplingStrategies& strategies,
    size_t maxOperations)
    : _samplers(samplersFromStrategies(strategies))
    , _defaultSampler(strategies.defaultSamplingProbability)
    , _lowerBound(strategies.defaultLowerBoundTracesPerSecond)
    , _maxOperations(maxOperations)
    , _rwMutex()
{
}

SamplingStatus AdaptiveSampler::isSampled(
    const TraceID& id, const std::string& operation)
{
    boost::upgrade_lock<boost::upgrade_mutex> readLock(_rwMutex);
    auto samplerItr = _samplers.find(operation);
    if (samplerItr != std::end(_samplers)) {
        return samplerItr->second->isSampled(id, operation);
    }

    boost::upgrade_to_unique_lock<boost::upgrade_mutex> writeLock(readLock);
    samplerItr = _samplers.find(operation);
    if (samplerItr != std::end(_samplers)) {
        return samplerItr->second->isSampled(id, operation);
    }

    if (_samplers.size() >= _maxOperations) {
        return _defaultSampler.isSampled(id, operation);
    }

    auto newSampler =
        std::make_shared<GuaranteedThroughputProbabilisticSampler>(
            _lowerBound, _defaultSampler.samplingRate());
    _samplers[operation] = newSampler;
    return newSampler->isSampled(id, operation);
}

void AdaptiveSampler::close()
{
    boost::unique_lock<boost::upgrade_mutex> writeLock(_rwMutex);
    for (auto&& pair : _samplers) {
        pair.second->close();
    }
}

}  // namespace samplers
}  // namespace jaeger
}  // namespace uber

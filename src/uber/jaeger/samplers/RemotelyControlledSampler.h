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

#ifndef UBER_JAEGER_SAMPLERS_REMOTELYCONTROLLEDSAMPLER_H
#define UBER_JAEGER_SAMPLERS_REMOTELYCONTROLLEDSAMPLER_H

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "uber/jaeger/Constants.h"
#include "uber/jaeger/samplers/Sampler.h"
#include "uber/jaeger/samplers/SamplerOptions.h"
#include "uber/jaeger/thrift_gen/SamplingManager.h"

namespace uber {
namespace jaeger {
namespace samplers {

class RemotelyControlledSampler : public Sampler {
  public:
    using Clock = std::chrono::steady_clock;

    explicit RemotelyControlledSampler(const std::string& serviceName)
        : RemotelyControlledSampler(serviceName, SamplerOptions())
    {
    }

    RemotelyControlledSampler(const std::string& serviceName,
                              const SamplerOptions& options);

    ~RemotelyControlledSampler() { close(); }

    SamplingStatus isSampled(const TraceID& id,
                             const std::string& operation) override;

    void close() override;

    Type type() const override { return Type::kRemotelyControlledSampler; }

  private:
    using PerOperationSamplingStrategies
        = thrift::sampling_manager::PerOperationSamplingStrategies;
    using SamplingStrategyResponse
        = thrift::sampling_manager::SamplingStrategyResponse;

    void pollController();

    void updateSampler();

    void
    updateAdaptiveSampler(const PerOperationSamplingStrategies& strategies);

    void updateRateLimitingOrProbabilisticSampler(
        const SamplingStrategyResponse& response);

    SamplerOptions _options;
    std::string _serviceName;
    std::shared_ptr<thrift::sampling_manager::SamplingManagerIf> _manager;
    bool _running;
    std::mutex _mutex;
    std::condition_variable _shutdownCV;
    std::thread _thread;
};

}  // namespace samplers
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_SAMPLERS_REMOTELYCONTROLLEDSAMPLER_H

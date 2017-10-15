/*
 * Copyright (c) 2017 Uber Technologies, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef JAEGERTRACING_SAMPLERS_SAMPLEROPTIONS_H
#define JAEGERTRACING_SAMPLERS_SAMPLEROPTIONS_H

#include <cassert>
#include <chrono>
#include <memory>
#include <string>

#include <spdlog/spdlog.h>

#include "jaegertracing/Constants.h"
#include "jaegertracing/metrics/Metrics.h"
#include "jaegertracing/samplers/Sampler.h"
#include "jaegertracing/utils/RateLimiter.h"

namespace jaegertracing {
namespace metrics {

class Metrics;

}  // namespace metrics

namespace samplers {

class SamplerOptions {
  public:
    using Clock = std::chrono::steady_clock;

    SamplerOptions();

    ~SamplerOptions();

    const std::shared_ptr<metrics::Metrics>& metrics() const
    {
        return _metrics;
    }

    void setMetrics(const std::shared_ptr<metrics::Metrics>& metrics)
    {
        assert(metrics);
        _metrics = metrics;
    }

    size_t maxOperations() const { return _maxOperations; }

    void setMaxOperations(size_t maxOperations)
    {
        _maxOperations = maxOperations;
    }

    const std::shared_ptr<Sampler>& sampler() const { return _sampler; }

    void setSampler(const std::shared_ptr<Sampler>& sampler)
    {
        assert(sampler);
        _sampler = sampler;
    }

    const std::shared_ptr<spdlog::logger>& logger() const { return _logger; }

    void setLogger(const std::shared_ptr<spdlog::logger>& logger)
    {
        assert(logger);
        _logger = logger;
    }

    const std::string& samplingServerURL() const { return _samplingServerURL; }

    void setSamplingServerURL(const std::string& samplingServerURL)
    {
        _samplingServerURL = samplingServerURL;
    }

    const Clock::duration& samplingRefreshInterval() const
    {
        return _samplingRefreshInterval;
    }

    void
    setSamplingRefreshInterval(const Clock::duration& samplingRefreshInterval)
    {
        _samplingRefreshInterval = samplingRefreshInterval;
    }

  private:
    std::shared_ptr<metrics::Metrics> _metrics;
    size_t _maxOperations;
    std::shared_ptr<Sampler> _sampler;
    std::shared_ptr<spdlog::logger> _logger;
    std::string _samplingServerURL;
    Clock::duration _samplingRefreshInterval;
};

}  // namespace samplers
}  // namespace jaegertracing

#endif  // JAEGERTRACING_SAMPLERS_SAMPLEROPTIONS_H

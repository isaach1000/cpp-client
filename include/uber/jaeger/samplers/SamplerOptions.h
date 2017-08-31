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

#ifndef UBER_JAEGER_SAMPLERS_SAMPLEROPTIONS_H
#define UBER_JAEGER_SAMPLERS_SAMPLEROPTIONS_H

#include "uber/jaeger/Constants.h"
#include "uber/jaeger/metrics/Metrics.h"
#include "uber/jaeger/samplers/Sampler.h"
#include "uber/jaeger/utils/RateLimiter.h"

#include <cassert>
#include <chrono>
#include <memory>
#include <string>

namespace uber {
namespace jaeger {
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

    const std::shared_ptr<Sampler>& sampler() const
    {
        return _sampler;
    }

    void setSampler(const std::shared_ptr<Sampler>& sampler)
    {
        assert(sampler);
        _sampler = sampler;
    }

    /* TODO
    const std::shared_ptr<Logger>& logger() const { return _logger; }

    void setLogger(const std::shared_ptr<Logger>& logger)
    {
        assert(logger);
        _logger = logger;
    }
    */

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
    std::string _samplingServerURL;
    Clock::duration _samplingRefreshInterval;
};

}  // namespace samplers
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_SAMPLERS_SAMPLEROPTIONS_H

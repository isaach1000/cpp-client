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

#ifndef UBER_JAEGER_REPORTERS_REPORTEROPTIONS_H
#define UBER_JAEGER_REPORTERS_REPORTEROPTIONS_H

#include <cassert>
#include <chrono>

#include "uber/jaeger/Logging.h"
#include "uber/jaeger/metrics/Metrics.h"
#include "uber/jaeger/metrics/NullStatsFactory.h"

namespace uber {
namespace jaeger {
namespace reporters {

class ReporterOptions {
  public:
    using Clock = std::chrono::steady_clock;

    ReporterOptions()
        : _queueSize(kDefaultQueueSize)
        , _bufferFlushInterval(defaultBufferFlushInterval())
        , _logger(logging::nullLogger())
        , _metrics()
    {
        metrics::NullStatsFactory factory;
        _metrics = std::make_shared<metrics::Metrics>(factory);
    }

    int queueSize() const { return _queueSize; }

    void setQueueSize(int queueSize) { _queueSize = queueSize; }

    const Clock::duration& bufferFlushInterval() const
    {
        return _bufferFlushInterval;
    }

    void setBufferFlushInterval(const Clock::duration& bufferFlushInterval)
    {
        _bufferFlushInterval = bufferFlushInterval;
    }

    const std::shared_ptr<spdlog::logger>& logger() const { return _logger; }

    void setLogger(const std::shared_ptr<spdlog::logger>& logger)
    {
        assert(_logger);
        _logger = logger;
    }

    const std::shared_ptr<metrics::Metrics>& metrics() const
    {
        return _metrics;
    }

    void setMetrics(const std::shared_ptr<metrics::Metrics>& metrics)
    {
        assert(metrics);
        _metrics = metrics;
    }

  private:
    static constexpr auto kDefaultQueueSize = 100;

    static constexpr Clock::duration defaultBufferFlushInterval()
    {
        return std::chrono::seconds(10);
    }

    int _queueSize;
    Clock::duration _bufferFlushInterval;
    std::shared_ptr<spdlog::logger> _logger;
    std::shared_ptr<metrics::Metrics> _metrics;
};

}  // namespace reporters
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_REPORTERS_REPORTEROPTIONS_H

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

#ifndef JAEGERTRACING_REPORTERS_REPORTEROPTIONS_H
#define JAEGERTRACING_REPORTERS_REPORTEROPTIONS_H

#include <cassert>
#include <chrono>

#include "jaegertracing/Logging.h"
#include "jaegertracing/metrics/Metrics.h"
#include "jaegertracing/metrics/NullStatsFactory.h"

namespace jaegertracing {
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
}  // namespace jaegertracing

#endif  // JAEGERTRACING_REPORTERS_REPORTEROPTIONS_H

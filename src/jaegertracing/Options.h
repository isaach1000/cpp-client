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

#ifndef JAEGERTRACING_OPTIONS_H
#define JAEGERTRACING_OPTIONS_H

#include "jaegertracing/Logging.h"
#include "jaegertracing/Tag.h"
#include "jaegertracing/metrics/NullStatsFactory.h"
#include "jaegertracing/metrics/StatsFactory.h"
#include "jaegertracing/reporters/NullReporter.h"
#include "jaegertracing/reporters/Reporter.h"

namespace jaegertracing {

class Options {
  public:
    Options()
        : Options(nullptr, nullptr, nullptr, {})
    {
    }

    Options(const std::shared_ptr<metrics::StatsFactory>& statsFactory,
            const std::shared_ptr<spdlog::logger>& logger,
            const std::shared_ptr<reporters::Reporter>& reporter,
            const std::vector<Tag>& tags)
        : _statsFactory(statsFactory ? statsFactory
                                     : std::shared_ptr<metrics::StatsFactory>(
                                           new metrics::NullStatsFactory()))
        , _logger(logger ? logger : logging::nullLogger())
        , _reporter(reporter ? reporter
                             : std::shared_ptr<reporters::Reporter>(
                                   new reporters::NullReporter()))
        , _tags(tags)
    {
    }

    const std::shared_ptr<metrics::StatsFactory>& statsFactory() const
    {
        return _statsFactory;
    }

    const std::shared_ptr<spdlog::logger>& logger() const { return _logger; }

    const std::shared_ptr<reporters::Reporter>& reporter() const
    {
        return _reporter;
    }

    const std::vector<Tag>& tags() const { return _tags; }

  private:
    std::shared_ptr<metrics::StatsFactory> _statsFactory;
    std::shared_ptr<spdlog::logger> _logger;
    std::shared_ptr<reporters::Reporter> _reporter;
    std::vector<Tag> _tags;
};

}  // namespace jaegertracing

#endif  // JAEGERTRACING_OPTIONS_H

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

#ifndef JAEGERTRACING_TRACER_H
#define JAEGERTRACING_TRACER_H

#include <chrono>
#include <random>
#include <vector>

#include <opentracing/tracer.h>

#include "jaegertracing/Logging.h"
#include "jaegertracing/Span.h"
#include "jaegertracing/Tag.h"
#include "jaegertracing/baggage/RestrictionManager.h"
#include "jaegertracing/metrics/Metrics.h"
#include "jaegertracing/net/IPAddress.h"
#include "jaegertracing/propagation/Propagator.h"
#include "jaegertracing/reporters/Reporter.h"
#include "jaegertracing/samplers/Sampler.h"

namespace jaegertracing {

class Tracer : public opentracing::Tracer {
  public:
    using Clock = std::chrono::steady_clock;

    using string_view = opentracing::string_view;

    Tracer(const std::string& serviceName,
           std::unique_ptr<samplers::Sampler>&& sampler,
           std::unique_ptr<reporters::Reporter>&& reporter)
        : _serviceName(serviceName)
        , _hostIPv4(net::IPAddress::host(AF_INET))
        , _sampler(std::move(sampler))
        , _reporter(std::move(reporter))
        , _metrics(metrics::Metrics::makeNullMetrics())
        , _logger(logging::nullLogger())
        , _randomNumberGenerator()
        , _textPropagator()
        , _httpHeaderPropagator()
        , _binaryPropagator()
        , _tags()
        , _restrictionManager(new baggage::DefaultRestrictionManager(0))
    {
        if (_hostIPv4 == net::IPAddress()) {
            _logger->error("Unable to determine this host's IP address");
        }

        std::random_device device;
        _randomNumberGenerator.seed(device());
    }

    std::unique_ptr<opentracing::Span> StartSpanWithOptions(
        string_view operationName,
        const opentracing::StartSpanOptions& options) const noexcept override;

  private:
    std::string _serviceName;
    net::IPAddress _hostIPv4;
    std::unique_ptr<samplers::Sampler> _sampler;
    std::unique_ptr<reporters::Reporter> _reporter;
    std::unique_ptr<metrics::Metrics> _metrics;
    std::shared_ptr<spdlog::logger> _logger;
    std::mt19937 _randomNumberGenerator;
    propagation::TextMapPropagator _textPropagator;
    propagation::HTTPHeaderPropagator _httpHeaderPropagator;
    propagation::BinaryPropagator _binaryPropagator;
    std::vector<Tag> _tags;
    std::unique_ptr<baggage::RestrictionManager> _restrictionManager;
};

}  // namespace jaegertracing

#endif  // JAEGERTRACING_TRACER_H

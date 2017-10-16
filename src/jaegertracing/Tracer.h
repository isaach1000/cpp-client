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
#include <memory>
#include <random>
#include <vector>

#include <opentracing/tracer.h>

#include "jaegertracing/Constants.h"
#include "jaegertracing/Logging.h"
#include "jaegertracing/Span.h"
#include "jaegertracing/Tag.h"
#include "jaegertracing/baggage/BaggageSetter.h"
#include "jaegertracing/baggage/RestrictionManager.h"
#include "jaegertracing/metrics/Metrics.h"
#include "jaegertracing/net/IPAddress.h"
#include "jaegertracing/platform/Hostname.h"
#include "jaegertracing/propagation/Propagator.h"
#include "jaegertracing/reporters/Reporter.h"
#include "jaegertracing/samplers/Sampler.h"

namespace jaegertracing {

class Tracer : public opentracing::Tracer,
               public std::enable_shared_from_this<Tracer> {
  public:
    using Clock = std::chrono::steady_clock;

    using string_view = opentracing::string_view;

    template <typename... Args>
    std::shared_ptr<Tracer> make(Args&&... args)
    {
        // Avoid `std::make_shared` when using `std::weak_ptr`.
        return std::shared_ptr<Tracer>(new Tracer(std::forward<Args>(args)...));
    }

    std::unique_ptr<opentracing::Span>
    StartSpanWithOptions(string_view operationName,
                         const opentracing::StartSpanOptions& options) const
        noexcept override;

    opentracing::expected<void> Inject(const opentracing::SpanContext& ctx,
                                       std::ostream& writer) const override
    {
        const auto* jaegerCtx = dynamic_cast<const SpanContext*>(&ctx);
        if (!jaegerCtx) {
            return opentracing::make_expected_from_error<void>(
                opentracing::invalid_span_context_error);
        }
        _binaryPropagator.inject(*jaegerCtx, writer);
        return opentracing::make_expected();
    }

    opentracing::expected<void>
    Inject(const opentracing::SpanContext& ctx,
           const opentracing::TextMapWriter& writer) const override
    {
        const auto* jaegerCtx = dynamic_cast<const SpanContext*>(&ctx);
        if (!jaegerCtx) {
            return opentracing::make_expected_from_error<void>(
                opentracing::invalid_span_context_error);
        }
        _textPropagator.inject(*jaegerCtx, writer);
        return opentracing::make_expected();
    }

    opentracing::expected<void>
    Inject(const opentracing::SpanContext& ctx,
           const opentracing::HTTPHeadersWriter& writer) const override
    {
        const auto* jaegerCtx = dynamic_cast<const SpanContext*>(&ctx);
        if (!jaegerCtx) {
            return opentracing::make_expected_from_error<void>(
                opentracing::invalid_span_context_error);
        }
        _textPropagator.inject(*jaegerCtx, writer);
        return opentracing::make_expected();
    }

    opentracing::expected<std::unique_ptr<opentracing::SpanContext>>
    Extract(std::istream& reader) const override
    {
        return std::unique_ptr<opentracing::SpanContext>(
            new SpanContext(_binaryPropagator.extract(reader)));
    }

    opentracing::expected<std::unique_ptr<opentracing::SpanContext>>
    Extract(const opentracing::TextMapReader& reader) const override
    {
        return std::unique_ptr<opentracing::SpanContext>(
            new SpanContext(_textPropagator.extract(reader)));
    }

    opentracing::expected<std::unique_ptr<opentracing::SpanContext>>
    Extract(const opentracing::HTTPHeadersReader& reader) const override
    {
        return std::unique_ptr<opentracing::SpanContext>(
            new SpanContext(_httpHeaderPropagator.extract(reader)));
    }

    void Close() noexcept override
    {
        _reporter->close();
        _sampler->close();
        _restrictionManager->close();
    }

    void close() noexcept { Close(); }

    const std::string& serviceName() const { return _serviceName; }

    const baggage::BaggageSetter& baggageSetter() const
    {
        return _baggageSetter;
    }

    void reportSpan(const Span& span) const
    {
        _metrics->spansFinished().inc(1);
        if (span.context().isSampled()) {
            _reporter->report(span);
        }
    }

  private:
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
        , _baggageSetter(*_restrictionManager, *_metrics)
    {
        _tags.push_back(Tag(kJaegerClientVersionTagKey, kJaegerClientVersion));

        try {
            _tags.push_back(Tag(kTracerHostnameTagKey, platform::hostname()));
        } catch (const std::system_error&) {
            // Ignore hostname error.
        }

        if (_hostIPv4 == net::IPAddress()) {
            _logger->error("Unable to determine this host's IP address");
        }
        else {
            std::ostringstream oss;
            oss << _hostIPv4;
            _tags.push_back(Tag(kTracerIPTagKey, oss.str()));
        }

        std::random_device device;
        {
            std::lock_guard<std::mutex> lock(_randomMutex);
            _randomNumberGenerator.seed(device());
        }
    }

    uint64_t randomID() const
    {
        std::lock_guard<std::mutex> lock(_randomMutex);
        auto value = _randomNumberGenerator();
        while (value == 0) {
            value = _randomNumberGenerator();
        }
        return value;
    }

    using OpenTracingTag = std::pair<std::string, opentracing::Value>;

    std::unique_ptr<Span>
    startSpanInternal(const SpanContext& context,
                      const std::string& operationName,
                      const Clock::time_point& startTime,
                      const std::vector<Tag>& internalTags,
                      const std::vector<OpenTracingTag>& tags,
                      bool newTrace,
                      const std::vector<Reference>& references) const;

    std::string _serviceName;
    net::IPAddress _hostIPv4;
    std::unique_ptr<samplers::Sampler> _sampler;
    std::unique_ptr<reporters::Reporter> _reporter;
    std::unique_ptr<metrics::Metrics> _metrics;
    std::shared_ptr<spdlog::logger> _logger;
    mutable std::mt19937 _randomNumberGenerator;
    mutable std::mutex _randomMutex;
    propagation::TextMapPropagator _textPropagator;
    propagation::HTTPHeaderPropagator _httpHeaderPropagator;
    propagation::BinaryPropagator _binaryPropagator;
    std::vector<Tag> _tags;
    std::unique_ptr<baggage::RestrictionManager> _restrictionManager;
    baggage::BaggageSetter _baggageSetter;
};

}  // namespace jaegertracing

#endif  // JAEGERTRACING_TRACER_H

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

#ifndef JAEGERTRACING_SPAN_H
#define JAEGERTRACING_SPAN_H

#include <chrono>
#include <memory>
#include <mutex>

#include <opentracing/span.h>

#include "jaegertracing/LogRecord.h"
#include "jaegertracing/Reference.h"
#include "jaegertracing/SpanContext.h"
#include "jaegertracing/Tag.h"
#include "jaegertracing/thrift-gen/jaeger_types.h"

namespace jaegertracing {

class Tracer;

class Span : public opentracing::Span {
  public:
    using Clock = std::chrono::steady_clock;

    Span(const Span& span)
    {
        std::lock_guard<std::mutex> lock(span._mutex);
        _context = span._context;
        _operationName = span._operationName;
        _startTime = span._startTime;
        _duration = span._duration;
        _tags = span._tags;
        _logs = span._logs;
        _references = span._references;
    }

    Span& operator=(const Span& rhs)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        std::lock_guard<std::mutex> rhsLock(rhs._mutex);
        _context = rhs._context;
        _operationName = rhs._operationName;
        _startTime = rhs._startTime;
        _duration = rhs._duration;
        _tags = rhs._tags;
        _logs = rhs._logs;
        _references = rhs._references;
        return *this;
    }

    ~Span();

    thrift::Span thrift() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        thrift::Span span;
        span.__set_traceIdHigh(_context.traceID().high());
        span.__set_traceIdLow(_context.traceID().low());
        span.__set_spanId(_context.spanID());
        span.__set_parentSpanId(_context.parentID());
        span.__set_operationName(_operationName);

        std::vector<thrift::SpanRef> refs;
        refs.reserve(_references.size());
        std::transform(std::begin(_references),
                       std::end(_references),
                       std::back_inserter(refs),
                       [](const Reference& ref) { return ref.thrift(); });
        span.__set_references(refs);

        span.__set_flags(_context.flags());
        span.__set_startTime(
            std::chrono::duration_cast<std::chrono::microseconds>(
                _startTime.time_since_epoch())
                .count());
        span.__set_duration(
            std::chrono::duration_cast<std::chrono::microseconds>(_duration)
                .count());

        std::vector<thrift::Tag> tags;
        tags.reserve(_tags.size());
        std::transform(std::begin(_tags),
                       std::end(_tags),
                       std::back_inserter(tags),
                       [](const Tag& tag) { return tag.thrift(); });
        span.__set_tags(tags);

        std::vector<thrift::Log> logs;
        logs.reserve(_logs.size());
        std::transform(std::begin(_logs),
                       std::end(_logs),
                       std::back_inserter(logs),
                       [](const LogRecord& log) { return log.thrift(); });
        span.__set_logs(logs);

        return span;
    }

    template <typename Stream>
    void print(Stream& out) const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        out << _context;
    }

    std::string operationName() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _operationName;
    }

    Clock::time_point startTime() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _startTime;
    }

    Clock::duration duration() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _duration;
    }

    std::vector<Tag> tags() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _tags;
    }

    template <typename... Arg>
    void setOperationName(Arg&&... args)
    {
        SetOperationName(std::forward<Arg>(args)...);
    }

    void FinishWithOptions(const opentracing::FinishSpanOptions&
                               finishSpanOptions) noexcept override
    {
        // TODO
    }

    void SetOperationName(opentracing::string_view name) noexcept override
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (isFinished()) {
            return;
        }
        _operationName = name;
    }

    void SetTag(opentracing::string_view key,
                const opentracing::Value& value) noexcept override
    {
        // TODO
    }

    void SetBaggageItem(opentracing::string_view restrictedKey,
                        opentracing::string_view value) noexcept override
    {
        // TODO
    }

    std::string BaggageItem(opentracing::string_view restrictedKey) const
        noexcept override
    {
        // TODO
        return "";
    }

    void Log(std::initializer_list<
             std::pair<opentracing::string_view, opentracing::Value>>
                 fields) noexcept override
    {
        // TODO
    }

  protected:
    const opentracing::SpanContext& context() const noexcept override
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _context;
    }

    const opentracing::Tracer& tracer() const noexcept override;

  private:
    bool isFinished() const { return _duration != Clock::duration(); }

    std::weak_ptr<Tracer> _tracer;
    SpanContext _context;
    std::string _operationName;
    Clock::time_point _startTime;
    Clock::duration _duration;
    std::vector<Tag> _tags;
    std::vector<LogRecord> _logs;
    std::vector<Reference> _references;
    mutable std::mutex _mutex;
};

}  // namespace jaegertracing

inline std::ostream& operator<<(
    std::ostream& out, const jaegertracing::Span& span)
{
    span.print(out);
    return out;
}

#endif  // JAEGERTRACING_SPAN_H

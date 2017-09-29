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

#ifndef UBER_JAEGER_SPAN_H
#define UBER_JAEGER_SPAN_H

#include <chrono>
#include <memory>
#include <mutex>

#include "uber/jaeger/LogRecord.h"
#include "uber/jaeger/Reference.h"
#include "uber/jaeger/SpanContext.h"
#include "uber/jaeger/Tag.h"

namespace uber {
namespace jaeger {

class Tracer;

class Span {
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
        return *this;
    }

    ~Span();

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

  private:
    // TODO: Tracer& _tracer;
    SpanContext _context;
    std::string _operationName;
    Clock::time_point _startTime;
    Clock::duration _duration;
    std::vector<Tag> _tags;
    std::vector<LogRecord> _logs;
    std::vector<Reference> _references;
    mutable std::mutex _mutex;
};

}  // namespace jaeger
}  // namespace uber

template <typename Stream>
inline Stream& operator<<(Stream& out, const uber::jaeger::Span& span)
{
    span.print(out);
    return out;
}

#endif  // UBER_JAEGER_SPAN_H

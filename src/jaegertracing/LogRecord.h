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

#ifndef JAEGERTRACING_LOGRECORD_H
#define JAEGERTRACING_LOGRECORD_H

#include <chrono>

#include "jaegertracing/Tag.h"

namespace jaegertracing {

class LogRecord {
  public:
    using Clock = std::chrono::steady_clock;

    LogRecord()
        : _timestamp(Clock::now())
    {
    }

    template <typename FieldIterator>
    LogRecord(const Clock::time_point& timestamp,
              FieldIterator first,
              FieldIterator last)
        : _timestamp(timestamp)
        , _fields(first, last)
    {
    }

    const Clock::time_point& timestamp() const { return _timestamp; }

    const std::vector<Tag>& fields() const { return _fields; }

    thrift::Log thrift() const
    {
        thrift::Log log;
        log.__set_timestamp(
            std::chrono::duration_cast<std::chrono::microseconds>(
                _timestamp.time_since_epoch())
                .count());

        std::vector<thrift::Tag> fields;
        fields.reserve(_fields.size());
        std::transform(std::begin(_fields),
                       std::end(_fields),
                       std::back_inserter(fields),
                       [](const Tag& tag) { return tag.thrift(); });
        log.__set_fields(fields);

        return log;
    }

  private:
    Clock::time_point _timestamp;
    std::vector<Tag> _fields;
};

}  // namespace jaegertracing

#endif  // JAEGERTRACING_LOGRECORD_H

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

#ifndef UBER_JAEGER_SPANCONTEXT_H
#define UBER_JAEGER_SPANCONTEXT_H

#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>

#include <opentracing/span.h>

#include "uber/jaeger/TraceID.h"

namespace uber {
namespace jaeger {

class SpanContext : public opentracing::SpanContext {
  public:
    static SpanContext fromStream(std::istream& in);

    SpanContext() = default;

    SpanContext(const TraceID& traceID,
                uint64_t spanID,
                uint64_t parentID,
                bool sampled,
                const std::unordered_map<std::string, std::string>& baggage)
        : _traceID(traceID)
        , _spanID(spanID)
        , _parentID(parentID)
        , _flags(sampled ? static_cast<unsigned char>(Flag::kSampled) : 0)
        , _baggage(baggage)
        , _debugID()
    {
    }

    const TraceID& traceID() const { return _traceID; }

    uint64_t spanID() const { return _spanID; }

    uint64_t parentID() const { return _parentID; }

    template <typename Function>
    void forEachBaggageItem(Function f) const
    {
        for (auto&& pair : _baggage) {
            if (!f(pair.first, pair.second)) {
                break;
            }
        }
    }

    template <typename Function>
    void forEachBaggageItem(Function f)
    {
        for (auto&& pair : _baggage) {
            if (!f(pair.first, pair.second)) {
                break;
            }
        }
    }

    unsigned char flags() const { return _flags; }

    bool isSampled() const
    {
        return _flags & static_cast<unsigned char>(Flag::kSampled);
    }

    bool isDebug() const
    {
        return _flags & static_cast<unsigned char>(Flag::kDebug);
    }

    bool isValid() const { return _traceID.isValid() && _spanID != 0; }

    template <typename Stream>
    void print(Stream& out) const
    {
        _traceID.print(out);
        out << ':' << std::hex << _spanID << ':' << std::hex << _parentID << ':'
            << std::hex << static_cast<size_t>(_flags);
    }

    void ForeachBaggageItem(
        std::function<bool(const std::string& key, const std::string& value)> f)
        const override
    {
        forEachBaggageItem(f);
    }

  private:
    enum class Flag : unsigned char { kSampled = 1, kDebug = 2 };

    using StrMap = std::unordered_map<std::string, std::string>;

    TraceID _traceID;
    uint64_t _spanID;
    uint64_t _parentID;
    unsigned char _flags;
    StrMap _baggage;
    std::string _debugID;
};

}  // namespace jaeger
}  // namespace uber

template <typename Stream>
inline Stream& operator<<(Stream& out,
                          const uber::jaeger::SpanContext& spanContext)
{
    spanContext.print(out);
    return out;
}

#endif  // UBER_JAEGER_SPANCONTEXT_H

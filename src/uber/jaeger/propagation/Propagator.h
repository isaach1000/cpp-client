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

#ifndef UBER_JAEGER_PROPAGATION_PROPAGATOR_H
#define UBER_JAEGER_PROPAGATION_PROPAGATOR_H

#include <cctype>
#include <sstream>

#include <opentracing/propagation.h>

#include "uber/jaeger/metrics/Metrics.h"
#include "uber/jaeger/net/URI.h"
#include "uber/jaeger/propagation/Extractor.h"
#include "uber/jaeger/propagation/HeadersConfig.h"
#include "uber/jaeger/propagation/Injector.h"

namespace uber {
namespace jaeger {
namespace propagation {

template <typename ReaderType, typename WriterType>
class Propagator : public Extractor<ReaderType>, public Injector<WriterType> {
  public:
    using Reader = ReaderType;
    using Writer = WriterType;
    using StrMap = SpanContext::StrMap;

    Propagator(const HeadersConfig& headerKeys, metrics::Metrics& metrics)
        : _headerKeys(headerKeys)
        , _metrics(metrics)
    {
    }

    virtual ~Propagator() = default;

    SpanContext extract(const Reader& reader) const override
    {
        SpanContext ctx;
        StrMap baggage;
        const auto err = reader.ForeachKey(
            [this, &reader, &ctx, &baggage](const std::string& rawKey,
                                            const std::string& value) {
                const auto key = normalizeKey(rawKey);
                if (key == _headerKeys.traceContextHeaderName()) {
                    const auto safeValue = decodeValue(value);
                    std::istringstream iss(safeValue);
                    if (!(iss >> ctx)) {
                        return opentracing::span_context_corrupted_error;
                    }
                }
                else if (key == _headerKeys.jaegerDebugHeader()) {
                    ctx.setDebug();
                }
                else if (key == _headerKeys.jaegerBaggageHeader()) {
                    for (auto&& pair : parseCommaSeparatedMap(value)) {
                        baggage[pair.first] = pair.second;
                    }
                }
                else {
                    const auto prefix = _headerKeys.traceBaggageHeaderPrefix();
                    if (key.size() >= prefix.size() &&
                        key.substr(0, prefix.size()) == prefix) {
                        const auto safeKey = removeBaggageKeyPrefix(key);
                        const auto safeValue = decodeValue(value);
                        baggage[safeKey] = safeValue;
                    }
                }
            });
        if (err) {
            _metrics.decodingErrors()->inc(1);
            return SpanContext();
        }
        if (!ctx.traceID().isValid() && !ctx.isDebug() && baggage.empty()) {
            return SpanContext();
        }
        ctx.setBaggage(baggage);
        return ctx;
    }

    void inject(const SpanContext& ctx, const Writer& writer) const override
    {
        std::ostringstream oss;
        oss << ctx;
        writer.Set(_headerKeys.traceContextHeaderName(), oss.str());
        ctx.forEachBaggageItem(
            [this, &writer](const std::string& key, const std::string& value) {
                const auto safeKey = addBaggageKeyPrefix(key);
                const auto safeValue = encodeValue(value);
                writer.Set(safeKey, safeValue);
            });
    }

  protected:
    virtual std::string encodeValue(const std::string& str) const
    {
        return str;
    }

    virtual std::string decodeValue(const std::string& str) const
    {
        return str;
    }

    virtual std::string normalizeKey(const std::string& rawKey) const
    {
        return rawKey;
    }

  private:
    static StrMap parseCommaSeparatedMap(const std::string& escapedValue)
    {
        StrMap map;
        const auto value = net::URI::queryUnescape(escapedValue);
        for (auto pos = value.find(','), prev = static_cast<size_t>(0);
             pos != std::string::npos;
             prev = pos, pos = value.find(',', pos + 1)) {
            const auto piece = value.substr(prev, pos);
            const auto eqPos = piece.find('=');
            if (eqPos != std::string::npos) {
                const auto key = piece.substr(0, eqPos);
                const auto value = piece.substr(eqPos + 1);
                map[key] = value;
            }
        }
        return map;
    }

    std::string addBaggageKeyPrefix(const std::string& key) const
    {
        return _headerKeys.traceBaggageHeaderPrefix() + key;
    }

    std::string removeBaggageKeyPrefix(const std::string& key) const
    {
        return key.substr(_headerKeys.traceBaggageHeaderPrefix().size());
    }

    HeadersConfig _headerKeys;
    metrics::Metrics& _metrics;
};

using TextMapPropagator =
    Propagator<opentracing::TextMapReader, opentracing::TextMapWriter>;

class HTTPHeaderPropagator : public Propagator<opentracing::HTTPHeadersReader,
                                               opentracing::HTTPHeadersWriter> {
  public:
    using Propagator<Reader, Writer>::Propagator;

  protected:
    std::string encodeValue(const std::string& str) const override
    {
        return net::URI::queryEscape(str);
    }

    std::string decodeValue(const std::string& str) const override
    {
        return net::URI::queryUnescape(str);
    }

    std::string normalizeKey(const std::string& rawKey) const override
    {
        std::string key;
        key.reserve(rawKey.size());
        std::transform(std::begin(rawKey),
                       std::end(rawKey),
                       std::back_inserter(key),
                       [](char ch) { return std::tolower(ch); });
        return key;
    }
};

}  // namespace propagation
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_PROPAGATION_PROPAGATOR_H

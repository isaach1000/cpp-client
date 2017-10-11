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
#include <climits>
#include <sstream>

#include <opentracing/propagation.h>

#include "uber/jaeger/SpanContext.h"
#include "uber/jaeger/metrics/Metrics.h"
#include "uber/jaeger/net/URI.h"
#include "uber/jaeger/platform/Endian.h"
#include "uber/jaeger/propagation/Extractor.h"
#include "uber/jaeger/propagation/HeadersConfig.h"
#include "uber/jaeger/propagation/Injector.h"

namespace uber {
namespace jaeger {

class Tracer;

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
            _metrics.decodingErrors().inc(1);
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
    Propagator<const opentracing::TextMapReader&,
               const opentracing::TextMapWriter&>;

class HTTPHeaderPropagator :
    public Propagator<const opentracing::HTTPHeadersReader&,
                      const opentracing::HTTPHeadersWriter&> {
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

class BinaryPropagator : public Extractor<std::istream&>,
                         public Injector<std::ostream&> {
  public:
    using StrMap = SpanContext::StrMap;

    void inject(const SpanContext& ctx, std::ostream& out) const override
    {
        writeBinary(out, ctx.traceID().high());
        writeBinary(out, ctx.traceID().low());
        writeBinary(out, ctx.spanID());
        writeBinary(out, ctx.parentID());
        // `flags` is a single byte, so endianness is not an issue.
        out.put(ctx.flags());

        writeBinary(out, static_cast<uint32_t>(ctx.baggage().size()));
        for (auto&& pair : ctx.baggage()) {
            auto&& key = pair.first;
            writeBinary(out, static_cast<uint32_t>(key.size()));
            out.write(key.c_str(), key.size());

            auto&& value = pair.second;
            writeBinary(out, static_cast<uint32_t>(value.size()));
            out.write(value.c_str(), value.size());
        }
    }

    SpanContext extract(std::istream& in) const override
    {
        const auto traceIDHigh = readBinary<uint64_t>(in);
        const auto traceIDLow = readBinary<uint64_t>(in);
        TraceID traceID(traceIDHigh, traceIDLow);
        const auto spanID = readBinary<uint64_t>(in);
        const auto parentID = readBinary<uint64_t>(in);

        auto ch = '\0';
        in.get(ch);
        const auto flags = static_cast<unsigned char>(ch);

        const auto numBaggageItems = readBinary<uint32_t>(in);
        StrMap baggage;
        baggage.reserve(numBaggageItems);
        for (auto i = static_cast<uint32_t>(0); i < numBaggageItems; ++i) {
            const auto keyLength = readBinary<uint32_t>(in);
            std::string key(keyLength, '\0');
            in.read(&key[0], keyLength);

            const auto valueLength = readBinary<uint32_t>(in);
            std::string value(valueLength, '\0');
            in.read(&value[0], valueLength);

            baggage[key] = value;
        }

        SpanContext ctx(traceID, spanID, parentID, false, baggage);
        ctx.setFlags(flags);
        return ctx;
    }

  private:
    template <typename ValueType>
    static
    typename std::enable_if<std::is_integral<ValueType>::value, void>::type
    writeBinary(std::ostream& out, ValueType value)
    {
        const ValueType outValue = platform::endian::toBigEndian(value);
        for (auto i = static_cast<size_t>(0); i < sizeof(ValueType); ++i) {
            const auto numShiftBits = (sizeof(ValueType) - i - 1) * CHAR_BIT;
            const auto byte = outValue >> numShiftBits;
            out.put(static_cast<unsigned char>(byte));
        }
    }

    template <typename ValueType>
    static
    typename std::enable_if<std::is_integral<ValueType>::value, ValueType>::type
    readBinary(std::istream& in)
    {
        auto value = static_cast<ValueType>(0);
        auto ch = '\0';
        for (auto i = static_cast<size_t>(0);
             i < sizeof(ValueType) && in.get(ch);
             ++i) {
            const auto byte = static_cast<uint8_t>(ch);
            value <<= CHAR_BIT;
            value |= byte;
        }
        return platform::endian::fromBigEndian(value);
    }
};

}  // namespace propagation
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_PROPAGATION_PROPAGATOR_H

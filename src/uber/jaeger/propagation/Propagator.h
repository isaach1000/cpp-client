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

    Propagator(const HeadersConfig& headerKeys,
               metrics::Metrics& metrics)
        : _headerKeys(headerKeys)
        , _metrics(metrics)
    {
    }

    virtual ~Propagator() = default;

    SpanContext extract(const Reader& reader) const override
    {
        SpanContext ctx;
        const auto result = reader.ForeachKey([this, &reader, &ctx](
            const std::string& key,
            const std::string& value) {
                if (key == _headerKeys.traceContextHeaderName()) {
                    std::istringstream iss(value);
                    if (!(iss >> ctx)) {
                        return opentracing::invalid_span_context_error;
                    }
                }
            });
    }

    void inject(const SpanContext& ctx, const Writer& writer) const override
    {
        std::ostringstream oss;
        oss << ctx;
        writer.Set(_headerKeys.traceContextHeaderName(), oss.str());
        ctx.forEachBaggageItem([this, &writer](
            const std::string& key,
            const std::string& value) {
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

  private:
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

class HTTPHeaderPropagator :
    public Propagator<opentracing::HTTPHeadersReader,
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
};

}  // namespace propagation
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_PROPAGATION_PROPAGATOR_H

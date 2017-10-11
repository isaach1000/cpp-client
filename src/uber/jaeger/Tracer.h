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

#ifndef UBER_JAEGER_TRACER_H
#define UBER_JAEGER_TRACER_H

#include <chrono>
#include <random>
#include <vector>

#include <opentracing/tracer.h>

#include "uber/jaeger/Logging.h"
#include "uber/jaeger/Span.h"
#include "uber/jaeger/Tag.h"
#include "uber/jaeger/baggage/RestrictionManager.h"
#include "uber/jaeger/metrics/Metrics.h"
#include "uber/jaeger/net/IPAddress.h"
#include "uber/jaeger/propagation/Propagator.h"
#include "uber/jaeger/reporters/Reporter.h"
#include "uber/jaeger/samplers/Sampler.h"

/* TODO
#include "uber/jaeger/internal/baggage/RestrictionManager.h"
*/

namespace uber {
namespace jaeger {

class Tracer : public opentracing::Tracer {
  public:
    using Clock = std::chrono::steady_clock;

    class Options {
      public:
        Options() = default;

        Options(bool poolSpans, bool gen128Bit)
            : _poolSpans(poolSpans)
            , _gen128Bit(gen128Bit)
        {
        }

        bool poolSpans() const { return _poolSpans; }

        bool gen128Bit() const { return _gen128Bit; }

      private:
        bool _poolSpans;
        bool _gen128Bit;
    };

  private:
    std::string _serviceName;
    net::IPAddress _hostIPv4;
    std::unique_ptr<samplers::Sampler> _sampler;
    std::unique_ptr<reporters::Reporter> _reporter;
    metrics::Metrics _metrics;
    std::shared_ptr<spdlog::logger> _logger;
    std::mt19937 _randomNumberGenerator;
    // TODO: `Span` object pool.
    propagation::TextMapPropagator _textPropagator;
    propagation::HTTPHeaderPropagator _httpHeaderPropagator;
    propagation::BinaryPropagator _binaryPropagator;
    std::vector<Tag> _tags;
    std::unique_ptr<baggage::RestrictionManager> _restrictionManager;
};

}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_TRACER_H

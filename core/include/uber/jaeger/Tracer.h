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
#include <random>
#include <vector>

#include "uber/jaeger/Span.h"
#include "uber/jaeger/Tag.h"
/* TODO
#include "uber/jaeger/Logger.h"
#include "uber/jaeger/internal/baggage/RestrictionManager.h"
#include "uber/jaeger/propagation/Extractor.h"
#include "uber/jaeger/propagation/Injector.h"
#include "uber/jaeger/reporters/Reporter.h"
#include "uber/jaeger/samplers/Sampler.h"
*/

namespace uber {
namespace jaeger {

template <typename ClockType = std::chrono::steady_clock>
class Tracer {
  public:
    using Clock = ClockType;
    using TimePoint = typename Clock::time_point;

    class Options {
      public:
        Options() = default;

        Options(bool poolSpans,
                bool gen128Bit,
                bool zipkinSharedRPCSpan)
            : _poolSpans(poolSpans)
            , _gen128Bit(gen128Bit)
            , _zipkinSharedRPCSpan(zipkinSharedRPCSpan)
        {
        }

        bool poolSpans() const { return _poolSpans; }

        bool gen128Bit() const { return _gen128Bit; }

        bool zipkinSharedRPCSpan() const { return _zipkinSharedRPCSpan; }

      private:
        bool _poolSpans;
        bool _gen128Bit;
        bool _zipkinSharedRPCSpan;
    };

  private:
    std::string _serviceName;
    uint32_t _hostIPv4;
    /* TODO
    samplers::Sampler _sampler;
    reporters::Reporter _reporter;
    metrics::Metrics _metrics;
    Logger _logger;
    */
    std::mt19937 _randomNumberGenerator;
    // TODO: `Span` object pool.
    /* TODO
    std::unordered_map<Format, Injector> _injectors;
    std::unordered_map<Format, Extractor> _extractors;
     */
    std::vector<Tag> _tags;
};

}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_SPAN_H

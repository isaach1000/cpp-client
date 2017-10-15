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

#include "jaegertracing/samplers/SamplerOptions.h"

#include "jaegertracing/Logging.h"
#include "jaegertracing/metrics/Counter.h"
#include "jaegertracing/metrics/Gauge.h"
#include "jaegertracing/metrics/NullStatsFactory.h"
#include "jaegertracing/samplers/ProbabilisticSampler.h"

namespace jaegertracing {
namespace samplers {
namespace {

constexpr auto kDefaultMaxOperations = 2000;
constexpr auto kDefaultSamplingRate = 0.001;
constexpr auto kDefaultSamplingServerURL = "http://localhost:5778/sampling";
const auto kDefaultSamplingRefreshInterval = std::chrono::minutes(1);

}  // anonymous namespace

SamplerOptions::SamplerOptions()
    : _metrics()
    , _maxOperations(kDefaultMaxOperations)
    , _sampler(std::make_shared<ProbabilisticSampler>(kDefaultSamplingRate))
    , _logger(logging::nullLogger())
    , _samplingServerURL(kDefaultSamplingServerURL)
    , _samplingRefreshInterval(kDefaultSamplingRefreshInterval)
{
    metrics::NullStatsFactory factory;
    _metrics = std::make_shared<metrics::Metrics>(factory);
}

SamplerOptions::~SamplerOptions() = default;

}  // namespace samplers
}  // namespace jaegertracing

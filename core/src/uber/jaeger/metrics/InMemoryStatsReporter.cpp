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

#include "uber/jaeger/metrics/InMemoryStatsReporter.h"

#include "uber/jaeger/metrics/Counter.h"
#include "uber/jaeger/metrics/Gauge.h"
#include "uber/jaeger/metrics/Metric.h"
#include "uber/jaeger/metrics/Metrics.h"
#include "uber/jaeger/metrics/Timer.h"

namespace uber {
namespace jaeger {
namespace metrics {
namespace {

template <typename Function>
void updateMap(InMemoryStatsReporter::ValueMap& map,
               const std::string& name,
               int64_t newValue,
               const std::unordered_map<std::string, std::string>& tags,
               Function f)
{
    const auto metricName = Metrics::addTagsToMetricName(name, tags);
    auto& initialValue = map[metricName];
    initialValue = f(initialValue, newValue);
}

}  // anonymous namespace

void InMemoryStatsReporter::incCounter(
    const std::string& name,
    int64_t delta,
    const std::unordered_map<std::string, std::string>& tags)
{
    updateMap(_counters,
              name,
              delta,
              tags,
              [](int64_t initialValue, int64_t newValue) {
                  return initialValue + newValue;
              });
}

void InMemoryStatsReporter::recordTimer(
    const std::string& name,
    int64_t time,
    const std::unordered_map<std::string, std::string>& tags)
{
    updateMap(
        _timers, name, time, tags, [](int64_t initialValue, int64_t newValue) {
            return initialValue + newValue;
        });
}

void InMemoryStatsReporter::updateGauge(
    const std::string& name,
    int64_t amount,
    const std::unordered_map<std::string, std::string>& tags)
{
    updateMap(_gauges, name, amount, tags, [](int64_t, int64_t newValue) {
        return newValue;
    });
}

void InMemoryStatsReporter::reset()
{
    _counters.clear();
    _gauges.clear();
    _timers.clear();
}

}  // namespace metrics
}  // namespace jaeger
}  // namespace uber

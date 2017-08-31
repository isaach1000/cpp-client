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

#include <gtest/gtest.h>

#include "uber/jaeger/metrics/Counter.h"
#include "uber/jaeger/metrics/Gauge.h"
#include "uber/jaeger/metrics/InMemoryStatsReporter.h"
#include "uber/jaeger/metrics/Metrics.h"
#include "uber/jaeger/metrics/Timer.h"

namespace uber {
namespace jaeger {
namespace metrics {

class MetricsTest : public ::testing::Test {
  public:
    MetricsTest()
        : _metricsReporter()
        , _metrics(Metrics::fromStatsReporter(_metricsReporter))
    {
    }

  protected:
    InMemoryStatsReporter _metricsReporter;
    std::unique_ptr<Metrics> _metrics;
};

TEST_F(MetricsTest, testCounter)
{
    constexpr auto counterValue = static_cast<int64_t>(3);
    constexpr auto metricName = "jaeger.test-counter";
    StatsFactoryImpl factory(_metricsReporter);
    auto counter = factory.createCounter(metricName, {});
    counter->inc(counterValue);
    const auto& counters = _metricsReporter.counters();
    ASSERT_EQ(1, counters.size());
    auto itr = counters.find(metricName);
    ASSERT_TRUE(itr != std::end(counters));
    ASSERT_EQ(counterValue, itr->second);
}

TEST_F(MetricsTest, testGauge)
{
    constexpr auto gaugeValue = static_cast<int64_t>(3);
    constexpr auto metricName = "jaeger.test-gauge";
    StatsFactoryImpl factory(_metricsReporter);
    auto gauge = factory.createGauge(metricName, {});
    gauge->update(gaugeValue);
    const auto& gauges = _metricsReporter.gauges();
    ASSERT_EQ(1, gauges.size());
    auto itr = gauges.find(metricName);
    ASSERT_TRUE(itr != std::end(gauges));
    ASSERT_EQ(gaugeValue, itr->second);
}

TEST_F(MetricsTest, testTimer)
{
    constexpr auto timeValue = static_cast<int64_t>(5);
    constexpr auto metricName = "jaeger.test-timer";
    StatsFactoryImpl factory(_metricsReporter);
    auto timer = factory.createTimer(metricName, {});
    timer->record(timeValue);
    const auto& timers = _metricsReporter.timers();
    ASSERT_EQ(1, timers.size());
    auto itr = timers.find(metricName);
    ASSERT_TRUE(itr != std::end(timers));
    ASSERT_EQ(timeValue, itr->second);
}

TEST_F(MetricsTest, testReset)
{
    _metrics->tracesJoinedSampled().inc(1);
    const auto& counters = _metricsReporter.counters();
    ASSERT_EQ(1, counters.size());

    _metrics->reporterQueueLength().update(1);
    const auto& gauges = _metricsReporter.gauges();
    ASSERT_EQ(1, gauges.size());

    constexpr auto time = static_cast<int64_t>(1);
    constexpr auto timerName = "jaeger.test-timer";
    _metricsReporter.recordTimer(timerName, time, {});
    const auto& timers = _metricsReporter.timers();
    ASSERT_EQ(1, timers.size());

    _metricsReporter.reset();
    ASSERT_TRUE(counters.empty());
    ASSERT_TRUE(gauges.empty());
    ASSERT_TRUE(timers.empty());
}

}  // namespace metrics
}  // namespace jaeger
}  // namespace uber

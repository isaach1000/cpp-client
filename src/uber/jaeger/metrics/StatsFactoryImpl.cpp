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

#include "uber/jaeger/metrics/StatsFactoryImpl.h"

#include "uber/jaeger/metrics/Counter.h"
#include "uber/jaeger/metrics/Gauge.h"
#include "uber/jaeger/metrics/Metric.h"
#include "uber/jaeger/metrics/Timer.h"

namespace uber {
namespace jaeger {
namespace metrics {
namespace {

class ReportedMetric : public Metric {
  public:
    ReportedMetric(StatsReporter& reporter,
                   const std::string& name,
                   const TagMap& tags)
        : Metric(name, tags)
        , _reporter(reporter)
    {
    }

    virtual ~ReportedMetric() = default;

  protected:
    StatsReporter& reporter() { return _reporter; }

  private:
    StatsReporter& _reporter;
};

class CounterImpl : public ReportedMetric, public Counter {
  public:
    CounterImpl(StatsReporter& reporter,
                const std::string& name,
                const std::unordered_map<std::string, std::string>& tags)
        : ReportedMetric(reporter, name, tags)
    {
    }

    void inc(int64_t delta) override
    {
        reporter().incCounter(name(), delta, tags());
    }
};

class TimerImpl : public ReportedMetric, public Timer {
  public:
    TimerImpl(StatsReporter& reporter,
              const std::string& name,
              const std::unordered_map<std::string, std::string>& tags)
        : ReportedMetric(reporter, name, tags)
    {
    }

    void durationMicros(int64_t time) override
    {
        reporter().recordTimer(name(), time, tags());
    }
};

class GaugeImpl : public ReportedMetric, public Gauge {
  public:
    GaugeImpl(StatsReporter& reporter,
              const std::string& name,
              const std::unordered_map<std::string, std::string>& tags)
        : ReportedMetric(reporter, name, tags)
    {
    }

    void update(int64_t amount) override
    {
        reporter().updateGauge(name(), amount, tags());
    }
};

}  // anonymous namespace

StatsFactoryImpl::StatsFactoryImpl(StatsReporter& reporter)
    : _reporter(reporter)
{
}

std::unique_ptr<Counter> StatsFactoryImpl::createCounter(
    const std::string& name,
    const std::unordered_map<std::string, std::string>& tags)
{
    return std::unique_ptr<Counter>(new CounterImpl(_reporter, name, tags));
}

std::unique_ptr<Timer> StatsFactoryImpl::createTimer(
    const std::string& name,
    const std::unordered_map<std::string, std::string>& tags)
{
    return std::unique_ptr<Timer>(new TimerImpl(_reporter, name, tags));
}

std::unique_ptr<Gauge> StatsFactoryImpl::createGauge(
    const std::string& name,
    const std::unordered_map<std::string, std::string>& tags)
{
    return std::unique_ptr<Gauge>(new GaugeImpl(_reporter, name, tags));
}

}  // namespace metrics
}  // namespace jaeger
}  // namespace uber

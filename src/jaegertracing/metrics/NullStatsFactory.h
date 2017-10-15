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

#ifndef JAEGERTRACING_METRICS_NULLSTATSFACTORY_H
#define JAEGERTRACING_METRICS_NULLSTATSFACTORY_H

#include <memory>
#include <string>
#include <unordered_map>

#include "jaegertracing/metrics/NullCounter.h"
#include "jaegertracing/metrics/NullGauge.h"
#include "jaegertracing/metrics/NullTimer.h"
#include "jaegertracing/metrics/StatsFactory.h"

namespace jaegertracing {
namespace metrics {

class Counter;
class Gauge;
class Timer;

class NullStatsFactory : public StatsFactory {
  public:
    std::unique_ptr<Counter>
    createCounter(const std::string&,
                  const std::unordered_map<std::string, std::string>&) override
    {
        return std::unique_ptr<Counter>(new NullCounter());
    }

    std::unique_ptr<Timer>
    createTimer(const std::string&,
                const std::unordered_map<std::string, std::string>&) override
    {
        return std::unique_ptr<Timer>(new NullTimer());
    }

    std::unique_ptr<Gauge>
    createGauge(const std::string&,
                const std::unordered_map<std::string, std::string>&) override
    {
        return std::unique_ptr<Gauge>(new NullGauge());
    }
};

}  // namespace metrics
}  // namespace jaegertracing

#endif  // JAEGERTRACING_METRICS_NULLSTATSFACTORY_H

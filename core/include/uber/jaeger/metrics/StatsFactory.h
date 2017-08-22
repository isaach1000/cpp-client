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

#ifndef UBER_JAEGER_METRICS_STATSFACTORY_H
#define UBER_JAEGER_METRICS_STATSFACTORY_H

#include <string>
#include <unordered_map>

namespace uber {
namespace jaeger {
namespace metrics {

class Counter;
class Gauge;
class Timer;

class StatsFactory {
  public:
    virtual ~StatsFactory() = default;

    virtual std::unique_ptr<Counter> createCounter(
        const std::string& name,
        const std::unordered_map<std::string, std::string>& tags) = 0;

    virtual std::unique_ptr<Timer> createTimer(
        const std::string& name,
        const std::unordered_map<std::string, std::string>& tags) = 0;

    virtual std::unique_ptr<Gauge> createGauge(
        const std::string& name,
        const std::unordered_map<std::string, std::string>& tags) = 0;
};

}  // namespace metrics
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_METRICS_STATSFACTORY_H

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

#ifndef UBER_JAEGER_METRICS_METRICS_H
#define UBER_JAEGER_METRICS_METRICS_H

#include <string>
#include <unordered_map>

#include "uber/jaeger/metrics/StatsFactory.h"
#include "uber/jaeger/metrics/StatsFactoryImpl.h"
#include "uber/jaeger/metrics/StatsReporter.h"

namespace uber {
namespace jaeger {
namespace metrics {

class Metrics {
  public:
    static std::unique_ptr<Metrics> fromStatsReporter(StatsReporter& reporter)
    {
        // Factory only used for constructor, so need not live past the
        // initialization of Metrics object.
        StatsFactoryImpl factory(reporter);
        return std::unique_ptr<Metrics>(new Metrics(factory));
    }

    static std::string addTagsToMetricName(
        const std::string& name,
        const std::unordered_map<std::string, std::string>& tags);

    explicit Metrics(StatsFactory& factory)
        : _traceStartedSampled(factory.createCounter(
              "jaeger.traces", { { "state", "started" }, { "sampled", "y" } }))
        , _traceStartedNotSampled(factory.createCounter(
              "jaeger.traces", { { "state", "started" }, { "sampled", "n" } }))
        , _tracesJoinedSampled(factory.createCounter(
              "jaeger.traces", { { "state", "joined" }, { "sampled", "y" } }))
        , _tracesJoinedNotSampled(factory.createCounter(
              "jaeger.traces", { { "state", "joined" }, { "sampled", "n" } }))
        , _spansStarted(factory.createCounter(
              "jaeger.spans",
              { { "state", "started" }, { "group", "lifecycle" } }))
        , _spansFinished(factory.createCounter(
              "jaeger.spans",
              { { "state", "finished" }, { "group", "lifecycle" } }))
        , _spansSampled(factory.createCounter(
              "jaeger.spans", { { "group", "sampling" }, { "sampled", "y" } }))
        , _spansNotSampled(factory.createCounter(
              "jaeger.spans", { { "group", "sampling" }, { "sampled", "n" } }))
        , _decodingErrors(factory.createCounter("jaeger.decoding-errors", {}))
        , _reporterSuccess(factory.createCounter("jaeger.reporter-spans",
                                                 { { "state", "success" } }))
        , _reporterFailure(factory.createCounter("jaeger.reporter-spans",
                                                 { { "state", "failure" } }))
        , _reporterDropped(factory.createCounter("jaeger.reporter-spans",
                                                 { { "state", "dropped" } }))
        , _reporterQueueLength(factory.createGauge("jaeger.reporter-queue", {}))
        , _samplerRetrieved(factory.createCounter("jaeger.sampler",
                                                  { { "state", "retrieved" } }))
        , _samplerUpdated(factory.createCounter("jaeger.sampler",
                                                { { "state", "updated" } }))
        , _samplerQueryFailure(factory.createCounter(
              "jaeger.sampler",
              { { "state", "failure" }, { "phase", "query" } }))
        , _samplerParsingFailure(factory.createCounter(
              "jaeger.sampler",
              { { "state", "failure" }, { "phase", "parsing" } }))
        , _baggageUpdateSuccess(factory.createCounter("jaeger.baggage-update",
                                                      { { "result", "ok" } }))
        , _baggageUpdateFailure(factory.createCounter("jaeger.baggage-update",
                                                      { { "result", "err" } }))
        , _baggageTruncate(factory.createCounter("jaeger.baggage-truncate", {}))
        , _baggageRestrictionsUpdateSuccess(factory.createCounter(
              "jaeger.baggage-restrictions-update", { { "result", "ok" } }))
        , _baggageRestrictionsUpdateFailure(factory.createCounter(
              "jaeger.baggage-restrictions-update", { { "result", "err" } }))
    {
    }

    ~Metrics();

    const Counter& traceStartedSampled() const { return *_traceStartedSampled; }
    Counter& traceStartedSampled() { return *_traceStartedSampled; }
    const Counter& traceStartedNotSampled() const
    {
        return *_traceStartedNotSampled;
    }
    Counter& traceStartedNotSampled() { return *_traceStartedNotSampled; }
    const Counter& tracesJoinedSampled() const { return *_tracesJoinedSampled; }
    Counter& tracesJoinedSampled() { return *_tracesJoinedSampled; }
    const Counter& tracesJoinedNotSampled() const
    {
        return *_tracesJoinedNotSampled;
    }
    Counter& tracesJoinedNotSampled() { return *_tracesJoinedNotSampled; }
    const Counter& spansStarted() const { return *_spansStarted; }
    Counter& spansStarted() { return *_spansStarted; }
    const Counter& spansFinished() const { return *_spansFinished; }
    Counter& spansFinished() { return *_spansFinished; }
    const Counter& spansSampled() const { return *_spansSampled; }
    Counter& spansSampled() { return *_spansSampled; }
    const Counter& spansNotSampled() const { return *_spansNotSampled; }
    Counter& spansNotSampled() { return *_spansNotSampled; }
    const Counter& decodingErrors() const { return *_decodingErrors; }
    Counter& decodingErrors() { return *_decodingErrors; }
    const Counter& reporterSuccess() const { return *_reporterSuccess; }
    Counter& reporterSuccess() { return *_reporterSuccess; }
    const Counter& reporterFailure() const { return *_reporterFailure; }
    Counter& reporterFailure() { return *_reporterFailure; }
    const Counter& reporterDropped() const { return *_reporterDropped; }
    Counter& reporterDropped() { return *_reporterDropped; }
    const Gauge& reporterQueueLength() const { return *_reporterQueueLength; }
    Gauge& reporterQueueLength() { return *_reporterQueueLength; }
    const Counter& samplerRetrieved() const { return *_samplerRetrieved; }
    Counter& samplerRetrieved() { return *_samplerRetrieved; }
    const Counter& samplerUpdated() const { return *_samplerUpdated; }
    Counter& samplerUpdated() { return *_samplerUpdated; }
    const Counter& samplerQueryFailure() const { return *_samplerQueryFailure; }
    Counter& samplerQueryFailure() { return *_samplerQueryFailure; }
    const Counter& samplerParsingFailure() const
    {
        return *_samplerParsingFailure;
    }
    Counter& samplerParsingFailure() { return *_samplerParsingFailure; }
    const Counter& baggageUpdateSuccess() const
    {
        return *_baggageUpdateSuccess;
    }
    Counter& baggageUpdateSuccess() { return *_baggageUpdateSuccess; }
    const Counter& baggageUpdateFailure() const
    {
        return *_baggageUpdateFailure;
    }
    Counter& baggageUpdateFailure() { return *_baggageUpdateFailure; }
    const Counter& baggageTruncate() const { return *_baggageTruncate; }
    Counter& baggageTruncate() { return *_baggageTruncate; }

  private:
    std::unique_ptr<Counter> _traceStartedSampled;
    std::unique_ptr<Counter> _traceStartedNotSampled;
    std::unique_ptr<Counter> _tracesJoinedSampled;
    std::unique_ptr<Counter> _tracesJoinedNotSampled;
    std::unique_ptr<Counter> _spansStarted;
    std::unique_ptr<Counter> _spansFinished;
    std::unique_ptr<Counter> _spansSampled;
    std::unique_ptr<Counter> _spansNotSampled;
    std::unique_ptr<Counter> _decodingErrors;
    std::unique_ptr<Counter> _reporterSuccess;
    std::unique_ptr<Counter> _reporterFailure;
    std::unique_ptr<Counter> _reporterDropped;
    std::unique_ptr<Gauge> _reporterQueueLength;
    std::unique_ptr<Counter> _samplerRetrieved;
    std::unique_ptr<Counter> _samplerUpdated;
    std::unique_ptr<Counter> _samplerQueryFailure;
    std::unique_ptr<Counter> _samplerParsingFailure;
    std::unique_ptr<Counter> _baggageUpdateSuccess;
    std::unique_ptr<Counter> _baggageUpdateFailure;
    std::unique_ptr<Counter> _baggageTruncate;
    std::unique_ptr<Counter> _baggageRestrictionsUpdateSuccess;
    std::unique_ptr<Counter> _baggageRestrictionsUpdateFailure;
};

}  // namespace metrics
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_METRICS_METRICS_H

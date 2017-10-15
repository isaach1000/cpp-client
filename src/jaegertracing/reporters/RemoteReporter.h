/*
 * Copyright (c) 2017 Uber Technologies, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef JAEGERTRACING_REPORTERS_REMOTEREPORTER_H
#define JAEGERTRACING_REPORTERS_REMOTEREPORTER_H

#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

#include "jaegertracing/Span.h"
#include "jaegertracing/Transport.h"
#include "jaegertracing/reporters/Reporter.h"
#include "jaegertracing/reporters/ReporterOptions.h"

namespace jaegertracing {
namespace reporters {

class RemoteReporter : public Reporter {
  public:
    using Clock = ReporterOptions::Clock;

    explicit RemoteReporter(
        std::unique_ptr<Transport> sender,
        const ReporterOptions& reporterOptions = ReporterOptions());

    void report(const Span& span) override;

    void close() override;

  private:
    void sweepQueue();

    void sendSpan(const Span& span);

    void flush();

    bool bufferFlushIntervalExpired() const
    {
        return (_lastFlush - Clock::now()) >=
               _reporterOptions.bufferFlushInterval();
    }

    ReporterOptions _reporterOptions;
    std::unique_ptr<Transport> _sender;
    std::deque<Span> _queue;
    std::atomic<int64_t> _queueLength;
    bool _running;
    bool _forceFlush;  // For testing
    Clock::time_point _lastFlush;
    std::condition_variable _cv;
    std::mutex _mutex;
    std::thread _thread;
};

}  // namespace reporters
}  // namespace jaegertracing

#endif  // JAEGERTRACING_REPORTERS_REMOTEREPORTER_H

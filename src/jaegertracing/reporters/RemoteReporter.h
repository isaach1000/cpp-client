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

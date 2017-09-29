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

#include "uber/jaeger/reporters/RemoteReporter.h"

namespace uber {
namespace jaeger {
namespace reporters {

RemoteReporter::RemoteReporter(std::unique_ptr<Transport> sender,
                               const ReporterOptions& reporterOptions)
    : _reporterOptions(reporterOptions)
    , _sender(std::move(sender))
    , _queue()
    , _queueLength(0)
    , _running(true)
    , _forceFlush(false)
    , _lastFlush(Clock::now())
    , _cv()
    , _mutex()
    , _thread()
{
    _thread = std::thread([this]() { sweepQueue(); });
}

void RemoteReporter::report(const Span& span)
{
    std::unique_lock<std::mutex> lock(_mutex);
    const auto pushed = (
        static_cast<int>(_queue.size()) < _reporterOptions.queueSize());
    if (pushed) {
        _queue.push_back(span);
        lock.unlock();
        _cv.notify_one();
        ++_queueLength;
    }
    else {
        _reporterOptions.metrics()->reporterDropped().inc(1);
    }
}

void RemoteReporter::close()
{
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _running = false;
        lock.unlock();
        _cv.notify_one();
    }
    _thread.join();
}

void RemoteReporter::sweepQueue()
{
    while (true) {
        std::unique_lock<std::mutex> lock(_mutex);

        if (!_running) {
            return;
        }

        _cv.wait(lock, [this]() {
            return !_running ||
                   !_queue.empty() ||
                   bufferFlushIntervalExpired() ||
                   _forceFlush;
        });

        if (!_running) {
            return;
        }

        if (!_queue.empty()) {
            const auto span = _queue.front();
            _queue.pop_front();
            sendSpan(span);
        }
        else if (bufferFlushIntervalExpired() || _forceFlush) {
            flush();
        }
    }
}

void RemoteReporter::sendSpan(const Span& span)
{
    --_queueLength;
    try {
        const auto flushed = _sender->append(span);
        if (flushed > 0) {
            _reporterOptions.metrics()->reporterSuccess().inc(flushed);
            _reporterOptions.metrics()->reporterQueueLength().update(
                _queueLength);
        }
    }
    catch (const Transport::Exception& ex) {
        _reporterOptions.metrics()->reporterFailure().inc(
            ex.numFailed());
        const auto& logger = _reporterOptions.logger();
        assert(logger);
        logger->error(fmt::format("error reporting span {0}: {1}",
                                  span.operationName(),
                                  ex.what()));
    }
}

void RemoteReporter::flush()
{
    try {
        const auto flushed = _sender->flush();
        if (flushed > 0) {
            _reporterOptions.metrics()->reporterSuccess().inc(flushed);
        }
    }
    catch (const Transport::Exception& ex) {
        _reporterOptions.metrics()->reporterFailure().inc(ex.numFailed());
        _reporterOptions.logger()->error(ex.what());
    }

    _lastFlush = Clock::now();
}

}  // namespace reporters
}  // namespace jaeger
}  // namespace uber

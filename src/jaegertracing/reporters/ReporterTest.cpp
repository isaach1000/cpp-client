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

#include <gtest/gtest.h>

#include "jaegertracing/Logging.h"
#include "jaegertracing/Tracer.h"
#include "jaegertracing/Transport.h"
#include "jaegertracing/reporters/InMemoryReporter.h"
#include "jaegertracing/reporters/LoggingReporter.h"
#include "jaegertracing/reporters/RemoteReporter.h"
#include "jaegertracing/samplers/ConstSampler.h"

namespace jaegertracing {
namespace reporters {
namespace {

class FakeTransport : public Transport {
  public:
    FakeTransport(std::vector<Span>& spans, std::mutex& mutex)
        : _spans(spans)
        , _mutex(mutex)
    {
    }

    int append(const Span& span) override
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _spans.push_back(span);
        return 1;
    }

    int flush() override { return 0; }

    void close() override {}

  private:
    std::vector<Span>& _spans;
    std::mutex& _mutex;
};

const Span span(std::weak_ptr<Tracer>(),
                    SpanContext(),
                    "",
                    Span::Clock::now(),
                    Span::Clock::duration(),
                    {},
                    {},
                    false);

}  // anonymous namespace

TEST(Reporter, testRemoteReporter)
{
    std::vector<Span> spans;
    std::mutex mutex;
    RemoteReporter reporter(
        std::unique_ptr<Transport>(new FakeTransport(spans, mutex)));
    const Span span(std::weak_ptr<Tracer>(),
                    SpanContext(),
                    "",
                    Span::Clock::now(),
                    Span::Clock::duration(),
                    {},
                    {},
                    false);
    constexpr auto kNumReports = 100;
    for (auto i = 0; i < kNumReports; ++i) {
        reporter.report(span);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    {
        std::lock_guard<std::mutex> lock(mutex);
        ASSERT_EQ(kNumReports, spans.size());
    }
    reporter.close();
}

TEST(Reporter, testLoggingReporter)
{
    LoggingReporter reporter(logging::nullLogger());
    constexpr auto kNumReports = 100;
    for (auto i = 0; i < kNumReports; ++i) {
        reporter.report(span);
    }
    reporter.close();
}

TEST(Reporter, testInMemoryReporter)
{
    InMemoryReporter reporter;
    constexpr auto kNumReports = 100;
    for (auto i = 0; i < kNumReports; ++i) {
        reporter.report(span);
    }
    ASSERT_EQ(kNumReports, reporter.spansSubmitted());
    reporter.reset();
    ASSERT_EQ(0, reporter.spansSubmitted());
    reporter.close();
}

}  // namespace reporters
}  // namespace jaegertracing

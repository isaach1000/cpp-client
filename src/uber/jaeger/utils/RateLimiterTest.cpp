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

#include "uber/jaeger/utils/RateLimiter.h"

namespace uber {
namespace jaeger {
namespace utils {
namespace {

std::chrono::steady_clock::time_point currentTime;

class MockClock {
  public:
    using rep = std::chrono::steady_clock::rep;
    using period = std::chrono::steady_clock::period;
    using duration = std::chrono::steady_clock::duration;
    using time_point = std::chrono::steady_clock::time_point;

    static const bool is_steady() { return false; }

    static time_point now() { return currentTime; }
};

}  // anonymous namespace

TEST(RateLimiter, testRateLimiter)
{
    const auto timestamp = std::chrono::steady_clock::now();
    currentTime = timestamp;
    RateLimiter<MockClock> limiter(2, 2);

    ASSERT_TRUE(limiter.checkCredit(1));
    ASSERT_TRUE(limiter.checkCredit(1));
    ASSERT_FALSE(limiter.checkCredit(1));

    currentTime = timestamp + std::chrono::milliseconds(250);
    ASSERT_FALSE(limiter.checkCredit(1));

    currentTime = timestamp + std::chrono::milliseconds(750);
    ASSERT_TRUE(limiter.checkCredit(1));
    ASSERT_FALSE(limiter.checkCredit(1));

    currentTime = timestamp + std::chrono::seconds(5);
    ASSERT_TRUE(limiter.checkCredit(1));
    ASSERT_TRUE(limiter.checkCredit(1));
    ASSERT_FALSE(limiter.checkCredit(1));
    ASSERT_FALSE(limiter.checkCredit(1));
    ASSERT_FALSE(limiter.checkCredit(1));
}

TEST(RateLimiter, testMaxBalance)
{
    const auto timestamp = std::chrono::steady_clock::now();
    currentTime = timestamp;
    RateLimiter<MockClock> limiter(0.1, 1.0);

    ASSERT_TRUE(limiter.checkCredit(1.0));

    currentTime = timestamp + std::chrono::seconds(20);
    ASSERT_TRUE(limiter.checkCredit(1.0));
    ASSERT_FALSE(limiter.checkCredit(1.0));
}

}  // namespace utils
}  // namespace jaeger
}  // namespace uber

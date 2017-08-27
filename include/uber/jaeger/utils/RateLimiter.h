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

#ifndef UBER_JAEGER_UTILS_RATELIMITER_H
#define UBER_JAEGER_UTILS_RATELIMITER_H

#include <chrono>
#include <mutex>

namespace uber {
namespace jaeger {
namespace utils {

template <typename ClockType = std::chrono::steady_clock>
class RateLimiter {
  public:
    using Clock = ClockType;
    using TimePoint = typename Clock::time_point;

    RateLimiter(double creditsPerSecond, double maxBalance)
        : _creditsPerSecond(creditsPerSecond)
        , _maxBalance(maxBalance)
        , _balance(_maxBalance)
        , _lastTick(Clock::now())
    {
    }

    bool checkCredit(double itemCost)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        const auto currentTime = Clock::now();
        const auto elapsedTime
            = std::chrono::duration<double>(currentTime - _lastTick);
        _lastTick = currentTime;

        _balance += elapsedTime.count() * _creditsPerSecond;
        if (_balance > _maxBalance) {
            _balance = _maxBalance;
        }

        if (_balance >= itemCost) {
            _balance -= itemCost;
            return true;
        }

        return false;
    }

  private:
    double _creditsPerSecond;
    double _maxBalance;
    double _balance;
    TimePoint _lastTick;
    std::mutex _mutex;
};

}  // namespace utils
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_UTILS_RATELIMITER_H

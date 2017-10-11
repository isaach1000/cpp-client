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

#include <random>

#include <gtest/gtest.h>

#include "uber/jaeger/propagation/Propagator.h"

namespace uber {
namespace jaeger {
namespace propagation {
namespace {

template <typename RandomGenerator>
std::string randomStr(RandomGenerator& gen)
{
    constexpr auto kMaxSize = 10;
    static constexpr char kLetters[] = "abcdefghijklmnopqrstuvwxyz"
                                       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                       "0123456789";
    constexpr auto kNumLetters = sizeof(kLetters) - 1;
    const auto size = gen() % kMaxSize;
    std::string result(size, '\0');
    for (auto i = static_cast<size_t>(0); i < size; ++i) {
        const auto pos = gen() % kNumLetters;
        result[pos] = kLetters[pos];
    }
    return result;
}

}  // anonymous namespace

TEST(Propagator, testBinaryPropagation)
{
    std::stringstream ss;
    BinaryPropagator binaryPropagator;
    std::random_device randomDevice;
    std::default_random_engine engine(randomDevice());
    constexpr auto kMaxNumBaggageItems = 10;
    const auto numBaggageItems = engine() % (kMaxNumBaggageItems - 1) + 1;
    SpanContext::StrMap baggage;
    for (auto i = static_cast<size_t>(0); i < numBaggageItems; ++i) {
        const auto key = randomStr(engine);
        const auto value = randomStr(engine);
        baggage[key] = value;
    }
    SpanContext ctx(
        TraceID(engine(), engine()),
        engine(),
        engine(),
        false,
        baggage);
    binaryPropagator.inject(ctx, ss);
    ASSERT_EQ(ctx, binaryPropagator.extract(ss));
}

}  // namespace propagation
}  // namespace jaeger
}  // namespace uber

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

#include <sstream>

#include "uber/jaeger/SpanContext.h"

namespace uber {
namespace jaeger {
namespace {

struct FromStreamTestCase {
    std::string _input;
    bool _success;
};

}  // anonymous namespace

TEST(SpanContext, testFromStream)
{
    const FromStreamTestCase testCases[] = {
        { "", false },
        { "abcd", false },
        { "ABCD", false },
        { "x:1:1:1", false },
        { "1:x:1:1", false },
        { "1:1:x:1", false },
        { "1:1:1:x", false },
        { "01234567890123456789012345678901234:1:1:1", false },
        { "01234567890123456789012345678901:1:1:1", true },
        { "01234_67890123456789012345678901:1:1:1", false },
        { "0123456789012345678901_345678901:1:1:1", false },
        { "1:0123456789012345:1:1", true },
        { "1:01234567890123456:1:1", false },
        { "10000000000000001:1:1:1", true },
        { "10000000000000001:1:1", false },
        { "1:1:1:1", true }
    };

    for (auto&& testCase : testCases) {
        std::stringstream ss;
        ss << testCase._input;
        auto spanContext = SpanContext::fromStream(ss);
        ASSERT_EQ(testCase._success, spanContext.isValid())
            << "input=" << testCase._input;
    }
}

TEST(SpanContext, testFormatting)
{
    SpanContext spanContext(TraceID(255, 255), 0, 0, false, {});
    std::ostringstream oss;
    oss << spanContext;
    ASSERT_EQ("ff00000000000000ff:0:0:0", oss.str());
}

}  // namespace jaeger
}  // namespace uber

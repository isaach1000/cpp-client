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

#include "jaegertracing/net/http/Header.h"

namespace jaegertracing {
namespace net {
namespace http {

TEST(Header, readLine)
{
    std::stringstream ss;
    ss << "test\r123\r\n";
    std::string line;
    while (readLineCRLF(ss, line)) {
        ASSERT_EQ("test\r123", line);
    }
}

TEST(Header, readHeaders)
{
    std::stringstream ss;
    ss << "Bad Header\r\n";
    std::vector<Header> headers;
    ASSERT_THROW(readHeaders(ss, headers), ParseError);
}

}  // namespace http
}  // namespace net
}  // namespace jaegertracing

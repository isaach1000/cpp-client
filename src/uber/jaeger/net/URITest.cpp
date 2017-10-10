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

#include "uber/jaeger/net/URI.h"

namespace uber {
namespace jaeger {
namespace net {

TEST(URI, testMatch)
{
    ASSERT_EQ(0, URI::parse("http://localhost")._port);
    ASSERT_EQ(80, URI::parse("http://localhost:80")._port);
}

TEST(URI, testAuthority)
{
    ASSERT_EQ("localhost", URI::parse("http://localhost").authority());
    ASSERT_EQ("localhost:80", URI::parse("http://localhost:80").authority());
}

TEST(URI, testPrint)
{
    std::ostringstream oss;
    oss << URI::parse("localhost");
    ASSERT_EQ(
        "{ scheme=\"\""
        ", host=\"\""
        ", port=0"
        ", path=\"localhost\""
        ", query=\"\" }",
        oss.str());
}

TEST(URI, queryEscape)
{
    ASSERT_EQ("hello%20world", URI::queryEscape("hello world"));
    ASSERT_EQ("hello-world", URI::queryEscape("hello-world"));
    ASSERT_EQ("hello.world", URI::queryEscape("hello.world"));
    ASSERT_EQ("hello_world", URI::queryEscape("hello_world"));
    ASSERT_EQ("hello~world", URI::queryEscape("hello~world"));
}

TEST(URI, queryUnescape)
{
    ASSERT_EQ("hello world", URI::queryUnescape("hello%20world"));
    ASSERT_EQ("hello%2world", URI::queryUnescape("hello%2world"));
    ASSERT_EQ("hello%world", URI::queryUnescape("hello%world"));
    ASSERT_EQ("hello world", URI::queryUnescape("hello w%6Frld"));
    ASSERT_EQ("hello world", URI::queryUnescape("hello w%6frld"));
}

TEST(URI, testResolveAddress)
{
    ASSERT_NO_THROW(resolveAddress("http://localhost", SOCK_STREAM));
    ASSERT_NO_THROW(resolveAddress("http://localhost:80", SOCK_STREAM));
    ASSERT_NO_THROW(resolveAddress("http://123456", SOCK_STREAM));
    ASSERT_THROW(resolveAddress("http://localhost", -1), std::runtime_error);
}

}  // namespace net
}  // namespace jaeger
}  // namespace uber

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

#include "uber/jaeger/net/Socket.h"

namespace uber {
namespace jaeger {
namespace net {

TEST(Socket, testFailOpen)
{
    Socket socket;
    ASSERT_THROW(socket.open(-5, -10), std::system_error);
}

TEST(Socket, testFailBind)
{
    Socket socket;
    socket.open(AF_INET, SOCK_STREAM);
    ASSERT_THROW(socket.bind(IPAddress::v4("127.0.0.1", 1)), std::system_error);
}

TEST(Socket, testFailConnect)
{
    Socket socket;
    socket.open(AF_INET, SOCK_STREAM);
    ASSERT_THROW(socket.connect(IPAddress::v4("127.0.0.1", 12345)),
                 std::runtime_error);
}

TEST(Socket, testFailConnectURI)
{
    Socket socket;
    socket.open(AF_INET, SOCK_STREAM);
    const auto uri = URI::parse("http://127.0.0.1:12345");
    ASSERT_THROW(socket.connect(uri), std::runtime_error);
}

TEST(Socket, testFailListen)
{
    Socket socket;
    socket.open(AF_INET, SOCK_DGRAM);
    ASSERT_THROW(socket.listen(), std::system_error);
}

TEST(Socket, testFailAccept)
{
    Socket socket;
    socket.open(AF_INET, SOCK_DGRAM);
    ASSERT_THROW(socket.accept(), std::system_error);
}

}  // namespace net
}  // namespace jaeger
}  // namespace uber

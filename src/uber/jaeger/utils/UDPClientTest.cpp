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

#include <future>
#include <thread>

#include "uber/jaeger/utils/UDPClient.h"

namespace uber {
namespace jaeger {
namespace utils {

TEST(UDPClient, testZipkinMessage)
{
    net::IPAddress serverAddr;
    std::promise<void> started;
    std::thread serverThread([&serverAddr, &started]() {
        net::Socket socket;
        socket.open(AF_INET, SOCK_DGRAM);
        socket.bind(net::IPAddress::v4("127.0.0.1", 0));
        ::sockaddr_storage addrStorage;
        ::socklen_t addrLen = sizeof(addrStorage);
        const auto returnCode =
            ::getsockname(socket.handle(),
                          reinterpret_cast<::sockaddr*>(&addrStorage),
                          &addrLen);
        ASSERT_EQ(0, returnCode);
        serverAddr = net::IPAddress(addrStorage, addrLen);
        started.set_value();
    });

    started.get_future().wait();
    UDPClient udpClient(serverAddr, 0);
    ASSERT_THROW(udpClient.emitZipkinBatch({}), std::logic_error);
    serverThread.join();
}

TEST(UDPClient, testBigMessage)
{
    net::IPAddress serverAddr;
    std::promise<void> started;
    std::thread serverThread([&serverAddr, &started]() {
        net::Socket socket;
        socket.open(AF_INET, SOCK_DGRAM);
        socket.bind(net::IPAddress::v4("127.0.0.1", 0));
        ::sockaddr_storage addrStorage;
        ::socklen_t addrLen = sizeof(addrStorage);
        const auto returnCode =
            ::getsockname(socket.handle(),
                          reinterpret_cast<::sockaddr*>(&addrStorage),
                          &addrLen);
        ASSERT_EQ(0, returnCode);
        serverAddr = net::IPAddress(addrStorage, addrLen);
        started.set_value();
    });

    started.get_future().wait();
    UDPClient udpClient(serverAddr, 1);
    ASSERT_THROW(udpClient.emitBatch(thrift::Batch()), std::logic_error);
    serverThread.join();
}

}  // namespace utils
}  // namespace jaeger
}  // namespace uber

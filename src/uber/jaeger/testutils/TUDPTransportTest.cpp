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

#include <thread>

#include <gtest/gtest.h>

#include "uber/jaeger/testutils/TUDPTransport.h"

namespace uber {
namespace jaeger {
namespace testutils {
namespace {

constexpr auto kBufferSize = 256;

}  // anonymous namespace

TEST(TUDPTransport, testUDPTransport)
{
    using udp = boost::asio::ip::udp;

    const std::string message("test");

    boost::asio::io_service io;
    TUDPTransport server(io, "127.0.0.1:0");

    ASSERT_NO_THROW({ server.open(); });
    ASSERT_TRUE(server.isOpen());

    const auto serverEndpoint = server.addr();
    std::thread clientThread([&io, serverEndpoint, message]() {
        udp::socket connUDP(io);
        connUDP.open(serverEndpoint.protocol());
        const auto numWritten =
            connUDP.send_to(boost::asio::buffer(message), serverEndpoint);
        ASSERT_EQ(numWritten, message.size());

        std::array<char, kBufferSize> buffer;
        const auto numRead =
            connUDP.receive(
                boost::asio::buffer(std::begin(buffer), kBufferSize));
        const std::string received(
            std::begin(buffer), std::begin(buffer) + numRead);
        ASSERT_EQ(message.size(), numRead);
        ASSERT_EQ(message, received);

        connUDP.close();
    });

    std::array<uint8_t, kBufferSize> buffer;
    const auto numRead = server.readAll(std::begin(buffer), message.size());
    ASSERT_LT(0, numRead);
    server.write(std::begin(buffer), numRead);

    clientThread.join();
}

}  // namespace testutils
}  // namespace jaeger
}  // namespace uber

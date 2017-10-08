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
    const std::string message("test");

    TUDPTransport server(utils::net::IPAddress::v4("127.0.0.1", 12345));
    server.open();  // Not necessary. Just making sure this is called.
    ASSERT_TRUE(server.isOpen());

    const auto serverAddr = server.addr();
    std::thread clientThread([serverAddr, message]() {
        utils::net::Socket connUDP;
        connUDP.open(SOCK_DGRAM);
        const auto numWritten =
            ::sendto(connUDP.handle(),
                     message.c_str(),
                     message.size(),
                     0,
                     reinterpret_cast<const ::sockaddr*>(
                        &serverAddr.addr()),
                     serverAddr.addrLen());
        ASSERT_EQ(numWritten, message.size());
        std::cout << "CLIENT WROTE " << numWritten << " TO " << serverAddr
                  << '\n';

        std::array<char, kBufferSize> buffer;
        const auto numRead =
            ::recvfrom(connUDP.handle(),
                       &buffer[0],
                       buffer.size(),
                       0,
                       nullptr,
                       0);
        const std::string received(&buffer[0], &buffer[numRead]);
        ASSERT_EQ(message.size(), numRead);
        ASSERT_EQ(message, received);
        std::cout << "CLIENT READ " << numRead << '\n';

        connUDP.close();
    });

    std::array<uint8_t, kBufferSize> buffer;
    const auto numRead = server.readAll(&buffer[0], message.size());
    ASSERT_LT(0, numRead);
    server.write(&buffer[0], numRead);

    clientThread.join();
    server.close();
}

}  // namespace testutils
}  // namespace jaeger
}  // namespace uber

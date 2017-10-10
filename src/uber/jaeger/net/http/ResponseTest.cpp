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

#include "uber/jaeger/Constants.h"
#include "uber/jaeger/net/Socket.h"
#include "uber/jaeger/net/http/Response.h"

namespace uber {
namespace jaeger {
namespace net {
namespace http {

TEST(Response, get)
{
    IPAddress serverAddress;
    std::promise<void> started;

    std::thread clientThread([&serverAddress, &started]() {
        started.get_future().wait();
        get(URI::parse("http://" + serverAddress.authority()));
    });

    Socket socket;
    socket.open(AF_INET, SOCK_STREAM);
    socket.bind(IPAddress::v4("127.0.0.1", 0));
    socket.listen();

    ::sockaddr_storage addrStorage;
    ::socklen_t addrLen = sizeof(addrStorage);
    const auto returnCode = ::getsockname(
        socket.handle(), reinterpret_cast<::sockaddr*>(&addrStorage), &addrLen);
    ASSERT_EQ(0, returnCode);
    serverAddress = IPAddress(addrStorage, addrLen);

    started.set_value();

    auto clientSocket = socket.accept();
    std::array<char, 256> buffer;
    const auto numRead =
        ::read(clientSocket.handle(), &buffer[0], buffer.size());

    std::ostringstream oss;
    oss << "GET / HTTP/1.1\r\n"
           "Host: 127.0.0.1:"
        << serverAddress.port() << "\r\n"
        << "User-Agent: jaeger/" << kJaegerClientVersion << "\r\n\r\n";
    ASSERT_EQ(oss.str(), std::string(&buffer[0], numRead));

    oss.str("");
    oss.clear();

    std::string response("HTTP/1.1 200 OK\r\n\r\n");
    response.append(buffer.size() * 2, '0');
    const auto numWritten =
        ::write(clientSocket.handle(), response.c_str(), response.size());
    ASSERT_EQ(response.size(), numWritten);

    clientThread.join();
}

}  // namespace http
}  // namespace net
}  // namespace jaeger
}  // namespace uber

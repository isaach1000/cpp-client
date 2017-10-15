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

#ifndef JAEGERTRACING_TESTUTILS_TUDPTRANSPORT_H
#define JAEGERTRACING_TESTUTILS_TUDPTRANSPORT_H

#include <sys/socket.h>

#include <thrift/transport/TVirtualTransport.h>

#include "jaegertracing/utils/UDPClient.h"

namespace jaegertracing {
namespace testutils {

class TUDPTransport
    : public apache::thrift::transport::TVirtualTransport<TUDPTransport> {
  public:
    explicit TUDPTransport(const net::IPAddress& addr)
        : _socket()
        , _serverAddr(addr)
    {
        _socket.open(AF_INET, SOCK_DGRAM);
        _socket.bind(_serverAddr);
        if (_serverAddr.port() == 0) {
            ::sockaddr_storage addrStorage;
            ::socklen_t addrLen = sizeof(addrStorage);
            const auto returnCode =
                ::getsockname(_socket.handle(),
                              reinterpret_cast<::sockaddr*>(&addrStorage),
                              &addrLen);
            if (returnCode == 0) {
                _serverAddr = net::IPAddress(addrStorage, addrLen);
            }
        }
    }

    bool isOpen() override { return _socket.handle() >= 0; }

    void open() override {}

    void close() override { _socket.close(); }

    net::IPAddress addr() const { return _serverAddr; }

    uint32_t read(uint8_t* buf, uint32_t len)
    {
        ::sockaddr_storage clientAddr;
        auto clientAddrLen = static_cast<::socklen_t>(sizeof(clientAddr));
        const auto numRead =
            ::recvfrom(_socket.handle(),
                       buf,
                       len,
                       0,
                       reinterpret_cast<::sockaddr*>(&clientAddr),
                       &clientAddrLen);
        _clientAddr = net::IPAddress(clientAddr, clientAddrLen);
        return numRead;
    }

    void write(const uint8_t* buf, uint32_t len)
    {
        ::sendto(_socket.handle(),
                 buf,
                 len,
                 0,
                 reinterpret_cast<const ::sockaddr*>(&_clientAddr.addr()),
                 _clientAddr.addrLen());
    }

  private:
    net::Socket _socket;
    net::IPAddress _serverAddr;
    net::IPAddress _clientAddr;
    ::socklen_t _clientAddrLen;
};

}  // namespace testutils
}  // namespace jaegertracing

#endif  // JAEGERTRACING_TESTUTILS_TUDPTRANSPORT_H

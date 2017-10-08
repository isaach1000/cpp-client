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

#ifndef UBER_JAEGER_TESTUTILS_TUDPTRANSPORT_H
#define UBER_JAEGER_TESTUTILS_TUDPTRANSPORT_H

#include <thrift/transport/TVirtualTransport.h>

#include "uber/jaeger/utils/UDPClient.h"

namespace uber {
namespace jaeger {
namespace testutils {

class TUDPTransport
    : public apache::thrift::transport::TVirtualTransport<TUDPTransport> {
  public:
    TUDPTransport(const std::string& ip, int port)
        : TUDPTransport(utils::net::makeAddress(ip, port))
    {
    }

    TUDPTransport(const ::sockaddr& addr)
        : _socket()
        , _serverAddr(addr)
    {
        _socket.open(SOCK_DGRAM);
        _socket.bind(_serverAddr);
    }

    bool isOpen() override { return _socket.handle() >= 0; }

    void open() override
    {
    }

    void close() override
    {
        _socket.close();
    }

    const ::sockaddr& addr() const
    {
        return reinterpret_cast<const ::sockaddr&>(_serverAddr);
    }

    uint32_t read(uint8_t* buf, uint32_t len)
    {
        return ::recvfrom(_socket.handle(),
                          buf,
                          len,
                          0,
                          reinterpret_cast<::sockaddr*>(&_clientAddr),
                          &_clientAddrLen);
    }

    void write(const uint8_t* buf, uint32_t len)
    {
        ::sendto(_socket.handle(),
                 buf,
                 len,
                 0,
                 reinterpret_cast<::sockaddr*>(&_clientAddr),
                 _clientAddrLen);
    }

  private:
    utils::net::Socket _socket;
    ::sockaddr_storage _serverAddr;
    ::sockaddr_storage _clientAddr;
    ::socklen_t _clientAddrLen;
};

}  // namespace testutils
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_TESTUTILS_TUDPTRANSPORT_H

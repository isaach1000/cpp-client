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

#ifndef UBER_JAEGER_NET_TCPCONNECTION_H
#define UBER_JAEGER_NET_TCPCONNECTION_H

#include <memory>

#include "uber/jaeger/net/Connection.h"

namespace uber {
namespace jaeger {
namespace net {

class TCPAddress final : public IPv4Address {
  public:
    TCPAddress()
        : IPv4Address()
        , _port(0)
    {
    }

    TCPAddress(uint32_t ip, int port)
        : IPv4Address(ip)
        , _port(port)
    {
    }

    std::string network() const noexcept override { return "tcp"; }

    std::string str() const noexcept override
    {
        return this->IPv4Address::str() + ':' + std::to_string(_port);
    }

    ::sockaddr_in handle() const override
    {
        auto addr = this->IPv4Address::handle();
        addr.sin_port = htons(_port);
        return addr;
    }

    int port() const { return _port; }

  private:
    int _port;
};

class TCPConnection : public Connection {
  public:
    ~TCPConnection();

    int read(char* buffer, int maxSize) override;

    int write(const std::string& buffer) override;

    void close() override;

    const Address& localAddress() const noexcept override;

    const Address& remoteAddress() const noexcept override;

  private:
    struct Impl;

    std::unique_ptr<Impl> _impl;
};

}  // namespace net
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_NET_TCPCONNECTION_H

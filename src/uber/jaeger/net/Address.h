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

#ifndef UBER_JAEGER_NET_ADDRESS_H
#define UBER_JAEGER_NET_ADDRESS_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cassert>
#include <cstdio>
#include <cstring>
#include <array>
#include <string>

namespace uber {
namespace jaeger {
namespace net {

class Address {
  public:
    virtual ~Address() = default;

    virtual std::string network() const noexcept = 0;

    virtual std::string str() const noexcept = 0;
};

class IPv4Address : public Address {
  public:
    static constexpr auto kLocalhost = static_cast<uint32_t>(0x7f000001);

    IPv4Address()
        : _ip(kLocalhost)
    {
    }

    explicit IPv4Address(uint32_t ip)
        : _ip(ip)
    {
    }

    virtual ~IPv4Address() = default;

    uint32_t ip() const { return _ip; }

    virtual std::string str() const noexcept override
    {
        unsigned char byte[4];
        byte[0] = (_ip >> 24) & 0xff;
        byte[1] = (_ip >> 16) & 0xff;
        byte[2] = (_ip >> 8) & 0xff;
        byte[3] = _ip & 0xff;
        char buffer[16];
        auto n = std::snprintf(&buffer[0],
                          sizeof(buffer),
                          "%u.%u.%u.%u",
                          byte[0],
                          byte[1],
                          byte[2],
                          byte[3]);
        assert(n < sizeof(buffer) && n > 0);
        return std::string(&buffer[0], &buffer[n]);
    }

    virtual ::sockaddr_in handle() const
    {
        ::sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(_ip);
        return addr;
    }

  private:
    uint32_t _ip;
};

class HostPort : public IPv4Address {
  public:
    HostPort()
        : IPv4Address()
        , _port(0)
    {
    }

    ~HostPort() = default;

    HostPort(uint32_t ip, int port)
        : IPv4Address(ip)
        , _port(port)
    {
    }

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

}  // namespace net
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_NET_CONNECTION_H

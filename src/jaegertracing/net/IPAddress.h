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

#ifndef JAEGERTRACING_NET_IPADDRESS_H
#define JAEGERTRACING_NET_IPADDRESS_H

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <array>
#include <cassert>
#include <cstring>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <system_error>
#include <vector>

struct ifaddrs;

namespace jaegertracing {
namespace net {

class IPAddress {
  public:
    static IPAddress v4(const std::string& ip, int port)
    {
        return versionFromString(ip, port, AF_INET);
    }

    static IPAddress v6(const std::string& ip, int port)
    {
        return versionFromString(ip, port, AF_INET6);
    }

    static IPAddress host(int family);

    static IPAddress host(std::function<bool(const ifaddrs*)> filter);

    IPAddress()
        : _addr()
        , _addrLen(sizeof(::sockaddr_in))
    {
        std::memset(&_addr, 0, sizeof(_addr));
    }

    IPAddress(const ::sockaddr_storage& addr, ::socklen_t addrLen)
        : _addr(addr)
        , _addrLen(addrLen)
    {
    }

    IPAddress(const ::sockaddr& addr, ::socklen_t addrLen)
        : IPAddress(reinterpret_cast<const ::sockaddr_storage&>(addr), addrLen)
    {
    }

    explicit IPAddress(const ::sockaddr_in& addr)
        : IPAddress(reinterpret_cast<const ::sockaddr&>(addr), sizeof(addr))
    {
    }

    explicit IPAddress(const ::sockaddr_in6& addr)
        : IPAddress(reinterpret_cast<const ::sockaddr&>(addr), sizeof(addr))
    {
    }

    const ::sockaddr_storage& addr() const { return _addr; }

    ::socklen_t addrLen() const { return _addrLen; }

    void print(std::ostream& out) const
    {
        out << "{ family=" << family();
        const auto addrStr = host();
        if (!addrStr.empty()) {
            out << ", addr=" << addrStr;
        }
        out << ", port=" << port() << " }";
    }

    std::string authority() const
    {
        const auto portNum = port();
        if (portNum != 0) {
            return host() + ':' + std::to_string(portNum);
        }
        return host();
    }

    std::string host() const
    {
        std::array<char, INET6_ADDRSTRLEN> buffer;
        const auto af = family();
        const auto* addrStr = ::inet_ntop(
            af,
            af == AF_INET
                ? static_cast<const void*>(
                      &reinterpret_cast<const ::sockaddr_in&>(_addr).sin_addr)
                : static_cast<const void*>(
                      &reinterpret_cast<const ::sockaddr_in6&>(_addr)
                           .sin6_addr),
            &buffer[0],
            buffer.size());
        return addrStr ? addrStr : "";
    }

    int port() const
    {
        if (family() == AF_INET) {
            return ntohs(
                reinterpret_cast<const ::sockaddr_in&>(_addr).sin_port);
        }
        return ntohs(reinterpret_cast<const ::sockaddr_in6&>(_addr).sin6_port);
    }

    int family() const
    {
        if (_addrLen == sizeof(::sockaddr_in)) {
            return AF_INET;
        }
        assert(_addrLen == sizeof(::sockaddr_in6));
        return AF_INET6;
    }

  private:
    static IPAddress
    versionFromString(const std::string& ip, int port, int family)
    {
        ::sockaddr_storage addrStorage;
        std::memset(&addrStorage, 0, sizeof(addrStorage));

        auto* addrBuffer = static_cast<void*>(nullptr);
        if (family == AF_INET) {
            ::sockaddr_in& addr =
                *reinterpret_cast<::sockaddr_in*>(&addrStorage);
            addr.sin_family = family;
            addr.sin_port = htons(port);
            addrBuffer = &addr.sin_addr;
        }
        else {
            assert(family == AF_INET6);
            ::sockaddr_in6& addr =
                *reinterpret_cast<::sockaddr_in6*>(&addrStorage);
            addr.sin6_family = family;
            addr.sin6_port = htons(port);
            addrBuffer = &addr.sin6_addr;
        }

        const auto returnCode = inet_pton(family, ip.c_str(), addrBuffer);
        if (returnCode == 0) {
            std::ostringstream oss;
            oss << "Invalid IP address"
                   ", ip="
                << ip << ", port=" << port;
            throw std::invalid_argument(oss.str());
        }
        return IPAddress(addrStorage,
                         family == AF_INET ? sizeof(::sockaddr_in)
                                           : sizeof(::sockaddr_in6));
    }

    ::sockaddr_storage _addr;
    ::socklen_t _addrLen;
};

}  // namespace net
}  // namespace jaegertracing

inline std::ostream& operator<<(std::ostream& out,
                                const jaegertracing::net::IPAddress& addr)
{
    addr.print(out);
    return out;
}

#endif  // JAEGERTRACING_NET_IPADDRESS_H

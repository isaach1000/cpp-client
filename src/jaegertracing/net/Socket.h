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

#ifndef JAEGERTRACING_NET_SOCKET_H
#define JAEGERTRACING_NET_SOCKET_H

#include "jaegertracing/net/IPAddress.h"
#include "jaegertracing/net/URI.h"

namespace jaegertracing {
namespace net {

class Socket {
  public:
    Socket()
        : _handle(-1)
        , _family(-1)
        , _type(-1)
    {
    }

    Socket(const Socket&) = delete;

    Socket& operator=(const Socket&) = delete;

    Socket(Socket&& socket)
        : _handle(socket._handle)
        , _family(socket._family)
        , _type(socket._type)
    {
    }

    Socket& operator=(Socket&& rhs)
    {
        _handle = rhs._handle;
        _family = rhs._family;
        _type = rhs._type;
        return *this;
    }

    ~Socket() { close(); }

    void open(int family, int type)
    {
        const auto handle = ::socket(family, type, 0);
        if (handle < 0) {
            std::ostringstream oss;
            oss << "Failed to open socket"
                   ", family="
                << family << ", type=" << type;
            throw std::system_error(errno, std::generic_category(), oss.str());
        }
        _handle = handle;
        _family = family;
        _type = type;
    }

    void bind(const IPAddress& addr)
    {
        const auto returnCode =
            ::bind(_handle,
                   reinterpret_cast<const ::sockaddr*>(&addr.addr()),
                   addr.addrLen());
        if (returnCode != 0) {
            std::ostringstream oss;
            oss << "Failed to bind socket to address"
                   ", addr=";
            addr.print(oss);
            throw std::system_error(errno, std::generic_category(), oss.str());
        }
    }

    void bind(const std::string& ip, int port)
    {
        const auto addr = IPAddress::v4(ip, port);
        bind(addr);
    }

    void connect(const IPAddress& serverAddr)
    {
        const auto returnCode =
            ::connect(_handle,
                      reinterpret_cast<const ::sockaddr*>(&serverAddr.addr()),
                      serverAddr.addrLen());
        if (returnCode != 0) {
            std::ostringstream oss;
            oss << "Cannot connect socket to remote address ";
            serverAddr.print(oss);
            throw std::runtime_error(oss.str());
        }
    }

    IPAddress connect(const std::string& serverURIStr)
    {
        return connect(URI::parse(serverURIStr));
    }

    IPAddress connect(const URI& serverURI)
    {
        auto result = resolveAddress(serverURI, _type);
        for (const auto* itr = result.get(); itr; itr = itr->ai_next) {
            const auto returnCode =
                ::connect(_handle, itr->ai_addr, itr->ai_addrlen);
            if (returnCode == 0) {
                return IPAddress(*itr->ai_addr, itr->ai_addrlen);
            }
        }
        std::ostringstream oss;
        oss << "Cannot connect socket to remote address ";
        serverURI.print(oss);
        throw std::runtime_error(oss.str());
    }

    static constexpr auto kDefaultBacklog = 128;

    void listen(int backlog = kDefaultBacklog)
    {
        const auto returnCode = ::listen(_handle, backlog);
        if (returnCode != 0) {
            throw std::system_error(
                errno, std::generic_category(), "Failed to listen on socket");
        }
    }

    Socket accept()
    {
        ::sockaddr_storage addrStorage;
        ::socklen_t addrLen = sizeof(addrStorage);
        const auto clientHandle = ::accept(
            _handle, reinterpret_cast<::sockaddr*>(&addrStorage), &addrLen);
        if (clientHandle < 0) {
            throw std::system_error(
                errno, std::generic_category(), "Failed to accept on socket");
        }

        Socket clientSocket;
        clientSocket._handle = clientHandle;
        clientSocket._family =
            (addrLen == sizeof(::sockaddr_in)) ? AF_INET : AF_INET6;
        clientSocket._type = SOCK_STREAM;
        return clientSocket;
    }

    void close()
    {
        if (_handle >= 0) {
            ::close(_handle);
            _handle = -1;
        }
    }

    int handle() { return _handle; }

  private:
    int _handle;
    int _family;
    int _type;
};

static constexpr auto kUDPPacketMaxLength = 65000;

}  // namespace net
}  // namespace jaegertracing

#endif  // JAEGERTRACING_NET_SOCKET_H

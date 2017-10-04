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

#ifndef UBER_JAEGER_UTILS_NET_H
#define UBER_JAEGER_UTILS_NET_H

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <array>
#include <sstream>
#include <stdexcept>
#include <system_error>
#include <vector>

inline std::ostream& operator<<(std::ostream& out, const ::sockaddr_in& addr)
{
    std::array<char, INET6_ADDRSTRLEN> buffer;
    const auto* addrStr =
        ::inet_ntop(addr.sin_family, &addr.sin_addr, &buffer[0], buffer.size());
    out << "{ family=" << static_cast<int>(addr.sin_family);
    if (addrStr) {
        out << ", addr=" << addrStr;
    }
    out << ", port=" << ntohs(addr.sin_port) << " }";
    return out;
}

namespace uber {
namespace jaeger {
namespace utils {
namespace net {

struct URI {
    static URI parse(const std::string& uriStr);

    URI()
        : _scheme()
        , _host()
        , _port(0)
        , _path()
        , _query()
    {
    }

    std::string _scheme;
    std::string _host;
    int _port;
    std::string _path;
    std::string _query;
};

inline ::sockaddr_in makeAddress(const std::string& ip, int port)
{
    ::sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    const auto returnCode =
        inet_pton(addr.sin_family, ip.c_str(), &addr.sin_addr);
    if (returnCode < 0) {
        if (returnCode == 0) {
            std::ostringstream oss;
            oss << "Invalid IP address"
                   ", ip=" << ip
                << ", port=" << port;
            throw std::invalid_argument(oss.str());
        }
        std::ostringstream oss;
        oss << "Failed to parse IP address (inet_pton)"
               ", ip=" << ip
            << ", port=" << port;
        throw std::system_error(errno,
                                std::generic_category(),
                                oss.str());
    }
    return addr;
}

struct AddrInfoDeleter : public std::function<void(::addrinfo*)> {
    void operator()(::addrinfo* addrInfo) const
    {
        ::freeaddrinfo(addrInfo);
    }
};

std::unique_ptr<::addrinfo, AddrInfoDeleter>
resolveAddress(const std::string& uriStr, int socketType);

class Socket {
  public:
    Socket()
        : _handle(-1)
        , _type(-1)
    {
    }

    Socket(const Socket&) = delete;

    Socket& operator=(const Socket&) = delete;

    ~Socket()
    {
        close();
    }

    void open(int type)
    {
        const auto handle = ::socket(AF_INET, type, 0);
        if (handle < 0) {
            std::ostringstream oss;
            oss << "Failed to open socket"
                << ", type=" << type;
            throw std::system_error(errno,
                                    std::generic_category(),
                                    oss.str());
        }
        _handle = handle;
        _type = type;
    }

    void bind(const ::sockaddr_in& addr)
    {
        const auto returnCode =
            ::bind(_handle,
                   reinterpret_cast<const ::sockaddr*>(&addr),
                   sizeof(addr));
        if (returnCode != 0) {
            std::ostringstream oss;
            oss << "Failed to bind socket to address"
                   ", addr=" << addr;
            throw std::system_error(errno,
                                    std::generic_category(),
                                    oss.str());
        }
    }

    void bind(const std::string& ip, int port)
    {
        const auto addr = makeAddress(ip, port);
        bind(addr);
    }

    void connect(const ::sockaddr_in& serverAddr)
    {
        const auto returnCode =
            ::connect(_handle,
                      reinterpret_cast<const ::sockaddr*>(&serverAddr),
                      sizeof(serverAddr));
        if (returnCode != 0) {
            std::ostringstream oss;
            oss << "Cannot connect socket to remote address "
                << serverAddr;
            throw std::runtime_error(oss.str());
        }
    }

    void connect(const std::string& serverAddr)
    {
        auto result = resolveAddress(serverAddr, _type);
        for (const auto* itr = result.get(); itr; itr = itr->ai_next) {
            const auto returnCode =
                ::connect(_handle, itr->ai_addr, itr->ai_addrlen);
            if (returnCode == 0) {
                return;
            }
        }
        std::ostringstream oss;
        oss << "Cannot connect socket to remote address "
            << serverAddr;
        throw std::runtime_error(oss.str());
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
    int _type;
};

static constexpr auto kUDPPacketMaxLength = 65000;

}  // namespace net
}  // namespace utils
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_UTILS_NET_H

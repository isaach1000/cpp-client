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

#ifndef UBER_JAEGER_NET_CONNECTIONIMPL_H
#define UBER_JAEGER_NET_CONNECTIONIMPL_H

#include <unistd.h>

#include <system_error>

#include "uber/jaeger/net/Connection.h"

namespace uber {
namespace jaeger {
namespace net {

class ConnectionImpl {
  public:
    explicit ConnectionImpl(int type)
        : _fd(::socket(AF_INET, type, 0))
    {
    }

    void connect()
    {
        if (_fd < 0) {
            throw std::system_error(errno,
                                    std::generic_category(),
                                    "Failed to create UDP socket");
        }

        auto localAddrHandle = localAddr().handle();
        auto returnCode =
            ::bind(_fd,
                   reinterpret_cast<::sockaddr*>(&localAddrHandle),
                   sizeof(localAddrHandle));
        if (returnCode < 0) {
            throw std::system_error(errno,
                                    std::generic_category(),
                                    "Failed to bind UDP socket");
        }

        auto remoteAddrHandle = remoteAddr().handle();
        returnCode =
            ::connect(_fd,
                      reinterpret_cast<::sockaddr*>(&remoteAddrHandle),
                      sizeof(remoteAddrHandle));
        if (returnCode < 0) {
            throw std::system_error(errno,
                                    std::generic_category(),
                                    "Failed to connect UDP socket");
        }
    }

    ~ConnectionImpl()
    {
        if (_fd >= 0) {
            close();
        }
    }

    int read(char* buffer, int maxSize)
    {
        return ::read(_fd, buffer, maxSize);
    }

    int write(const std::string& buffer)
    {
        return ::write(_fd, buffer.c_str(), buffer.size());
    }

    void close()
    {
        ::close(_fd);
        _fd = -1;
    }

  protected:
    virtual HostPort& localAddr() = 0;

    virtual HostPort& remoteAddr() = 0;

  private:
    int _fd;
};

}  // namespace net
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_NET_CONNECTIONIMPL_H

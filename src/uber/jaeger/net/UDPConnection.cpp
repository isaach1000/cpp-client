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

#include "uber/jaeger/net/UDPConnection.h"

#include "uber/jaeger/net/ConnectionImpl.h"

namespace uber {
namespace jaeger {
namespace net {
namespace {

struct UDPAddress final : public HostPort {
    using HostPort::HostPort;

    std::string network() const noexcept override { return "udp"; }
};

}  // anonymous namespace

struct UDPConnection::Impl final : public ConnectionImpl {
    Impl()
        : ConnectionImpl(SOCK_DGRAM)
    {
        connect();
    }

    HostPort& localAddr() { return _localAddr; }

    HostPort& remoteAddr() { return _remoteAddr; }

    UDPAddress _localAddr;
    UDPAddress _remoteAddr;
};

UDPConnection::~UDPConnection() = default;

int UDPConnection::read(char* buffer, int maxSize)
{
    return _impl->read(buffer, maxSize);
}

int UDPConnection::write(const std::string& buffer)
{
    return _impl->write(buffer);
}

void UDPConnection::close()
{
    return _impl->close();
}

const Address& UDPConnection::localAddress() const noexcept
{
    return _impl->_localAddr;
}

const Address& UDPConnection::remoteAddress() const noexcept
{
    return _impl->_remoteAddr;
}

}  // namespace net
}  // namespace jaeger
}  // namespace uber

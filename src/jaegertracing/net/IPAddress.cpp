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

#include "jaegertracing/net/IPAddress.h"

#include <ifaddrs.h>
#include <sys/types.h>

namespace jaegertracing {
namespace net {
namespace {

struct IfAddrDeleter : public std::function<void(ifaddrs*)> {
    void operator()(ifaddrs* ifAddr) const
    {
        if (ifAddr) {
            ::freeifaddrs(ifAddr);
        }
    }
};

}  // anonymous namespace

IPAddress IPAddress::host(int family)
{
    return host([family](const ifaddrs* ifAddr) {
        return ifAddr->ifa_addr->sa_family == family;
    });
}

IPAddress IPAddress::host(std::function<bool(const ifaddrs*)> filter)
{
    auto* ifAddrRawPtr = static_cast<ifaddrs*>(nullptr);
    getifaddrs(&ifAddrRawPtr);
    std::unique_ptr<ifaddrs, IfAddrDeleter> ifAddr(ifAddrRawPtr);
    for (auto* itr = ifAddr.get(); itr; itr = itr->ifa_next) {
        if (filter(itr)) {
            const auto family = ifAddr->ifa_addr->sa_family;
            const auto addrLen = (family == AF_INET) ? sizeof(::sockaddr_in)
                                                     : sizeof(::sockaddr_in6);
            return IPAddress(*itr->ifa_addr, addrLen);
        }
    }
    return IPAddress();
}

}  // namespace net
}  // namespace jaegertracing

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

#ifndef UBER_JAEGER_NET_CONNECTION_H
#define UBER_JAEGER_NET_CONNECTION_H

#include <string>

#include "uber/jaeger/net/Address.h"

namespace uber {
namespace jaeger {
namespace net {

class Connection {
  public:
    virtual ~Connection() = default;

    virtual int read(char* buffer, int maxSize) = 0;

    virtual int write(const std::string& buffer) = 0;

    virtual void close() = 0;

    virtual const Address& localAddress() const noexcept = 0;

    virtual const Address& remoteAddress() const noexcept = 0;
};

}  // namespace net
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_NET_CONNECTION_H

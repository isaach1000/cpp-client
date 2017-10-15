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

#ifndef JAEGERTRACING_NET_HTTP_REQUEST_H
#define JAEGERTRACING_NET_HTTP_REQUEST_H

#include "jaegertracing/net/URI.h"
#include "jaegertracing/net/http/Error.h"
#include "jaegertracing/net/http/Header.h"
#include "jaegertracing/net/http/Method.h"

namespace jaegertracing {
namespace net {
namespace http {

class Request {
  public:
    static Request parse(std::istream& in);

    Request()
        : _method()
        , _target()
        , _version()
        , _headers()
    {
    }

    Method method() const { return _method; }

    const std::string& target() const { return _target; }

    const std::string& version() const { return _version; }

    const std::vector<Header>& headers() const { return _headers; }

  private:
    Method _method;
    std::string _target;
    std::string _version;
    std::vector<Header> _headers;
};

}  // namespace http
}  // namespace net
}  // namespace jaegertracing

#endif  // JAEGERTRACING_NET_HTTP_REQUEST_H

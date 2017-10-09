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

#ifndef UBER_JAEGER_NET_HTTP_RESPONSE_H
#define UBER_JAEGER_NET_HTTP_RESPONSE_H

#include "uber/jaeger/net/URI.h"
#include "uber/jaeger/net/http/Error.h"
#include "uber/jaeger/net/http/Header.h"
#include "uber/jaeger/net/http/Method.h"

namespace uber {
namespace jaeger {
namespace net {
namespace http {

class Response {
  public:
    static Response parse(std::istream& in);

    Response()
        : _version()
        , _statusCode(0)
        , _reason()
        , _headers()
        , _body()
    {
    }

    int statusCode() const { return _statusCode; }

    const std::string& reason() const { return _reason; }

    const std::vector<Header>& headers() const { return _headers; }

    const std::string& body() const { return _body; }

  private:
    std::string _version;
    int _statusCode;
    std::string _reason;
    std::vector<Header> _headers;
    std::string _body;
};

Response get(const URI& uri);

}  // namespace http
}  // namespace net
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_NET_HTTP_RESPONSE_H

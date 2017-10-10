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

#ifndef UBER_JAEGER_NET_URI_H
#define UBER_JAEGER_NET_URI_H

#include <netdb.h>

#include <functional>
#include <memory>
#include <sstream>
#include <string>

namespace uber {
namespace jaeger {
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

    std::string authority() const
    {
        if (_port != 0) {
            return _host + ':' + std::to_string(_port);
        }
        return _host;
    }

    std::string target() const
    {
        auto result = _path;
        if (result.empty()) {
            result = "/";
        }
        if (!_query.empty()) {
            result += '?' + _query;
        }
        return result;
    }

    template <typename Stream>
    void print(Stream& out) const
    {
        out << "{ scheme=\"" << _scheme << '"' << ", host=\"" << _host << '"'
            << ", port=" << _port << ", path=\"" << _path << '"' << ", query=\""
            << _query << '"' << " }";
    }

    std::string _scheme;
    std::string _host;
    int _port;
    std::string _path;
    std::string _query;
};

struct AddrInfoDeleter : public std::function<void(::addrinfo*)> {
    void operator()(::addrinfo* addrInfo) const { ::freeaddrinfo(addrInfo); }
};

std::unique_ptr<::addrinfo, AddrInfoDeleter> resolveAddress(const URI& uri,
                                                            int socketType);

inline std::unique_ptr<::addrinfo, AddrInfoDeleter>
resolveAddress(const std::string& uriStr, int socketType)
{
    return resolveAddress(URI::parse(uriStr), socketType);
}

}  // namespace net
}  // namespace jaeger
}  // namespace uber

inline std::ostream& operator<<(std::ostream& out,
                                const uber::jaeger::net::URI& uri)
{
    uri.print(out);
    return out;
}

#endif  // UBER_JAEGER_NET_URI_H

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

#include <string>
#include <tuple>

namespace uber {
namespace jaeger {
namespace utils {
namespace net {

struct URI {
    bool operator==(const URI& rhs) const
    {
        return _host == rhs._host && _port == rhs._port && _path == rhs._path
               && _query == rhs._query;
    }

    std::string _host;
    int _port;
    std::string _path;
    std::string _query;
};

std::string percentEncode(const std::string& input);

std::string percentDecode(const std::string& input);

URI parseURI(const std::string& uriStr);

std::string httpGetRequest(const URI& uri);

inline std::string httpGetRequest(const std::string& uriStr)
{
    return httpGetRequest(parseURI(uriStr));
}

static constexpr auto kUDPPacketMaxLength = 65000;

inline std::tuple<std::string, std::string>
parseHostPort(const std::string& hostPort)
{
    const auto colonPos = hostPort.find(':');
    if (colonPos == std::string::npos) {
        std::ostringstream oss;
        oss << "Invalid host/port string contains no colon: " << hostPort;
        throw std::logic_error(oss.str());
    }

    return std::make_tuple(hostPort.substr(0, colonPos),
                           hostPort.substr(colonPos + 1));
}

}  // namespace net
}  // namespace utils
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_UTILS_NET_H

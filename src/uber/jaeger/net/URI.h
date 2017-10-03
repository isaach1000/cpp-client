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

#include <sstream>
#include <tuple>

namespace uber {
namespace jaeger {
namespace net {

class URI {
  public:
    static URI parse(const std::string& uriStr);

    std::tuple<std::string, std::string>
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

    static std::string percentEncode(const std::string& input);

    static std::string percentDecode(const std::string& input);

    bool operator==(const URI& rhs) const
    {
        return _host == rhs._host && _port == rhs._port && _path == rhs._path
               && _query == rhs._query;
    }

    void setHost(const std::string& host) { _host = host; }

    const std::string& host() const { return _host; }

    void setPort(int port) { _port = port; }

    int port() const { return _port; }

    void setPath(const std::string& path) { _path = path; }

    const std::string& path() const { return _path; }

    void setQuery(const std::string& query) { _query = query; }

    const std::string& query() const { return _query; }

  private:
    std::string _host;
    int _port;
    std::string _path;
    std::string _query;
};

}  // namespace net
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_NET_URI_H

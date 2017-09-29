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

#ifndef UBER_JAEGER_UTILS_UDPCLIENT_H
#define UBER_JAEGER_UTILS_UDPCLIENT_H

#include <boost/asio/ip/udp.hpp>
#include <thrift/transport/TBufferTransports.h>

#include "uber/jaeger/thrift-gen/Agent.h"

namespace uber {
namespace jaeger {
namespace utils {

static constexpr auto kUDPPacketMaxLength = 65000;

inline std::tuple<std::string, std::string> parseHostPort(
    const std::string& hostPort)
{
    const auto colonPos = hostPort.find(':');
    if (colonPos == std::string::npos) {
        std::ostringstream oss;
        oss << "Invalid host/port string contains no colon: "
            << hostPort;
        throw std::logic_error(oss.str());
    }

    return std::make_tuple(hostPort.substr(0, colonPos),
                           hostPort.substr(colonPos + 1));
}

class UDPClient : public agent::thrift::AgentIf {
  public:
    using udp = boost::asio::ip::udp;

    UDPClient(boost::asio::io_service& io,
              const std::string& hostPort,
              int maxPacketSize)
        : _maxPacketSize(
            maxPacketSize >= 0 ? maxPacketSize : kUDPPacketMaxLength)
        , _buffer(_maxPacketSize)
        , _socket(io)
        , _client()
    {
        std::string host;
        std::string port;
        std::tie(host, port) = parseHostPort(hostPort);
        udp::resolver resolver(_socket.get_io_service());
        const auto entryItr =
            resolver.resolve(udp::resolver::query(host, port));
        const auto endpoint = entryItr->endpoint();
        _socket.open(endpoint.protocol());
        _socket.bind(endpoint);

        /* TODO
        boost::shared_ptr<apache::thrift::protocol::TProtocolFactory> factory(
            new apache::thrift::protocol::TCompactProtocolFactory());*/
    }

  private:
    int _maxPacketSize;
    apache::thrift::transport::TMemoryBuffer _buffer;
    udp::socket _socket;
    std::unique_ptr<agent::thrift::AgentClient> _client;
};

}  // namespace utils
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_UTILS_UDPCLIENT_H

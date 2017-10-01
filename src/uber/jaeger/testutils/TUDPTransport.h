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

#ifndef UBER_JAEGER_TESTUTILS_TUDPTRANSPORT_H
#define UBER_JAEGER_TESTUTILS_TUDPTRANSPORT_H

#include <boost/asio/ip/udp.hpp>
#include <thrift/transport/TVirtualTransport.h>

#include "uber/jaeger/utils/UDPClient.h"

namespace uber {
namespace jaeger {
namespace testutils {

class TUDPTransport
    : public apache::thrift::transport::TVirtualTransport<TUDPTransport> {
  public:
    using udp = boost::asio::ip::udp;

    TUDPTransport(boost::asio::io_service& io, const std::string& hostPort)
        : _socket(io)
        , _host()
        , _port()
        , _senderEndpoint()
    {
        std::tie(_host, _port) = utils::net::parseHostPort(hostPort);
    }

    bool isOpen() override { return _socket.is_open(); }

    void open() override
    {
        udp::resolver resolver(_socket.get_io_service());
        const auto entryItr
            = resolver.resolve(udp::resolver::query(_host, _port));
        const auto endpoint = entryItr->endpoint();
        _socket.open(endpoint.protocol());
        _socket.bind(endpoint);
    }

    void close() override { _socket.close(); }

    udp::endpoint addr() const { return _socket.local_endpoint(); }

    uint32_t read(uint8_t* buf, uint32_t len)
    {
        return _socket.receive_from(boost::asio::buffer(buf, len),
                                    _senderEndpoint);
    }

    void write(const uint8_t* buf, uint32_t len)
    {
        _socket.send_to(boost::asio::buffer(buf, len), _senderEndpoint);
    }

    boost::asio::io_service& ioService() { return _socket.get_io_service(); }

  private:
    udp::socket _socket;
    std::string _host;
    std::string _port;
    udp::endpoint _senderEndpoint;
};

}  // namespace testutils
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_TESTUTILS_TUDPTRANSPORT_H

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
        : _io(io)
        , _socket(io, utils::net::resolveHostPort<udp>(_io, hostPort))
        , _host()
        , _port()
        , _senderEndpoint()
    {
    }

    bool isOpen() override { return _socket.is_open(); }

    void open() override
    {
    }

    void close() override { _socket.close(); }

    udp::endpoint addr() const { return _socket.local_endpoint(); }

    uint32_t read(uint8_t* buf, uint32_t len)
    {
        return _socket.receive(boost::asio::buffer(buf, len));
    }

    void write(const uint8_t* buf, uint32_t len)
    {
        _socket.send(boost::asio::buffer(buf, len));
    }

    boost::asio::io_service& ioService() { return _socket.get_io_service(); }

  private:
    boost::asio::io_service& _io;
    udp::socket _socket;
    std::string _host;
    std::string _port;
    udp::endpoint _senderEndpoint;
};

}  // namespace testutils
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_TESTUTILS_TUDPTRANSPORT_H

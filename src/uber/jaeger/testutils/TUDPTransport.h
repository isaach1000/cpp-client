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
#include <thrift/transport/TBufferTransports.h>

namespace uber {
namespace jaeger {
namespace testutils {

class TUDPTransport : public apache::thrift::transport::TTransport {
  public:
    TUDPTransport(boost::asio::io_service& io,
                 const std::string& hostPort)
        : _socket(io)
    {
        const auto colonPos = hostPort.find(':');
        if (colonPos == std::string::npos) {
            std::ostringstream oss;
            oss << "Invalid host/port string contains no colon: "
                << hostPort;
            throw std::logic_error(oss.str());
        }
        _host = hostPort.substr(0, colonPos);
        _port = hostPort.substr(colonPos + 1);
    }

    bool isOpen() override { return _socket.is_open(); }

    void open() override
    {
        udp::resolver resolver(_socket.get_io_service());
        boost::system::error_code error;
        const auto entryItr =
            resolver.resolve(udp::resolver::query(_host, _port), error);
        if (error) {
            throw boost::system::system_error(error);
        }
        const auto endpoint = entryItr->endpoint();

        _socket.open(endpoint.protocol(), error);
        if (error) {
            throw boost::system::system_error(error);
        }

        _socket.bind(endpoint, error);
        if (error) {
            throw boost::system::system_error(error);
        }
    }

    void close() override { _socket.close(); }

    uint32_t read(uint8_t* buf, uint32_t len)
    {
        return _socket.receive(boost::asio::buffer(buf, len));
    }

    void write(const uint8_t* buf, uint32_t len)
    {
        _socket.send(boost::asio::buffer(buf, len));
    }

  private:
    using udp = boost::asio::ip::udp;

    udp::socket _socket;
    std::string _host;
    std::string _port;
};

}  // namespace testutils
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_TESTUTILS_TUDPTRANSPORT_H

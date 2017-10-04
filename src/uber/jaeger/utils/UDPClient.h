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

#include <sstream>
#include <stdexcept>
#include <system_error>

#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/transport/TBufferTransports.h>

#include "uber/jaeger/thrift-gen/Agent.h"
#include "uber/jaeger/utils/Net.h"

namespace uber {
namespace jaeger {
namespace utils {

class UDPClient : public agent::thrift::AgentIf {
  public:
    UDPClient(const std::string& ip, int port, int maxPacketSize)
        : UDPClient(net::makeAddress(ip, port), maxPacketSize)
    {
    }

    UDPClient(const ::sockaddr_in& serverAddr, int maxPacketSize);

    ~UDPClient()
    {
        close();
    }

    void emitZipkinBatch(
        const std::vector<twitter::zipkin::thrift::Span>& spans) override
    {
        throw std::logic_error("emitZipkinBatch not implemented");
    }

    void emitBatch(const thrift::Batch& batch) override
    {
        _buffer->resetBuffer();
        _client->emitBatch(batch);
        uint8_t* data = nullptr;
        uint32_t size = 0;
        _buffer->getBuffer(&data, &size);
        if (static_cast<int>(size) > _maxPacketSize) {
            std::ostringstream oss;
            oss << "Data does not fit within one UDP packet"
                   "; size "
                << size << ", max " << _maxPacketSize << ", spans "
                << batch.spans.size();
            throw std::logic_error(oss.str());
        }
        const auto numWritten = ::write(_socket.handle(), data, size);
        if (numWritten != size) {
            throw std::system_error(errno,
                                    std::generic_category(),
                                    "Failed to write message");
        }
    }

    int maxPacketSize() const { return _maxPacketSize; }

    boost::shared_ptr<apache::thrift::protocol::TProtocol> protocol() const
    {
        apache::thrift::protocol::TCompactProtocolFactory protocolFactory;
        return protocolFactory.getProtocol(_buffer);
    }

    void close()
    {
        _socket.close();
    }

  private:
    int _maxPacketSize;
    boost::shared_ptr<apache::thrift::transport::TMemoryBuffer> _buffer;
    net::Socket _socket;
    ::sockaddr_in _serverAddr;
    std::unique_ptr<agent::thrift::AgentClient> _client;
};

}  // namespace utils
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_UTILS_UDPCLIENT_H

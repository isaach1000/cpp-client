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

#include "uber/jaeger/utils/UDPClient.h"

namespace uber {
namespace jaeger {
namespace utils {

UDPClient(const char* ip,
          int port,
          int maxPacketSize)
    : _maxPacketSize(maxPacketSize == 0 ? net::kUDPPacketMaxLength
                                        : maxPacketSize)
    , _buffer(new apache::thrift::transport::TMemoryBuffer(_maxPacketSize))
    , _socketFD(::socket(AF_INET, SOCK_DGRAM, 0))
    , _serverAddr()
    , _client()
{
    if (_socketFD < 0) {
        throw std::system_error(
            errno, std::generic_category(), "Failed to open socket");
    }
    _socketFD = fd;

    std::memset(&_serverAddr, 0, sizeof(_serverAddr));
    auto returnCode = inet_pton(AF_INET, ip, &_serverAddr);
    if (returnCode < 0) {
        ::close(_socketFD);
        if (returnCode == 0) {
            std::ostringstream oss;
            oss << "Invalid IP format: " << ip;
            throw std::invalid_argument(oss.what());
        }
        throw std::system_error(
            errno,
            std::generic_category(),
            "Internal parse error (inet_pton)");
    }
    if (port > 0) {
        _serverAddr.sin_port = port;
    }

    returnCode = ::connect(_socketFD,
                           reinterpret_cast<::sockaddr*>(&_serverAddr),
                           sizeof(_serverAddr));
    if (returnCode != 0) {
        throw std::system_error(errno,
                                std::generic_category(),
                                "Failed to connect to UDP server");
    }

    boost::shared_ptr<TProtocolFactory> protocolFactory(
        new TCompactProtocolFactory());
    auto protocol = protocolFactory->getProtocol(_buffer);
    _client.reset(new agent::thrift::AgentClient(protocol));
}

}  // namespace utils
}  // namespace jaeger
}  // namespace uber

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

#include "uber/jaeger/testutils/MockAgent.h"

#include <regex>
#include <thread>

#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/protocol/TJSONProtocol.h>
#include <thrift/transport/TBufferTransports.h>

#include "uber/jaeger/Logging.h"
#include "uber/jaeger/utils/UDPClient.h"

namespace uber {
namespace jaeger {
namespace testutils {

MockAgent::~MockAgent()
{
    close();
}

void MockAgent::start()
{
    std::promise<void> startedUDP;
    std::promise<void> startedHTTP;
    _udpThread = std::thread([this, &startedUDP]() { serveUDP(startedUDP); });
    _httpThread = std::thread([this, &startedHTTP]() {
                                  serveHTTP(startedHTTP);
                              });
    startedUDP.get_future().wait();
    startedHTTP.get_future().wait();
}

void MockAgent::close()
{
    if (_servingUDP) {
        _servingUDP = false;
        _transport.close();
        _udpThread.join();
    }

    if (_servingHTTP) {
        _servingHTTP = false;
        _httpThread.join();
    }
}

void MockAgent::emitBatch(const thrift::Batch& batch)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _batches.push_back(batch);
}

utils::net::IPAddress MockAgent::samplingServerAddr() const
{
    // TODO
    return utils::net::IPAddress();
}

MockAgent::MockAgent()
    : _transport(utils::net::IPAddress::v4("127.0.0.1", 0))
    , _batches()
    , _servingUDP(false)
    , _samplingMgr()
    , _mutex()
    , _udpThread()
    , _httpThread()
{
}

void MockAgent::serveUDP(std::promise<void>& started)
{
    using TCompactProtocolFactory
        = apache::thrift::protocol::TCompactProtocolFactory;
    using TMemoryBuffer = apache::thrift::transport::TMemoryBuffer;

    // Trick for converting `std::shared_ptr` into `boost::shared_ptr`. See
    // https://stackoverflow.com/a/12315035/1930331.
    auto ptr = shared_from_this();
    boost::shared_ptr<agent::thrift::AgentIf> iface(
        ptr.get(), [&ptr](MockAgent*) { ptr.reset(); });
    agent::thrift::AgentProcessor handler(iface);
    TCompactProtocolFactory protocolFactory;
    boost::shared_ptr<TMemoryBuffer> trans(
        new TMemoryBuffer(utils::net::kUDPPacketMaxLength));

    // Notify main thread that setup is done.
    _servingUDP = true;
    started.set_value();

    std::array<uint8_t, utils::net::kUDPPacketMaxLength> buffer;
    while (isServingUDP()) {
        try {
            const auto numRead
                = _transport.read(&buffer[0], utils::net::kUDPPacketMaxLength);
            trans->write(&buffer[0], numRead);
            auto protocol = protocolFactory.getProtocol(trans);
            handler.process(protocol, protocol, nullptr);
        } catch (const std::exception& ex) {
            logging::consoleLogger()->error(
                "An error occurred in MockAgent, {0}", ex.what());
        }
    }
}

void MockAgent::serveHTTP(std::promise<void>& started)
{
    // TODO
    _servingHTTP = true;
    started.set_value();
}

}  // namespace testutils
}  // namespace jaeger
}  // namespace uber

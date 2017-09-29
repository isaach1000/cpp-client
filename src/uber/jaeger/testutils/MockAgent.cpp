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

#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/transport/TBufferTransports.h>

#include "uber/jaeger/utils/UDPClient.h"

namespace uber {
namespace jaeger {
namespace testutils {

void MockAgent::start()
{
    _thread = std::thread([this]() {
        serve();
    });
    std::unique_lock<std::mutex> lock(_mutex);
    _started.wait(lock, [this]() { return isServing(); });
}

void MockAgent::close()
{
    _serving = false;
    _transport.close();
    _thread.join();
}

void MockAgent::emitBatch(const thrift::Batch& batch)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _batches.push_back(batch);
}

void MockAgent::serve()
{
    using TCompactProtocolFactory =
        apache::thrift::protocol::TCompactProtocolFactory;
    using TMemoryBuffer = apache::thrift::transport::TMemoryBuffer;

    // Trick for converting std::shared_ptr into boost::shared_ptr. See
    // https://stackoverflow.com/a/12315035/1930331.
    auto ptr = shared_from_this();
    boost::shared_ptr<agent::thrift::AgentIf> iface(
        ptr.get(),
        [&ptr](MockAgent*) {
            ptr.reset();
        });
    agent::thrift::AgentProcessor handler(iface);
    TCompactProtocolFactory protocolFactory;
    boost::shared_ptr<TMemoryBuffer> trans(
        new TMemoryBuffer(utils::kUDPPacketMaxLength));

    // Notify main thread that setup is done.
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _serving = true;
    }
    _started.notify_one();

    std::array<uint8_t, utils::kUDPPacketMaxLength> buffer;
    while (isServing()) {
        try {
            auto numRead = _transport.read(
                &buffer[0], utils::kUDPPacketMaxLength);
            trans->write(&buffer[0], numRead);
            auto protocol = protocolFactory.getProtocol(trans);
            handler.process(protocol, protocol, nullptr);
        } catch (...) {
            // TODO: Exception logging/handling
        }
    }
}

}  // namespace testutils
}  // namespace jaeger
}  // namespace uber

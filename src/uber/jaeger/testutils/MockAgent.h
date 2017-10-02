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

#ifndef UBER_JAEGER_TESTUTILS_MOCKAGENT_H
#define UBER_JAEGER_TESTUTILS_MOCKAGENT_H

#include <atomic>
#include <future>
#include <mutex>
#include <thread>
#include <vector>

#include <boost/asio.hpp>

#include "uber/jaeger/testutils/SamplingManager.h"
#include "uber/jaeger/testutils/TUDPTransport.h"
#include "uber/jaeger/thrift-gen/Agent.h"
#include "uber/jaeger/thrift-gen/jaeger_types.h"
#include "uber/jaeger/utils/UDPClient.h"

namespace uber {
namespace jaeger {
namespace testutils {

class MockAgent : public agent::thrift::AgentIf,
                  public std::enable_shared_from_this<MockAgent> {
  public:
    using tcp = boost::asio::ip::tcp;
    using udp = boost::asio::ip::udp;

    static std::shared_ptr<MockAgent> make(boost::asio::io_service& io)
    {
        // Avoid `make_shared` when `weak_ptr` might be used.
        std::shared_ptr<MockAgent> newInstance(new MockAgent(io));
        return newInstance;
    }

    ~MockAgent();

    void start();

    void close();

    void
    emitZipkinBatch(const std::vector<twitter::zipkin::thrift::Span>&) override
    {
        throw std::logic_error("emitZipkinBatch not implemented");
    }

    void emitBatch(const thrift::Batch& batch) override;

    bool isServing() const { return _serving; }

    template <typename... Args>
    void addSamplingStrategy(Args&&... args)
    {
        _samplingMgr.addSamplingStrategy(std::forward<Args>(args)...);
    }

    std::vector<thrift::Batch> batches() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _batches;
    }

    udp::endpoint spanServerAddress() const { return _transport.addr(); }

    std::unique_ptr<agent::thrift::AgentIf> spanServerClient()
    {
        return std::unique_ptr<agent::thrift::AgentIf>(new utils::UDPClient(
            _transport.ioService(), spanServerAddress(), 0));
    }

    tcp::endpoint samplingServerAddr() const;

    void resetBatches()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _batches.clear();
    }

  private:
    class HTTPServer;

    explicit MockAgent(boost::asio::io_service& io);

    void serve(std::promise<void>& started);

    TUDPTransport _transport;
    std::vector<thrift::Batch> _batches;
    std::atomic<bool> _serving;
    SamplingManager _samplingMgr;
    std::unique_ptr<HTTPServer> _samplingSrv;
    mutable std::mutex _mutex;
    std::thread _thread;
};

}  // namespace testutils
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_TESTUTILS_MOCKAGENT_H

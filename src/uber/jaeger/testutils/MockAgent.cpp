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

#include <beast/core.hpp>
#include <beast/http.hpp>
#include <boost/asio.hpp>
#include <boost/tokenizer.hpp>
#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/protocol/TJSONProtocol.h>
#include <thrift/transport/TBufferTransports.h>

#include "uber/jaeger/Logging.h"
#include "uber/jaeger/utils/UDPClient.h"

namespace http = beast::http;
using tcp = boost::asio::ip::tcp;

namespace uber {
namespace jaeger {
namespace testutils {
namespace {

class HTTPConnection : public std::enable_shared_from_this<HTTPConnection> {
  public:
    static std::shared_ptr<HTTPConnection> make(boost::asio::io_service& io,
                                                SamplingManager& samplingMgr)
    {
        return std::shared_ptr<HTTPConnection>(
            new HTTPConnection(io, samplingMgr));
    }

    void start()
    {
        readRequest();
    }

    tcp::socket& socket() { return _socket; }

  private:
    HTTPConnection(boost::asio::io_service& io,
                   SamplingManager& samplingMgr)
        : _samplingMgr(samplingMgr)
        , _socket(io)
        , _buffer()
        , _request()
        , _response()
    {
    }

    void readRequest()
    {
        auto self = shared_from_this();
        http::async_read(
            _socket,
            _buffer,
            _request,
            [self](const beast::error_code& err) {
                if (err) {
                    logging::consoleLogger()->error(
                        "Read error occurred, {0}", err.message());
                    return;
                }

                self->handleRequest();
                self->writeResponse();
            });
    }

    void handleRequest()
    {
        _response.version = 11;
        _response.set(http::field::connection, "close");

        const auto uri = utils::net::parseURI(std::string(_request.target()));
        const auto& query = uri._query;
        constexpr auto kPattern = "(^service|&service)=([^&=]+)";
        std::regex serviceRegex(kPattern);
        std::smatch results;
        std::regex_search(query, results, serviceRegex);

        if (results.empty()) {
            _response.result(http::status::bad_request);
            _response.set(http::field::content_type, "text/plain");
            beast::ostream(_response.body)
                << "no 'service' parameter";
            return;
        }

        if (results.size() > 1) {
            _response.result(http::status::bad_request);
            _response.set(http::field::content_type, "text/plain");
            beast::ostream(_response.body)
                << "'service' parameter must occur only once";
            return;
        }

        const auto service = std::string(results[0].first, results[0].second);
        thrift::sampling_manager::SamplingStrategyResponse response;
        try {
            _samplingMgr.getSamplingStrategy(response, service);
        } catch (const std::exception& ex) {
            _response.result(http::status::internal_server_error);
            _response.set(http::field::content_type, "text/plain");
            beast::ostream(_response.body)
                << "Error retrieving strategy: " << ex.what();
            return;
        }

        try {
            const auto json = apache::thrift::ThriftJSONString(response);
            _response.result(http::status::ok);
            _response.set(http::field::content_type, "application/json");
        } catch (const std::exception& ex) {
            _response.result(http::status::internal_server_error);
            _response.set(http::field::content_type, "text/plain");
            beast::ostream(_response.body)
                << "Cannot marshal Thrift to JSON: " << ex.what();
        }
    }

    void writeResponse()
    {
        auto self = shared_from_this();
        _response.prepare_payload();

        http::async_write(
            _socket,
            _response,
            [self](boost::system::error_code err) {
                self->_socket.shutdown(tcp::socket::shutdown_send, err);
            });
    }

    SamplingManager& _samplingMgr;
    tcp::socket _socket;
    beast::flat_buffer _buffer;
    http::request<http::dynamic_body> _request;
    http::response<http::dynamic_body> _response;
};

}  // anonymous namespace

class MockAgent::HTTPServer {
  public:
    HTTPServer(boost::asio::io_service& io,
               SamplingManager& samplingMgr,
               const std::string& hostPort)
        : _samplingMgr(samplingMgr)
        , _acceptor(io)
        , _hostPort(hostPort)
        , _serving(false)
        , _thread()
    {
    }

    void start()
    {
        if (!_serving) {
            std::promise<void> started;
            _thread = std::thread([this, &started]() { serve(started); });
            started.get_future().wait();
        }
    }

    void close()
    {
        if (_serving) {
            _serving = false;
            _thread.join();
        }
    }

    tcp::endpoint addr() const
    {
        return _acceptor.local_endpoint();
    }

  private:
    void serve(std::promise<void>& started)
    {
        auto& io = _acceptor.get_io_service();
        const auto endpoint = utils::net::resolveHostPort<tcp>(io, _hostPort);
        _acceptor.open(endpoint.protocol());
        _acceptor.bind(endpoint);
        _acceptor.listen();

        _serving = true;
        started.set_value();

        accept();
    }

    void accept()
    {
        auto& io = _acceptor.get_io_service();
        auto conn = HTTPConnection::make(io, _samplingMgr);
        _acceptor.async_accept(
            conn->socket(),
            [this, conn](const boost::system::error_code& err) {
                handleConnection(conn, err);
            });
    }

    void handleConnection(
        const std::shared_ptr<HTTPConnection>& conn,
        const boost::system::error_code& err)
    {
        if (err) {
            logging::consoleLogger()->error(
                "Accept error, {0}", err.message());
            return;
        }

        if (!_serving) {
            conn->socket().shutdown(tcp::socket::shutdown_both);
            return;
        }

        conn->start();

        if (_serving) {
            accept();
        }
    }

    SamplingManager& _samplingMgr;
    tcp::acceptor _acceptor;
    std::string _hostPort;
    std::atomic<bool> _serving;
    std::thread _thread;
};

MockAgent::~MockAgent()
{
    close();
}

void MockAgent::start()
{
    _samplingSrv->start();
    std::promise<void> started;
    _thread = std::thread([this, &started]() { serve(started); });
    started.get_future().wait();
}

void MockAgent::close()
{
    if (_serving) {
        _serving = false;
        _transport.close();
        _thread.join();
        _samplingSrv->close();
    }
}

void MockAgent::emitBatch(const thrift::Batch& batch)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _batches.push_back(batch);
}

tcp::endpoint MockAgent::samplingServerAddr() const
{
    return _samplingSrv->addr();
}

MockAgent::MockAgent(boost::asio::io_service& io)
    : _transport(io, "127.0.0.1:0")
    , _batches()
    , _serving(false)
    , _samplingMgr()
    , _samplingSrv(new HTTPServer(io, _samplingMgr, "127.0.0.1:0"))
    , _mutex()
    , _thread()
{
}

void MockAgent::serve(std::promise<void>& started)
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
    _serving = true;
    started.set_value();

    std::array<uint8_t, utils::net::kUDPPacketMaxLength> buffer;
    while (isServing()) {
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

}  // namespace testutils
}  // namespace jaeger
}  // namespace uber

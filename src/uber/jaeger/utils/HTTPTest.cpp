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

#include <gtest/gtest.h>

#include "uber/jaeger/utils/HTTP.h"

#include <thread>

#include <beast/core.hpp>
#include <beast/http.hpp>
#include <beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace uber {
namespace jaeger {
namespace utils {

TEST(HTTP, testPercentEncode)
{
    struct TestCase {
        std::string _input;
        std::string _expected;
    };

    const TestCase testCases[] = {
        { "example", "example" },
        { "hello world-test", "hello%20world-test" }
    };

    for (auto&& testCase : testCases) {
        const auto result = http::percentEncode(testCase._input);
        ASSERT_EQ(testCase._expected, result);
    }
}

TEST(HTTP, testParseURI)
{
    struct TestCase {
        std::string _input;
        http::URI _expected;
    };

    const TestCase testCases[] = {
        { "http://example.com", {"example.com", 80, "", ""} },
        { "http://example.com:abc", {"example.com", 80, "", ""} }
    };

    for (auto&& testCase : testCases) {
        const auto uri = http::parseURI("http://example.com");
        ASSERT_EQ(testCase._expected, uri);
    }
}

TEST(HTTP, testHTTPGetRequest)
{
    using tcp = boost::asio::ip::tcp;
    using string_body = beast::http::string_body;

    constexpr auto kJSONContents = "{ \"test\": [] }";

    boost::asio::io_service io;
    const auto address = boost::asio::ip::address::from_string("0.0.0.0");
    auto port = 0;

    tcp::endpoint endpoint(address, 0);
    tcp::acceptor acceptor(io, endpoint);
    endpoint = acceptor.local_endpoint();
    port = endpoint.port();
    tcp::socket socket(io);
    acceptor.async_accept(
        socket,
        [&socket](boost::system::error_code error) {
            ASSERT_FALSE(static_cast<bool>(error));
            beast::http::request<string_body> req;
            beast::flat_buffer buffer;
            beast::http::read(socket, buffer, req, error);
            if (error) {
                std::cerr << (error) << '\n';
            }

            beast::http::response<string_body> res;
            res.set(beast::http::field::server, BEAST_VERSION_STRING);
            res.set(beast::http::field::content_type, "application/json");
            res.body = kJSONContents;
            res.content_length(res.body.size());
            res.prepare_payload();
            beast::http::response_serializer<string_body> serializer(res);
            beast::http::write(socket, serializer, error);
            ASSERT_FALSE(static_cast<bool>(error));

            socket.shutdown(tcp::socket::shutdown_both, error);
            ASSERT_TRUE(!error || error == boost::system::errc::not_connected)
                << "error: " << error;
        });

    std::thread clientThread([&io, port]() {
        std::string uriStr("http://0.0.0.0:");
        uriStr += std::to_string(port);
        const auto uri = http::parseURI(uriStr);
        const auto responseStr = http::httpGetRequest(io, uri);
        ASSERT_EQ(kJSONContents, responseStr);
    });
    io.run();
    clientThread.join();
}

}  // namespace utils
}  // namespace jaeger
}  // namespace uber

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

#include "uber/jaeger/utils/Net.h"

#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>

#include <beast/core.hpp>
#include <beast/http.hpp>
#include <beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "uber/jaeger/utils/HexParsing.h"

namespace uber {
namespace jaeger {
namespace utils {
namespace net {
namespace {

bool isUnreserved(char ch)
{
    if (std::isalpha(ch) || std::isdigit(ch)) {
        return true;
    }

    switch (ch) {
    case '-':
    case '.':
    case '_':
    case '~':
        return true;
    default:
        return false;
    }
}

}  // anonymous namespace

std::string percentEncode(const std::string& input)
{
    std::ostringstream oss;
    for (auto&& ch : input) {
        if (isUnreserved(ch)) {
            oss << ch;
        }
        else {
            oss << '%' << std::uppercase << std::hex << static_cast<int>(ch);
        }
    }
    return oss.str();
}

std::string percentDecode(const std::string& input)
{
    enum class State {
        kDefault,
        kPercent,
        kFirstDigit
    };

    std::string result;
    auto state = State::kDefault;
    auto value = 0;
    for (auto&& ch : input) {
        switch (state) {
        case State::kDefault: {
            if (ch == '%') {
                state = State::kPercent;
            }
            else {
                result += ch;
            }
        } break;
        case State::kPercent: {
            if (ch == '%') {
                result += '%';
                state = State::kDefault;
            }
            else if (HexParsing::isHex(ch)) {
                if (ch >= '0' && ch <= '9') {
                    value = ch - '0';
                }
                else if (ch >= 'A' && ch <= 'F') {
                    value = ch - 'A' + 10;
                }
                else {
                    assert(ch >= 'a' && ch <= 'f');
                    value = ch - 'a' + 10;
                }
                state = State::kFirstDigit;
            }
            else {
                result += '%';
                result += ch;
            }
        } break;
        default: {
            assert(state == State::kFirstDigit);
            if (HexParsing::isHex(ch)) {
                value <<= 4;
                if (ch >= '0' && ch <= '9') {
                    value |= ch - '0';
                }
                else if (ch >= 'A' && ch <= 'F') {
                    value |= ch - 'A' + 10;
                }
                else {
                    assert(ch >= 'a' && ch <= 'f');
                    value |= ch - 'a' + 10;
                }
                result += static_cast<char>(value);
            }
            else {
                result += '%';
                result += static_cast<char>(value);
                result += ch;
            }
            state = State::kDefault;
        } break;
        }
    }

    return result;
}

URI parseURI(const std::string& uriStr)
{
    // See https://tools.ietf.org/html/rfc3986 for explanation.
    URI uri;
    std::regex uriRegex(
        "^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?",
        std::regex::extended);
    std::smatch match;
    if (!std::regex_match(uriStr, match, uriRegex)) {
        return uri;
    }

    constexpr auto kHostIndex = 4;
    constexpr auto kPathIndex = 5;
    constexpr auto kQueryIndex = 7;
    constexpr auto kDefaultHTTPPort = 80;

    const auto numMatchingGroups = match.size();

    if (numMatchingGroups < kHostIndex) {
        return uri;
    }
    const auto authority = match[kHostIndex].str();
    const auto colonPos = authority.find(':');
    if (colonPos == std::string::npos) {
        uri._host = authority;
        uri._port = kDefaultHTTPPort;
    }
    else {
        uri._host = authority.substr(0, colonPos);
        const auto portStr = authority.substr(colonPos + 1);
        std::istringstream iss(portStr);
        if (!(iss >> uri._port)) {
            uri._port = kDefaultHTTPPort;
        }
    }

    if (numMatchingGroups < kPathIndex) {
        return uri;
    }
    uri._path = match[kPathIndex].str();

    if (numMatchingGroups < kQueryIndex) {
        return uri;
    }
    uri._query = match[kQueryIndex].str();

    return uri;
}

std::string httpGetRequest(boost::asio::io_service& io, const URI& uri)
{
    using tcp = boost::asio::ip::tcp;
    namespace http = beast::http;

    tcp::resolver resolver(io);
    const tcp::resolver::query query(uri._host, std::to_string(uri._port));
    const auto lookup = resolver.resolve(query);
    tcp::socket socket(io);
    boost::asio::connect(socket, lookup);

    auto target = uri._path + uri._query;
    if (target.empty()) {
        target = "/";
    }
    http::request<http::string_body> req(http::verb::get, target, 11);
    req.set(http::field::host, uri._host);
    req.set(http::field::user_agent, BEAST_VERSION_STRING);
    http::write(socket, req);

    beast::flat_buffer buffer;
    http::response<http::string_body> res;
    http::read(socket, buffer, res);

    boost::system::error_code error;
    socket.shutdown(tcp::socket::shutdown_both, error);
    if (error && error != boost::system::errc::not_connected) {
        throw boost::system::system_error(error);
    }

    if (res.result() != http::status::ok) {
        std::ostringstream oss;
        oss << "Unexpected response status code"
               ", host="
            << uri._host << ", target=" << target
            << ", status=" << res.result();
        throw std::runtime_error(oss.str());
    }

    return res.body;
}

}  // namespace net
}  // namespace utils
}  // namespace jaeger
}  // namespace uber

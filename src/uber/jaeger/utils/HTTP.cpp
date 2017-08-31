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

#include "uber/jaeger/utils/HTTP.h"

#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>

#include <boost/asio.hpp>

namespace uber {
namespace jaeger {
namespace utils {
namespace http {
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

std::string makeRequestStr(const URI& uri)
{
    std::ostringstream oss;
    oss
        << "GET ";

    if (uri._path.empty()) {
        oss << '/';
    }
    else {
        oss << uri._path;
    }

    if (!uri._query.empty()) {
        oss << '?' << uri._query;
    }

    oss << " HTTP/1.1\r\n"
        << "Host: " << uri._host << "\r\n"
        << "Connection: close\r\n\r\n";

    return oss.str();
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

    const auto numMatchingGroups = match.size();

    if (numMatchingGroups < kHostIndex) {
        return uri;
    }
    uri._host = match[kHostIndex].str();

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

std::string httpGetRequest(const URI& uri)
{
    using tcp = boost::asio::ip::tcp;

    boost::asio::io_service io;
    tcp::resolver resolver(io);
    tcp::resolver::query query(uri._host, "http");
    auto endpointItr = resolver.resolve(query);
    tcp::socket socket(io);
    boost::asio::connect(socket, endpointItr);

    const auto requestStr = makeRequestStr(uri);
    boost::asio::streambuf request;
    std::ostream requestStream(&request);
    requestStream << requestStr;
    boost::asio::write(socket, request);

    boost::asio::streambuf response;
    boost::asio::read_until(socket, response, "\r\n");

    std::istream responseStream(&response);
    std::string httpVersion;
    responseStream >> httpVersion;
    unsigned int statusCode = 0;
    responseStream >> statusCode;
    std::string statusMessage;
    std::getline(responseStream, statusMessage);
    if (httpVersion.substr(0, 5) != "HTTP/") {
        std::ostringstream oss;
        oss << "Invalid response: " << httpVersion;
        throw std::runtime_error(oss.str());
    }
    if (!responseStream) {
        throw std::runtime_error("Invalid response");
    }
    if (statusCode != 200) {
        std::ostringstream oss;
        oss << "Received unexpected status code"
               ", expectedStatusCode=200"
               ", actualStatusCode=" << statusCode;
        throw std::runtime_error(oss.str());
    }

    boost::asio::read_until(socket, response, "\r\n\r\n");
    std::string header;
    while (std::getline(responseStream, header) && header != "\r");

    std::ostringstream bodyStream;
    if (response.size() > 0) {
        bodyStream << &response;
    }
    boost::system::error_code error;
    while (boost::asio::read(socket,
                             response,
                             boost::asio::transfer_at_least(1),
                             error)) {
        bodyStream << &response;
    }
    if (error != boost::asio::error::eof) {
        throw boost::system::system_error(error);
    }

    return bodyStream.str();
}

}  // namespace http
}  // namespace utils
}  // namespace jaeger
}  // namespace uber

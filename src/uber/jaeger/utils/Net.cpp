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

#include <regex>
#include <sstream>
#include <stdexcept>

#include "uber/jaeger/Constants.h"

namespace uber {
namespace jaeger {
namespace utils {
namespace net {

URI URI::parse(const std::string& uriStr)
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

    constexpr auto kSchemeIndex = 2;
    constexpr auto kAuthorityIndex = 4;
    constexpr auto kPathIndex = 5;
    constexpr auto kQueryIndex = 7;

    const auto numMatchingGroups = match.size();

    if (numMatchingGroups < kSchemeIndex) {
        return uri;
    }
    uri._scheme = match[kSchemeIndex].str();

    if (numMatchingGroups < kAuthorityIndex) {
        return uri;
    }
    const auto authority = match[kAuthorityIndex].str();
    const auto colonPos = authority.find(':');
    if (colonPos == std::string::npos) {
        uri._host = authority;
    }
    else {
        uri._host = authority.substr(0, colonPos);
        const auto portStr = authority.substr(colonPos + 1);
        std::istringstream iss(portStr);
        iss >> uri._port;
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

std::unique_ptr<::addrinfo, AddrInfoDeleter>
resolveAddress(const URI& uri, int socketType)
{
    ::addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = socketType;

    std::string service;
    if (uri._port != 0) {
        service = std::to_string(uri._port);
    }
    else {
        service = uri._scheme;
    }

    auto* servInfoPtr = static_cast<::addrinfo*>(nullptr);
    const auto returnCode =
        getaddrinfo(uri._host.c_str(), service.c_str(), &hints, &servInfoPtr);
    std::unique_ptr<::addrinfo, AddrInfoDeleter> servInfo(servInfoPtr);
    if (returnCode != 0) {
        std::ostringstream oss;
        oss << "Error resolving address: " << gai_strerror(returnCode);
        throw std::runtime_error(oss.str());
    }

    return servInfo;
}

namespace http {
namespace {

std::istream& readLineCRLF(std::istream& in, std::string& line)
{
    line.clear();
    auto ch = '\0';
    auto sawCR = false;
    while (in.get(ch)) {
        if (sawCR) {
            if (ch == '\n') {
                return in;
            }
            else {
                line.push_back('\r');
                line.push_back(ch);
                sawCR = false;
            }
        }
        else {
            if (ch == '\r') {
                sawCR = true;
            }
            else {
                line.push_back(ch);
            }
        }
    }

    return in;
}

std::invalid_argument parseError(
    const std::string& expected, const std::string& actual)
{
    std::ostringstream oss;
    oss << "Parse error, expected " << expected
        << ", encountered \"" << actual << '"';
    return ParseError(oss.str());
}

void readHeaders(std::istream& in, std::vector<Header>& headers)
{
    const std::regex headerPattern("([^:]+):(.+)$");
    std::string line;
    std::smatch match;
    while (readLineCRLF(in, line)) {
        if (line.empty()) {
            break;
        }
        if (!std::regex_match(line, match, headerPattern) ||
            match.size() < 3) {
            throw parseError("header", line);
        }
        headers.emplace_back(Header(match[1], match[2]));
    }
}

}  // anonymous namespace

Method parseMethod(const std::string& methodName)
{
    static constexpr auto kMethodNames = {
        "OPTIONS",
        "GET",
        "HEAD",
        "POST",
        "PUT",
        "DELETE",
        "TRACE",
        "CONNECT"
    };

    auto itr = std::find(std::begin(kMethodNames),
                         std::end(kMethodNames),
                         methodName);
    if (itr == std::end(kMethodNames)) {
        return Method::EXTENSION;
    }
    return static_cast<Method>(std::distance(std::begin(kMethodNames), itr));
}

Request Request::parse(std::istream& in)
{
    const std::regex requestLinePattern(
            "([A-Z]+) ([^ ]+) HTTP/([0-9]\\.[0-9])$");
    std::string line;
    std::smatch match;
    if (!readLineCRLF(in, line) ||
        !std::regex_match(line, match, requestLinePattern) ||
        match.size() < 4) {
        throw parseError("request line", line);
    }
    Request request;

    request._method = parseMethod(match[1]);
    request._target = match[2];
    request._version = match[3];

    return request;
}

Response Response::parse(std::istream& in)
{
    const std::regex statusLinePattern("HTTP/([0-9]\\.[0-9]) ([0-9]+) (.+)$");
    std::string line;
    std::smatch match;
    if (!readLineCRLF(in, line) ||
        !std::regex_match(line, match, statusLinePattern) ||
        match.size() < 4) {
        throw parseError("status line", line);
    }
    Response response;
    response._version = match[1];
    std::istringstream iss(match[2]);
    iss >> response._statusCode;
    response._reason = match[3];

    readHeaders(in, response._headers);

    response._body = std::string(std::istreambuf_iterator<char>(in),
                                 std::istreambuf_iterator<char>{});

    return response;
}

Response get(const URI& uri)
{
    Socket socket;
    socket.open(AF_INET, SOCK_STREAM);
    socket.connect(uri);
    std::ostringstream requestStream;
    requestStream
        << "GET " << uri.target() << " HTTP/1.1\r\n"
           "Host: " << uri.authority() << "\r\n"
           "User-Agent: jaeger/" << kJaegerClientVersion << "\r\n\r\n";
    const auto request = requestStream.str();
    const auto numWritten =
        ::write(socket.handle(), request.c_str(), request.size());
    if (numWritten != request.size()) {
        std::ostringstream oss;
        oss << "Failed to write entire HTTP request"
            << ", uri=" << uri
            << ", request=" << request;
        throw std::system_error(errno,
                                std::generic_category(),
                                oss.str());
    }

    constexpr auto kBufferSize = 256;
    std::array<char, kBufferSize> buffer;
    auto numRead = ::read(socket.handle(), &buffer[0], buffer.size());
    std::string response;
    while (numRead > 0) {
        response.append(&buffer[0], numRead);
        if (numRead < buffer.size()) {
            break;
        }
        numRead = ::read(socket.handle(), &buffer[0], buffer.size());
    }
    std::istringstream responseStream(response);
    return Response::parse(responseStream);
}

}  // namespace http
}  // namespace net
}  // namespace utils
}  // namespace jaeger
}  // namespace uber

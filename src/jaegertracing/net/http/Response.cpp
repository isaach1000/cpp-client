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

#include "jaegertracing/net/http/Response.h"

#include <unistd.h>

#include <regex>
#include <sstream>
#include <stdexcept>

#include "jaegertracing/Constants.h"
#include "jaegertracing/net/Socket.h"

namespace jaegertracing {
namespace net {
namespace http {

Response Response::parse(std::istream& in)
{
    const std::regex statusLinePattern("HTTP/([0-9]\\.[0-9]) ([0-9]+) (.+)$");
    std::string line;
    std::smatch match;
    if (!readLineCRLF(in, line) ||
        !std::regex_match(line, match, statusLinePattern) || match.size() < 4) {
        throw ParseError::make("status line", line);
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
    requestStream << "GET " << uri.target()
                  << " HTTP/1.1\r\n"
                     "Host: "
                  << uri.authority()
                  << "\r\n"
                     "User-Agent: jaegertracing/"
                  << kJaegerClientVersion << "\r\n\r\n";
    const auto request = requestStream.str();
    const auto numWritten =
        ::write(socket.handle(), request.c_str(), request.size());
    if (numWritten != static_cast<int>(request.size())) {
        std::ostringstream oss;
        oss << "Failed to write entire HTTP request"
            << ", uri=" << uri << ", request=" << request;
        throw std::system_error(errno, std::generic_category(), oss.str());
    }

    constexpr auto kBufferSize = 256;
    std::array<char, kBufferSize> buffer;
    auto numRead = ::read(socket.handle(), &buffer[0], buffer.size());
    std::string response;
    while (numRead > 0) {
        response.append(&buffer[0], numRead);
        if (numRead < static_cast<int>(buffer.size())) {
            break;
        }
        numRead = ::read(socket.handle(), &buffer[0], buffer.size());
    }
    std::istringstream responseStream(response);
    return Response::parse(responseStream);
}

}  // namespace http
}  // namespace net
}  // namespace jaegertracing

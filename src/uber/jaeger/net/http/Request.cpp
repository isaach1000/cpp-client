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

#include "uber/jaeger/net/http/Request.h"

namespace uber {
namespace jaeger {
namespace net {
namespace http {

Request Request::parse(std::istream& in)
{
    const std::regex requestLinePattern(
        "([A-Z]+) ([^ ]+) HTTP/([0-9]\\.[0-9])$");
    std::string line;
    std::smatch match;
    if (!readLineCRLF(in, line) ||
        !std::regex_match(line, match, requestLinePattern) ||
        match.size() < 4) {
        throw ParseError::make("request line", line);
    }
    Request request;

    request._method = parseMethod(match[1]);
    request._target = match[2];
    request._version = match[3];

    return request;
}

}  // namespace http
}  // namespace net
}  // namespace jaeger
}  // namespace uber

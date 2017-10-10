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

#ifndef UBER_JAEGER_NET_HTTP_HEADER_H
#define UBER_JAEGER_NET_HTTP_HEADER_H

#include <cassert>
#include <regex>
#include <string>

#include "uber/jaeger/net/http/Error.h"

namespace uber {
namespace jaeger {
namespace net {
namespace http {

class Header {
  public:
    Header() = default;

    Header(const std::string& key,
           const std::string& value)
        : _key(key)
        , _value(value)
    {
    }

  private:
    std::string _key;
    std::string _value;
};

inline std::istream& readLineCRLF(std::istream& in, std::string& line)
{
    line.clear();
    auto ch = '\0';
    auto sawCR = false;
    while (in.get(ch)) {
        if (sawCR) {
            if (ch == '\n') {
                break;
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

inline void readHeaders(std::istream& in, std::vector<Header>& headers)
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
            throw ParseError::make("header", line);
        }
        headers.emplace_back(Header(match[1], match[2]));
    }
}

}  // namespace http
}  // namespace net
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_NET_HTTP_HEADER_H

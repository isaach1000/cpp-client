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

#include "uber/jaeger/net/URI.h"

#include <cassert>
#include <iomanip>
#include <iostream>
#include <regex>

namespace uber {
namespace jaeger {
namespace net {
namespace {

bool isHex(char ch)
{
    return (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') ||
           (ch >= 'a' && ch <= 'f');
}

int decodeHex(char ch)
{
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    }
    assert(ch >= 'a' && ch <= 'f');
    return ch - 'a' + 10;
}

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

URI URI::parse(const std::string& uriStr)
{
    // See https://tools.ietf.org/html/rfc3986 for explanation.
    URI uri;
    std::regex uriRegex(
        "^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?",
        std::regex::extended);
    std::smatch match;
    std::regex_match(uriStr, match, uriRegex);

    constexpr auto kSchemeIndex = 2;
    constexpr auto kAuthorityIndex = 4;
    constexpr auto kPathIndex = 5;
    constexpr auto kQueryIndex = 7;

    assert(match.size() >= kQueryIndex);

    uri._scheme = match[kSchemeIndex].str();

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

    uri._path = match[kPathIndex].str();
    uri._query = match[kQueryIndex].str();

    return uri;
}

std::string URI::queryEscape(const std::string& input)
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

std::string URI::queryUnescape(const std::string& input)
{
    enum class State { kDefault, kPercent, kFirstHex };

    std::ostringstream oss;
    auto hex = 0;
    auto state = State::kDefault;
    for (auto&& ch : input) {
        switch (state) {
        case State::kDefault: {
            if (ch == '%') {
                state = State::kPercent;
            }
            else {
                oss.put(ch);
            }
        } break;
        case State::kPercent: {
            if (isHex(ch)) {
                state = State::kFirstHex;
                hex = (decodeHex(ch) & 0xff);
            }
            else {
                state = State::kDefault;
                oss.put('%');
                oss.put(ch);
            }
        } break;
        default: {
            assert(state == State::kFirstHex);
            if (isHex(ch)) {
                hex <<= 4;
                hex |= (decodeHex(ch) & 0xff);
                oss.put(static_cast<char>(hex));
            }
            else {
                oss.put('%');
                oss << std::hex << hex;
                oss.put(ch);
            }
            state = State::kDefault;
            hex = 0;
        } break;
        }
    }
    return oss.str();
}

std::unique_ptr<::addrinfo, AddrInfoDeleter> resolveAddress(const URI& uri,
                                                            int socketType)
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

}  // namespace net
}  // namespace jaeger
}  // namespace uber

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

#include <regex>

namespace uber {
namespace jaeger {
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

}  // namespace net
}  // namespace jaeger
}  // namespace uber

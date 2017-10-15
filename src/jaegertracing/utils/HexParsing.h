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

#ifndef JAEGERTRACING_UTILS_HEXPARSING_H
#define JAEGERTRACING_UTILS_HEXPARSING_H

#include <cassert>
#include <iomanip>
#include <iostream>

namespace jaegertracing {
namespace utils {
namespace HexParsing {

inline bool isHex(char ch)
{
    return (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') ||
           (ch >= 'a' && ch <= 'f');
}

inline std::string readSegment(std::istream& in, size_t maxChars, char delim)
{
    std::string buffer;
    auto ch = '\0';
    for (auto i = static_cast<size_t>(0); i < maxChars && in.get(ch); ++i) {
        if (!isHex(ch)) {
            if (ch == delim) {
                in.putback(ch);
                break;
            }
            else {
                return "";
            }
        }

        buffer.push_back(ch);
    }
    return buffer;
}

template <typename ResultType>
ResultType decodeHex(const std::string& str)
{
    auto first = std::begin(str);
    auto last = std::end(str);
    ResultType result = 0;
    for (; first != last; ++first) {
        const auto ch = *first;

        // This condition is guaranteed by `readSegment`.
        assert(isHex(ch));

        auto hexDigit = 0;
        if (std::isdigit(ch)) {
            hexDigit = (ch - '0');
        }
        else if (std::isupper(ch)) {
            hexDigit = (ch - 'A') + 10;
        }
        else {
            hexDigit = (ch - 'a') + 10;
        }

        result = (result << 4) | hexDigit;
    }

    return result;
}

}  // namespace HexParsing
}  // namespace utils
}  // namespace jaegertracing

#endif  // JAEGERTRACING_UTILS_HEXPARSING_H

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

#include "jaegertracing/TraceID.h"

#include "jaegertracing/utils/HexParsing.h"

namespace jaegertracing {

TraceID TraceID::fromStream(std::istream& in)
{
    TraceID traceID;
    constexpr auto kMaxChars = static_cast<size_t>(32);
    auto buffer = utils::HexParsing::readSegment(in, kMaxChars, ':');

    if (buffer.empty()) {
        return TraceID();
    }

    if (buffer.size() < kMaxChars / 2) {
        traceID._low = utils::HexParsing::decodeHex<uint64_t>(buffer);
    }
    else {
        auto beginLowStr = std::end(buffer) - kMaxChars / 2;
        const std::string highStr(std::begin(buffer), beginLowStr);
        traceID._high = utils::HexParsing::decodeHex<uint64_t>(highStr);
        const std::string lowStr(beginLowStr, std::end(buffer));
        traceID._low = utils::HexParsing::decodeHex<uint64_t>(lowStr);
    }

    return traceID;
}

}  // namespace jaegertracing

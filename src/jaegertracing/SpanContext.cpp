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

#include "jaegertracing/SpanContext.h"

#include "jaegertracing/utils/HexParsing.h"

namespace jaegertracing {

SpanContext SpanContext::fromStream(std::istream& in)
{
    SpanContext spanContext;
    spanContext._traceID = TraceID::fromStream(in);
    if (!spanContext._traceID.isValid()) {
        return SpanContext();
    }

    char ch = '\0';
    if (!in.get(ch) || ch != ':') {
        return SpanContext();
    }

    constexpr auto kMaxUInt64Chars = static_cast<size_t>(16);
    auto buffer = utils::HexParsing::readSegment(in, kMaxUInt64Chars, ':');
    if (buffer.empty()) {
        return SpanContext();
    }
    spanContext._spanID = utils::HexParsing::decodeHex<uint64_t>(buffer);

    if (!in.get(ch) || ch != ':') {
        return SpanContext();
    }

    buffer = utils::HexParsing::readSegment(in, kMaxUInt64Chars, ':');
    if (buffer.empty()) {
        return SpanContext();
    }
    spanContext._parentID = utils::HexParsing::decodeHex<uint64_t>(buffer);

    if (!in.get(ch) || ch != ':') {
        return SpanContext();
    }

    constexpr auto kMaxByteChars = static_cast<size_t>(2);
    buffer = utils::HexParsing::readSegment(in, kMaxByteChars, ':');
    if (buffer.empty()) {
        return SpanContext();
    }
    spanContext._flags = utils::HexParsing::decodeHex<unsigned char>(buffer);

    return spanContext;
}

}  // namespace jaegertracing

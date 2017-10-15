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

#ifndef JAEGERTRACING_REFERENCE_H
#define JAEGERTRACING_REFERENCE_H

#include <sstream>
#include <string>

#include <opentracing/propagation.h>

#include "jaegertracing/SpanContext.h"
#include "jaegertracing/thrift-gen/jaeger_types.h"

namespace jaegertracing {

class Reference {
  public:
    using Type = opentracing::SpanReferenceType;

    Reference(const SpanContext& spanContext, Type type)
        : _spanContext(spanContext)
        , _type(type)
    {
    }

    const SpanContext& spanContext() const { return _spanContext; }

    Type type() const { return _type; }

    thrift::SpanRef thrift() const
    {
        thrift::SpanRef spanRef;
        switch (_type) {
        case Type::ChildOfRef: {
            spanRef.__set_refType(thrift::SpanRefType::CHILD_OF);
        } break;
        case Type::FollowsFromRef: {
            spanRef.__set_refType(thrift::SpanRefType::FOLLOWS_FROM);
        } break;
        default: {
            std::ostringstream oss;
            oss << "Invalid span reference type " << static_cast<int>(_type)
                << ", context " << _spanContext;
            throw std::invalid_argument(oss.str());
        } break;
        }
        spanRef.__set_traceIdHigh(_spanContext.traceID().high());
        spanRef.__set_traceIdLow(_spanContext.traceID().low());
        spanRef.__set_spanId(_spanContext.spanID());
        return spanRef;
    }

  private:
    SpanContext _spanContext;
    Type _type;
};

}  // namespace jaegertracing

#endif  // JAEGERTRACING_REFERENCE_H

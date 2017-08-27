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

#ifndef UBER_JAEGER_REFERENCE_H
#define UBER_JAEGER_REFERENCE_H

#include <string>

#include "uber/jaeger/SpanContext.h"

namespace uber {
namespace jaeger {

class Reference {
  public:
    enum class Type {
        kChildOfRef,
        kFollowsFromRef
    };

    Reference(const SpanContext& spanContext, Type type)
        : _spanContext(spanContext)
        , _type(type)
    {
    }

    const SpanContext& spanContext() const { return _spanContext; }

    Type type() const { return _type; }

  private:
    SpanContext _spanContext;
    Type _type;
};

}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_REFERENCE_H

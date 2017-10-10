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

#ifndef UBER_JAEGER_PROPAGATION_TEXTMAPPROPAGATOR_H
#define UBER_JAEGER_PROPAGATION_TEXTMAPPROPAGATOR_H

#include <opentracing/propagation.h>

namespace uber {
namespace jaeger {
namespace propagation {

class TextMapPropagator : public opentracing::TextMapReader,
                          public opentracing::TextMapWriter {
  public:
    using expected_void = opentracing::expected<void>;
    using string_view = opentracing::string_view;

    template <typename... Args>
    expected_void forEachKey(Args&&... args)
    {
        return ForeachKey(std::forward<Args>(args)...);
    }

    expected_void ForeachKey(
        std::function<expected_void(string_view key, string_view value)> f)
        const override;

    template <typename... Args>
    expected_void set(Args&&... args)
    {
        return Set(std::forward<Args>(args)...);
    }

    expected_void Set(string_view key, string_view value) const override;
};

}  // namespace propagation
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_PROPAGATION_TEXTMAPPROPAGATOR_H

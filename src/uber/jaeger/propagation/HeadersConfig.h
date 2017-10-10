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

#ifndef UBER_JAEGER_PROPAGATION_HEADERSCONFIG_H
#define UBER_JAEGER_PROPAGATION_HEADERSCONFIG_H

#include <string>

#include "uber/jaeger/Constants.h"

namespace uber {
namespace jaeger {
namespace propagation {

class HeadersConfig {
  public:
    HeadersConfig()
        : _jaegerDebugHeader(kJaegerDebugHeader)
        , _jaegerBaggageHeader(kJaegerBaggageHeader)
        , _traceContextHeaderName(kTraceContextHeaderName)
        , _traceBaggageHeaderPrefix(kTraceBaggageHeaderPrefix)
    {
    }

    const std::string& jaegerBaggageHeader() const
    {
        return _jaegerBaggageHeader;
    }

    void setJaegerBaggageHeader(const std::string& jaegerBaggageHeader)
    {
        _jaegerBaggageHeader = jaegerBaggageHeader;
    }

    const std::string& jaegerDebugHeader() const { return _jaegerDebugHeader; }

    void setJaegerDebugHeader(const std::string& jaegerDebugHeader)
    {
        _jaegerDebugHeader = jaegerDebugHeader;
    }

    const std::string& traceBaggageHeaderPrefix() const
    {
        return _traceBaggageHeaderPrefix;
    }

    void
    setTraceBaggageHeaderPrefix(const std::string& traceBaggageHeaderPrefix)
    {
        _traceBaggageHeaderPrefix = traceBaggageHeaderPrefix;
    }

    const std::string& traceContextHeaderName() const
    {
        return _traceContextHeaderName;
    }

    void setTraceContextHeaderName(const std::string& traceContextHeaderName)
    {
        _traceContextHeaderName = traceContextHeaderName;
    }

  private:
    std::string _jaegerDebugHeader;
    std::string _jaegerBaggageHeader;
    std::string _traceContextHeaderName;
    std::string _traceBaggageHeaderPrefix;
};

}  // namespace propagation
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_PROPAGATION_HEADERSCONFIG_H

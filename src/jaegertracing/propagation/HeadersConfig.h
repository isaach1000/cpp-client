/*
 * Copyright (c) 2017 Uber Technologies, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef JAEGERTRACING_PROPAGATION_HEADERSCONFIG_H
#define JAEGERTRACING_PROPAGATION_HEADERSCONFIG_H

#include <string>

#include "jaegertracing/Constants.h"

#ifdef JAEGERTRACING_WITH_YAML_CPP
#include <yaml-cpp/yaml.h>
#endif  // JAEGERTRACING_WITH_YAML_CPP

namespace jaegertracing {
namespace propagation {

class HeadersConfig {
  public:
#ifdef JAEGERTRACING_WITH_YAML_CPP

    static HeadersConfig parse(const YAML::Node& configYAML)
    {
        // TODO
        return HeadersConfig();
    }

#endif  // JAEGERTRACING_WITH_YAML_CPP

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
}  // namespace jaegertracing

#endif  // JAEGERTRACING_PROPAGATION_HEADERSCONFIG_H

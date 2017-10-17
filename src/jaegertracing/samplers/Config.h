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

#ifndef JAEGERTRACING_SAMPLERS_CONFIG_H
#define JAEGERTRACING_SAMPLERS_CONFIG_H

#include <chrono>
#include <string>

#include "jaegertracing/Constants.h"

#ifdef JAEGERTRACING_WITH_YAML_CPP
#include <yaml-cpp/yaml.h>
#endif  // JAEGERTRACING_WITH_YAML_CPP

namespace jaegertracing {
namespace samplers {

class Config {
  public:
    using Clock = std::chrono::steady_clock;

#ifdef JAEGERTRACING_WITH_YAML_CPP

    static Config parse(const YAML::Node& configYAML)
    {
        // TODO
        return Config();
    }

#endif  // JAEGERTRACING_WITH_YAML_CPP

    Config() = default;

    Config(const std::string& type,
           double param,
           const std::string& samplingServerURL,
           int maxOperations,
           const Clock::duration& samplingRefreshInterval)
        : _type(type)
        , _param(param)
        , _samplingServerURL(samplingServerURL)
        , _maxOperations(maxOperations)
        , _samplingRefreshInterval(samplingRefreshInterval)
    {
    }

    const std::string& type() const { return _type; }

    void setType(const std::string& type) { _type = type; }

    double param() const { return _param; }

    void setParam(double param) { _param = param; }

    const std::string& samplingServerURL() const { return _samplingServerURL; }

    void setSamplingServerURL(const std::string& samplingServerURL)
    {
        _samplingServerURL = samplingServerURL;
    }

    int maxOperations() const { return _maxOperations; }

    void setMaxOperations(int maxOperations) { _maxOperations = maxOperations; }

    const Clock::duration& samplingRefreshInterval() const
    {
        return _samplingRefreshInterval;
    }

    void
    setSamplingRefreshInterval(const Clock::duration& samplingRefreshInterval)
    {
        _samplingRefreshInterval = samplingRefreshInterval;
    }

  private:
    std::string _type;
    double _param;
    std::string _samplingServerURL;
    int _maxOperations;
    Clock::duration _samplingRefreshInterval;
};

}  // namespace samplers
}  // namespace jaegertracing

#endif  // JAEGERTRACING_SAMPLERS_CONFIG_H

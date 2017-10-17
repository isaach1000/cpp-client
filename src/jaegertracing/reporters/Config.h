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

#ifndef JAEGERTRACING_REPORTERS_CONFIG_H
#define JAEGERTRACING_REPORTERS_CONFIG_H

#include <chrono>
#include <string>

#include "jaegertracing/utils/YAML.h"

namespace jaegertracing {
namespace reporters {

class Config {
  public:
    using Clock = std::chrono::steady_clock;

#ifdef JAEGERTRACING_WITH_YAML_CPP

    static Config parse(const YAML::Node& configYAML)
    {
        if (!configYAML.IsMap()) {
            return Config();
        }

        const auto queueSize =
            utils::yaml::findOrDefault<int>(configYAML, "queueSize", 0);
        const auto bufferFlushInterval =
            std::chrono::seconds(
                utils::yaml::findOrDefault<int>(
                    configYAML, "bufferFlushInterval", 0));
        const auto logSpans =
            utils::yaml::findOrDefault<bool>(configYAML, "logSpans", false);
        const auto localAgentHostPort =
            utils::yaml::findOrDefault<std::string>(
                configYAML, "localAgentHostPort", "");
        return Config(queueSize,
                      bufferFlushInterval,
                      logSpans,
                      localAgentHostPort);
    }

#endif  // JAEGERTRACING_WITH_YAML_CPP

    Config() = default;

    Config(int queueSize,
           const Clock::duration& bufferFlushInterval,
           bool logSpans,
           const std::string& localAgentHostPort)
        : _queueSize(queueSize)
        , _bufferFlushInterval(bufferFlushInterval)
        , _logSpans(logSpans)
        , _localAgentHostPort(localAgentHostPort)
    {
    }

    int queueSize() const { return _queueSize; }

    void setQueueSize(int queueSize) { _queueSize = queueSize; }

    const Clock::duration& bufferFlushInterval() const
    {
        return _bufferFlushInterval;
    }

    void setBufferFlushInterval(const Clock::duration& bufferFlushInterval)
    {
        _bufferFlushInterval = bufferFlushInterval;
    }

    bool logSpans() const { return _logSpans; }

    void setLogSpans(bool logSpans) { _logSpans = logSpans; }

    const std::string& localAgentHostPort() const
    {
        return _localAgentHostPort;
    }

    void setLocalAgentHostPort(const std::string& localAgentHostPort)
    {
        _localAgentHostPort = localAgentHostPort;
    }

  private:
    int _queueSize;
    Clock::duration _bufferFlushInterval;
    bool _logSpans;
    std::string _localAgentHostPort;
};

}  // namespace reporters
}  // namespace jaegertracing

#endif  // JAEGERTRACING_REPORTERS_CONFIG_H

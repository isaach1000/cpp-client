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

#include "uber/jaeger/samplers/RemotelyControlledSampler.h"

#include <cassert>

#include <nlohmann/json.hpp>

#include "uber/jaeger/metrics/Counter.h"
#include "uber/jaeger/metrics/Gauge.h"
#include "uber/jaeger/samplers/AdaptiveSampler.h"

namespace uber {
namespace jaeger {

namespace thrift {
namespace sampling_manager {

#define DECODE_FIELD(field)                                                    \
    {                                                                          \
        result.__set_##field(                                                  \
            jsonValue.at(#field).get<decltype(result.field)>());               \
    }

void from_json(const nlohmann::json& jsonValue,
               ProbabilisticSamplingStrategy& result)
{
    DECODE_FIELD(samplingRate);
}

void from_json(const nlohmann::json& jsonValue,
               RateLimitingSamplingStrategy& result)
{
    DECODE_FIELD(maxTracesPerSecond);
}

void from_json(const nlohmann::json& jsonValue,
               OperationSamplingStrategy& result)
{
    DECODE_FIELD(operation);
    DECODE_FIELD(probabilisticSampling);
}

void from_json(const nlohmann::json& jsonValue,
               PerOperationSamplingStrategies& result)
{
    DECODE_FIELD(defaultSamplingProbability);
    DECODE_FIELD(defaultLowerBoundTracesPerSecond);
    DECODE_FIELD(perOperationStrategies);
    DECODE_FIELD(defaultUpperBoundTracesPerSecond);
}

void from_json(const nlohmann::json& jsonValue,
               SamplingStrategyResponse& result)
{
    auto operationSamplingItr = jsonValue.find("operationSampling");
    if (operationSamplingItr != std::end(jsonValue)) {
        result.operationSampling
            = operationSamplingItr->get<PerOperationSamplingStrategies>();
    }

    result.strategyType = static_cast<SamplingStrategyType::type>(
        jsonValue.at("strategyType").get<int>());
    switch (result.strategyType) {
    case SamplingStrategyType::PROBABILISTIC: {
        DECODE_FIELD(probabilisticSampling);
    } break;
    case SamplingStrategyType::RATE_LIMITING: {
        DECODE_FIELD(rateLimitingSampling);
    } break;
    default: {
        std::ostringstream oss;
        oss << "Invalid strategy type" << result.strategyType;
        throw std::runtime_error(oss.str());
    } break;
    }
}

#undef DECODE_FIELD

}  // namespace sampling_manager
}  // namespace thrift

namespace samplers {
namespace {

bool isUnreserved(char ch)
{
    if (std::isalpha(ch) || std::isdigit(ch)) {
        return true;
    }

    switch (ch) {
    case '-':
    case '.':
    case '_':
    case '~':
        return true;
    default:
        return false;
    }
}

std::string percentEncode(const std::string& input)
{
    std::ostringstream oss;
    for (auto&& ch : input) {
        if (isUnreserved(ch)) {
            oss << ch;
        }
        else {
            oss << '%' << std::uppercase << std::hex << static_cast<int>(ch);
        }
    }
    return oss.str();
}

class HTTPSamplingManager : public thrift::sampling_manager::SamplingManagerIf {
  public:
    using SamplingStrategyResponse
        = thrift::sampling_manager::SamplingStrategyResponse;

    explicit HTTPSamplingManager(const std::string& serverURL)
        : _serverURL(serverURL)
    {
    }

    void getSamplingStrategy(SamplingStrategyResponse& result,
                             const std::string& serviceName) override
    {
        const auto uriStr =
            _serverURL + "?service=" + percentEncode(serviceName);
        /* TODO:
        const auto uri = parseURI(uriStr);
        const auto response = httpGetRequest(uri);
        const auto jsonValue = nlohmann::json::parse(response);
        result = jsonValue.get<SamplingStrategyResponse>();*/
    }

  private:
    std::string _serverURL;
};

}  // anonymous namespace

RemotelyControlledSampler::RemotelyControlledSampler(
    const std::string& serviceName, const SamplerOptions& options)
    : _options(options)
    , _serviceName(serviceName)
    , _manager(
          std::make_shared<HTTPSamplingManager>(_options.samplingServerURL()))
    , _running(true)
    , _mutex()
    , _shutdownCV()
    , _thread([this]() { pollController(); })
{
}

SamplingStatus
RemotelyControlledSampler::isSampled(const TraceID& id,
                                     const std::string& operation)
{
    std::lock_guard<std::mutex> lock(_mutex);
    assert(_options.sampler());
    return _options.sampler()->isSampled(id, operation);
}

void RemotelyControlledSampler::close()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _running = false;
    lock.unlock();
    _shutdownCV.notify_one();
    _thread.join();
}

void RemotelyControlledSampler::pollController()
{
    while (_running) {
        updateSampler();
        std::unique_lock<std::mutex> lock(_mutex);
        _shutdownCV.wait_for(lock,
                             _options.samplingRefreshInterval(),
                             [this]() { return !_running; });
    }
}

void RemotelyControlledSampler::updateSampler()
{
    assert(_manager);
    assert(_options.metrics());
    thrift::sampling_manager::SamplingStrategyResponse response;
    try {
        assert(_manager);
        _manager->getSamplingStrategy(response, _serviceName);
    } catch (const std::exception& ex) {
        _options.metrics()->samplerQueryFailure().inc(1);
        return;
    } catch (...) {
        _options.metrics()->samplerQueryFailure().inc(1);
        return;
    }

    std::lock_guard<std::mutex> lock(_mutex);
    _options.metrics()->samplerRetrieved().inc(1);

    if (response.__isset.operationSampling) {
        updateAdaptiveSampler(response.operationSampling);
    }
    else {
        try {
            updateRateLimitingOrProbabilisticSampler(response);
        } catch (const std::exception& ex) {
            _options.metrics()->samplerUpdateFailure().inc(1);
            return;
        } catch (...) {
            _options.metrics()->samplerUpdateFailure().inc(1);
            return;
        }
    }
    _options.metrics()->samplerUpdated().inc(1);
}

void RemotelyControlledSampler::updateAdaptiveSampler(
    const PerOperationSamplingStrategies& strategies)
{
    auto sampler = _options.sampler();
    assert(sampler);
    if (sampler->type() == Type::kAdaptiveSampler) {
        static_cast<AdaptiveSampler&>(*sampler).update(strategies);
    }
    else {
        sampler = std::make_shared<AdaptiveSampler>(strategies,
                                                    _options.maxOperations());
        _options.setSampler(sampler);
    }
}

void RemotelyControlledSampler::updateRateLimitingOrProbabilisticSampler(
    const SamplingStrategyResponse& response)
{
    std::shared_ptr<Sampler> sampler;
    if (response.__isset.probabilisticSampling) {
        sampler = std::make_shared<ProbabilisticSampler>(
            response.probabilisticSampling.samplingRate);
    }
    else if (response.__isset.rateLimitingSampling) {
        sampler = std::make_shared<RateLimitingSampler>(
            response.rateLimitingSampling.maxTracesPerSecond);
    }
    else {
        std::ostringstream oss;
        oss << "Unsupported sampling strategy type " << response.strategyType;
        throw std::runtime_error(oss.str());
    }
    _options.setSampler(sampler);
}

}  // namespace samplers
}  // namespace jaeger
}  // namespace uber

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

#include <gtest/gtest.h>

#include <thrift/protocol/TJSONProtocol.h>

#include "uber/jaeger/testutils/MockAgent.h"
#include "uber/jaeger/utils/Net.h"

namespace uber {
namespace jaeger {
namespace testutils {

TEST(MockAgent, testSpanServer)
{
    std::shared_ptr<MockAgent> mockAgent = MockAgent::make();
    mockAgent->start();

    auto client = mockAgent->spanServerClient();

    constexpr auto kBiggestBatch = 5;
    for (auto i = 1; i < kBiggestBatch; ++i) {
        thrift::Batch batch;
        batch.spans.resize(i);
        for (auto j = 0; j < i; ++j) {
            std::string operationName("span-");
            operationName += std::to_string(j);
            batch.spans[j].__set_operationName(operationName);
        }

        client->emitBatch(batch);

        constexpr auto kNumTries = 100;
        for (auto k = 0; k < kNumTries; ++k) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            const auto batches = mockAgent->batches();
            if (!batches.empty() &&
                static_cast<int>(batches[0].spans.size()) == i) {
                break;
            }
        }

        const auto batches = mockAgent->batches();
        ASSERT_FALSE(batches.empty());
        ASSERT_EQ(i, static_cast<int>(batches[0].spans.size()));
        for (auto j = 0; j < i; ++j) {
            std::string operationName("span-");
            operationName += std::to_string(j);
            ASSERT_EQ(operationName, batches[0].spans[j].operationName);
        }
        mockAgent->resetBatches();
    }
}

TEST(MockAgent, testSamplingManager)
{
    auto mockAgent = MockAgent::make();
    mockAgent->start();

    {
        std::ostringstream oss;
        oss << "http://" << mockAgent->samplingServerAddr().authority() << '/';
        const auto uriStr = oss.str();
        const auto uri = utils::net::URI::parse(uriStr);
        const auto response = utils::net::http::get(uri);
        ASSERT_EQ("no 'service' parameter", response.body());
    }
    {
        std::ostringstream oss;
        oss << "http://" << mockAgent->samplingServerAddr().authority()
            << "/?service=a&service=b";
        const auto uriStr = oss.str();
        const auto uri = utils::net::URI::parse(uriStr);
        const auto response = utils::net::http::get(uri);
        ASSERT_EQ("'service' parameter must occur only once", response.body());
    }
    {
        std::ostringstream oss;
        oss << "http://" << mockAgent->samplingServerAddr().authority()
            << "/?service=something";
        const auto uriStr = oss.str();
        const auto uri = utils::net::URI::parse(uriStr);
        const auto responseHTTP = utils::net::http::get(uri);
        thrift::sampling_manager::SamplingStrategyResponse response;
        boost::shared_ptr<apache::thrift::transport::TMemoryBuffer> transport(
            new apache::thrift::transport::TMemoryBuffer());
        apache::thrift::protocol::TJSONProtocol protocol(transport);
        std::vector<uint8_t> buffer;
        buffer.reserve(responseHTTP.body().size());
        buffer.insert(std::end(buffer),
                      std::begin(responseHTTP.body()),
                      std::end(responseHTTP.body()));
        transport->write(&buffer[0], buffer.size());
        response.read(&protocol);
        ASSERT_EQ(thrift::sampling_manager::SamplingStrategyType::PROBABILISTIC,
                  response.strategyType);
    }
    {
        thrift::sampling_manager::SamplingStrategyResponse config;
        config.__set_strategyType(
            thrift::sampling_manager::SamplingStrategyType::RATE_LIMITING);
        thrift::sampling_manager::RateLimitingSamplingStrategy rateLimiting;
        rateLimiting.__set_maxTracesPerSecond(123);
        config.__set_rateLimitingSampling(rateLimiting);
        mockAgent->addSamplingStrategy("service123", config);

        std::ostringstream oss;
        oss << "http://" << mockAgent->samplingServerAddr().authority()
            << "/?service=service123";
        const auto uriStr = oss.str();
        const auto uri = utils::net::URI::parse(uriStr);
        const auto responseHTTP = utils::net::http::get(uri);
        thrift::sampling_manager::SamplingStrategyResponse response;
        boost::shared_ptr<apache::thrift::transport::TMemoryBuffer> transport(
            new apache::thrift::transport::TMemoryBuffer());
        apache::thrift::protocol::TJSONProtocol protocol(transport);
        std::vector<uint8_t> buffer;
        buffer.reserve(responseHTTP.body().size());
        buffer.insert(std::end(buffer),
                      std::begin(responseHTTP.body()),
                      std::end(responseHTTP.body()));
        transport->write(&buffer[0], buffer.size());
        response.read(&protocol);
        ASSERT_EQ(config, response);
    }
}

}  // namespace testutils
}  // namespace jaeger
}  // namespace uber

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

#include <random>

#include <gtest/gtest.h>

#include "uber/jaeger/Constants.h"
#include "uber/jaeger/Tag.h"
#include "uber/jaeger/samplers/AdaptiveSampler.h"
#include "uber/jaeger/samplers/ConstSampler.h"
#include "uber/jaeger/samplers/GuaranteedThroughputProbabilisticSampler.h"
#include "uber/jaeger/samplers/ProbabilisticSampler.h"
#include "uber/jaeger/samplers/RateLimitingSampler.h"
#include "uber/jaeger/samplers/RemotelyControlledSampler.h"
#include "uber/jaeger/samplers/Sampler.h"
#include "uber/jaeger/samplers/SamplerOptions.h"
#include "uber/jaeger/samplers/SamplingStatus.h"
#include "uber/jaeger/testutils/TUDPTransport.h"

namespace uber {
namespace jaeger {
namespace samplers {
namespace {

constexpr auto kTestOperationName = "op";
constexpr auto kTestFirstTimeOperationName = "firstTimeOp";
constexpr auto kTestDefaultSamplingProbability = 0.5;
constexpr auto kTestMaxID = std::numeric_limits<uint64_t>::max() / 2 + 1;
constexpr auto kTestDefaultMaxOperations = 10;

const Tag testProbablisticExpectedTags[]
    = { { "sampler.type", "probabilistic" }, { "sampler.param", 0.5 } };

const Tag testLowerBoundExpectedTags[]
    = { { "sampler.type", "lowerbound" }, { "sampler.param", 0.5 } };

#define CMP_TAGS(tagArr, tagVec)                                               \
    {                                                                          \
        ASSERT_EQ(sizeof(tagArr) / sizeof(Tag), (tagVec).size());              \
        for (auto i = static_cast<size_t>(0); i < (tagVec).size(); ++i) {      \
            ASSERT_EQ((tagArr)[i].thrift(), (tagVec)[i].thrift());             \
        }                                                                      \
    }

}  // anonymous namespace

TEST(Sampler, testSamplerTags)
{
    ConstSampler constTrue(true);
    ConstSampler constFalse(false);
    ProbabilisticSampler prob(0.1);
    RateLimitingSampler rate(0.1);
    SamplerOptions options;
    options.setSampler(std::make_shared<ConstSampler>(true));
    // TODO: RemotelyControlledSampler remote("", options);

    const struct {
        Sampler& _sampler;
        Tag::ValueType _typeTag;
        Tag::ValueType _paramTag;
    } tests[] = { { constTrue, "const", true },
                  { constFalse, "const", false },
                  { prob, "probabilistic", 0.1 },
                  { rate, "ratelimiting", 0.1 } };
                  // TODO: { remote, "const", true } };

    for (auto&& test : tests) {
        const auto tags
            = test._sampler.isSampled(TraceID(), kTestOperationName).tags();
        auto count = 0;
        for (auto&& tag : tags) {
            if (tag.key() == kSamplerTypeTagKey) {
                ASSERT_EQ(test._typeTag, tag.value());
                ++count;
            }
            else if (tag.key() == kSamplerParamTagKey) {
                ASSERT_EQ(test._paramTag, tag.value());
                ++count;
            }
        }
        ASSERT_EQ(2, count);
    }
}

TEST(Sampler, testSamplerOptions)
{
    SamplerOptions options;
    ASSERT_EQ(Sampler::Type::kProbabilisticSampler, options.sampler()->type());
    auto sampler
        = std::static_pointer_cast<ProbabilisticSampler>(options.sampler());
    ASSERT_EQ(0.001, sampler->samplingRate());
    ASSERT_NE(0, options.maxOperations());
    ASSERT_FALSE(options.samplingServerURL().empty());
    ASSERT_TRUE(static_cast<bool>(options.metrics()));
    ASSERT_NE(0, options.samplingRefreshInterval().count());
}

TEST(Sampler, testProbabilisticSamplerErrors)
{
    ProbabilisticSampler sampler(-0.1);
    ASSERT_LE(0, sampler.samplingRate());
    ASSERT_GE(1, sampler.samplingRate());
    sampler = ProbabilisticSampler(1.1);
    ASSERT_LE(0, sampler.samplingRate());
    ASSERT_GE(1, sampler.samplingRate());
}

TEST(Sampler, testProbabilisticSampler)
{
    ProbabilisticSampler sampler(0.5);
    auto result
        = sampler.isSampled(TraceID(0, kTestMaxID + 10), kTestOperationName);
    ASSERT_FALSE(result.isSampled());
    CMP_TAGS(testProbablisticExpectedTags, result.tags());

    result = sampler.isSampled(TraceID(0, kTestMaxID - 20), kTestOperationName);
    ASSERT_TRUE(result.isSampled());
    CMP_TAGS(testProbablisticExpectedTags, result.tags());
}

TEST(Sampler, DISABLED_testProbabilisticSamplerPerformance)
{
    constexpr auto kNumSamples = static_cast<uint64_t>(100000000);

    ProbabilisticSampler sampler(0.01);
    std::random_device randomDevice;
    std::default_random_engine randomGenerator(randomDevice());
    std::uniform_int_distribution<uint64_t> distribution;
    auto count = static_cast<uint64_t>(0);
    for (auto i = static_cast<uint64_t>(0); i < kNumSamples; ++i) {
        TraceID id(0, distribution(randomGenerator));
        if (sampler.isSampled(id, kTestOperationName).isSampled()) {
            ++count;
        }
    }
    const auto rate = static_cast<double>(count) / kNumSamples;
    std::cout << "Sampled: " << count << " rate=" << rate << '\n';
}

TEST(Sampler, testRateLimitingSampler)
{
    {
        RateLimitingSampler sampler(2);
        auto result = sampler.isSampled(TraceID(), kTestOperationName);
        ASSERT_TRUE(result.isSampled());
        result = sampler.isSampled(TraceID(), kTestOperationName);
        ASSERT_TRUE(result.isSampled());
        result = sampler.isSampled(TraceID(), kTestOperationName);
        ASSERT_FALSE(result.isSampled());
    }

    {
        RateLimitingSampler sampler(0.1);
        auto result = sampler.isSampled(TraceID(), kTestOperationName);
        ASSERT_TRUE(result.isSampled());
        result = sampler.isSampled(TraceID(), kTestOperationName);
        ASSERT_FALSE(result.isSampled());
    }
}

TEST(Sampler, testGuaranteedThroughputProbabilisticSamplerUpdate)
{
    auto lowerBound = 2.0;
    auto samplingRate = 0.5;
    GuaranteedThroughputProbabilisticSampler sampler(lowerBound, samplingRate);
    ASSERT_EQ(lowerBound, sampler.lowerBound());
    ASSERT_EQ(samplingRate, sampler.samplingRate());

    auto newLowerBound = 1.0;
    auto newSamplingRate = 0.6;
    sampler.update(newLowerBound, newSamplingRate);
    ASSERT_EQ(newLowerBound, sampler.lowerBound());
    ASSERT_EQ(newSamplingRate, sampler.samplingRate());

    newSamplingRate = 1.1;
    sampler.update(newLowerBound, newSamplingRate);
    ASSERT_EQ(1.0, sampler.samplingRate());
}

TEST(Sampler, testAdaptiveSampler)
{
    namespace thriftgen = thrift::sampling_manager;

    thriftgen::OperationSamplingStrategy strategy;
    strategy.__set_operation(kTestOperationName);
    thriftgen::ProbabilisticSamplingStrategy probabilisticSampling;
    probabilisticSampling.__set_samplingRate(kTestDefaultSamplingProbability);
    strategy.__set_probabilisticSampling(probabilisticSampling);

    thriftgen::PerOperationSamplingStrategies strategies;
    strategies.__set_defaultSamplingProbability(
        kTestDefaultSamplingProbability);
    strategies.__set_defaultLowerBoundTracesPerSecond(1.0);
    strategies.__set_perOperationStrategies({ strategy });

    AdaptiveSampler sampler(strategies, kTestDefaultMaxOperations);
    auto result
        = sampler.isSampled(TraceID(0, kTestMaxID + 10), kTestOperationName);
    ASSERT_TRUE(result.isSampled());
    CMP_TAGS(testLowerBoundExpectedTags, result.tags());

    result = sampler.isSampled(TraceID(0, kTestMaxID - 20), kTestOperationName);
    ASSERT_TRUE(result.isSampled());
    CMP_TAGS(testProbablisticExpectedTags, result.tags());

    result = sampler.isSampled(TraceID(0, kTestMaxID + 10), kTestOperationName);
    ASSERT_FALSE(result.isSampled());

    result = sampler.isSampled(TraceID(0, kTestMaxID),
                               kTestFirstTimeOperationName);
    ASSERT_TRUE(result.isSampled());
    CMP_TAGS(testProbablisticExpectedTags, result.tags());
}

TEST(Sampler, testAdaptiveSamplerErrors)
{
    namespace thriftgen = thrift::sampling_manager;

    thriftgen::OperationSamplingStrategy strategy;
    strategy.__set_operation(kTestOperationName);
    thriftgen::ProbabilisticSamplingStrategy probabilisticSampling;
    probabilisticSampling.__set_samplingRate(-0.1);
    strategy.__set_probabilisticSampling(probabilisticSampling);

    thriftgen::PerOperationSamplingStrategies strategies;
    strategies.__set_defaultSamplingProbability(
        kTestDefaultSamplingProbability);
    strategies.__set_defaultLowerBoundTracesPerSecond(2.0);
    strategies.__set_perOperationStrategies({ strategy });

    {
        AdaptiveSampler sampler(strategies, kTestDefaultMaxOperations);
    }

    {
        strategies.perOperationStrategies.at(0)
            .probabilisticSampling.__set_samplingRate(1.1);
        AdaptiveSampler sampler(strategies, kTestDefaultMaxOperations);
    }
}

TEST(Sampler, testAdaptiveSamplerUpdate)
{
    namespace thriftgen = thrift::sampling_manager;

    constexpr auto kSamplingRate = 0.1;
    constexpr auto kLowerBound = 2.0;

    thriftgen::OperationSamplingStrategy strategy;
    strategy.__set_operation(kTestOperationName);
    thriftgen::ProbabilisticSamplingStrategy probabilisticSampling;
    probabilisticSampling.__set_samplingRate(kSamplingRate);
    strategy.__set_probabilisticSampling(probabilisticSampling);

    thriftgen::PerOperationSamplingStrategies strategies;
    strategies.__set_defaultSamplingProbability(
        kTestDefaultSamplingProbability);
    strategies.__set_defaultLowerBoundTracesPerSecond(kLowerBound);
    strategies.__set_perOperationStrategies({ strategy });

    AdaptiveSampler sampler(strategies, kTestDefaultMaxOperations);

    constexpr auto kNewSamplingRate = 0.2;
    constexpr auto kNewLowerBound = 3.0;
    constexpr auto kNewDefaultSamplingProbability = 0.1;

    // Updated kTestOperationName strategy.
    thriftgen::OperationSamplingStrategy updatedStrategy;
    updatedStrategy.__set_operation(kTestOperationName);
    thriftgen::ProbabilisticSamplingStrategy updatedProbabilisticSampling;
    updatedProbabilisticSampling.__set_samplingRate(kNewSamplingRate);
    updatedStrategy.__set_probabilisticSampling(updatedProbabilisticSampling);

    // New kTestFirstTimeOperationName strategy.
    thriftgen::OperationSamplingStrategy newStrategy;
    newStrategy.__set_operation(kTestFirstTimeOperationName);
    thriftgen::ProbabilisticSamplingStrategy newProbabilisticSampling;
    newProbabilisticSampling.__set_samplingRate(kNewSamplingRate);
    newStrategy.__set_probabilisticSampling(newProbabilisticSampling);

    thriftgen::PerOperationSamplingStrategies newStrategies;
    newStrategies.__set_defaultSamplingProbability(
        kNewDefaultSamplingProbability);
    newStrategies.__set_defaultLowerBoundTracesPerSecond(kNewLowerBound);
    newStrategies.__set_perOperationStrategies(
        { updatedStrategy, newStrategy });

    sampler.update(newStrategies);
}

}  // namespace samplers
}  // namespace jaeger
}  // namespace uber

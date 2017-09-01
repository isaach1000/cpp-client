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

namespace uber {
namespace jaeger {
namespace samplers {
namespace {

constexpr auto kTestOperationName = "op";
constexpr auto kTestFirstTimeOperationName = "firstTimeOp";
constexpr auto kTestDefaultSamplingProbability = 0.5;
constexpr auto kTestMaxID = static_cast<uint64_t>(1) << 62;
constexpr auto kDefaultMaxOperations = 10;

const Tag testProbablisticExpectedTags[] = {
    {"sampler.type", "probabilistic"},
    {"sampler.param", 0.5}
};

const Tag testLowerBoundExpectedTags[] = {
    {"sampler.type", "lowerbound"},
    {"sampler.param", 0.5}
};

}  // anonymous namespace

TEST(Sampler, testSamplerTags)
{
    ConstSampler constTrue(true);
    ConstSampler constFalse(false);
    ProbabilisticSampler prob(0.1);
    RateLimitingSampler rate(0.1);
    SamplerOptions options;
    options.setSampler(std::make_shared<ConstSampler>(true));
    RemotelyControlledSampler remote("", options);

    const struct {
        Sampler& _sampler;
        Tag::ValueType _typeTag;
        Tag::ValueType _paramTag;
    } tests[] = {
        {constTrue, "const", true},
        {constFalse, "const", false},
        {prob, "probabilistic", 0.1},
        {rate, "ratelimiting", 0.1},
        {remote, "const", true}
    };

    for (auto&& test : tests) {
        const auto tags =
            test._sampler.isSampled(TraceID(), kTestOperationName).tags();
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

}  // namespace samplers
}  // namespace jaeger
}  // namespace uber

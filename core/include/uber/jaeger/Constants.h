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

#ifndef UBER_JAEGER_CONSTANTS_H
#define UBER_JAEGER_CONSTANTS_H

namespace uber {
namespace jaeger {

// TODO: kJaegerVersionClientVersion
static const auto kJaegerClientVersionTagKey = "jaeger.version";
static const auto kJaegerDebugHeader = "jaeger-debug-id";
static const auto kJaegerBaggageHeader = "jaeger-baggage";
static const auto kTracerHostnameTagKey = "hostname";
static const auto kTracerIPTagKey = "ip";
static const auto kSamplerTypeTagKey = "sampler.type";
static const auto kSamplerParamTagKey = "sampler.param";
static const auto kTraceContextHeaderName = "uber-trace-id";
static const auto kTracerStateHeaderName = kTraceContextHeaderName;
static const auto kTraceBaggageHeaderPrefix = "uberctx-";
static const auto kSamplerTypeConst = "const";
static const auto kSamplerTypeRemote = "remote";
static const auto kSamplerTypeProbabilistic = "probabilistic";
static const auto kSamplerTypeRateLimiting = "ratelimiting";
static const auto kSamplerTypeLowerBound = "lowerbound";

}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_CONSTANTS_H

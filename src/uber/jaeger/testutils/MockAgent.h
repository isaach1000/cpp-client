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

#ifndef UBER_JAEGER_TESTUTILS_MOCKAGENT_H
#define UBER_JAEGER_TESTUTILS_MOCKAGENT_H

#include <atomic>
#include <mutex>
#include <vector>

#include "uber/jaeger/testutils/SamplingManager.h"
#include "uber/jaeger/testutils/TUDPTransport.h"
#include "uber/jaeger/thrift-gen/jaeger_types.h"

namespace uber {
namespace jaeger {
namespace testutils {

class MockAgent {
  public:

  private:
    TUDPTransport _transport;
    std::vector<thrift::Batch> _batches;
    std::mutex _mutex;
    std::atomic<bool> _serving;
    SamplingManager _samplingMgr;
};

}  // namespace testutils
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_TESTUTILS_MOCKAGENT_H

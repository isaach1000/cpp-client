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

#ifndef UBER_JAEGER_UDPTRANSPORT_H
#define UBER_JAEGER_UDPTRANSPORT_H

#include "uber/jaeger/Span.h"
#include "uber/jaeger/Transport.h"
#include "uber/jaeger/thrift-gen/jaeger_types.h"
#include "uber/jaeger/utils/UDPClient.h"

namespace uber {
namespace jaeger {

class UDPTransport : public Transport {
  public:
    UDPTransport(const char* ip, int port, int maxPacketSize);

    int append(const Span& span) override;

    int flush() override;

    void close() override { _client->close(); }

  private:
    static constexpr auto kEmitBatchOverhead = 30;

    void resetBuffers()
    {
        _spanBuffer.clear();
        _byteBufferSize = _processByteSize;
    }

    std::unique_ptr<utils::UDPClient> _client;
    int _maxSpanBytes;
    int _byteBufferSize;
    std::vector<thrift::Span> _spanBuffer;
    boost::shared_ptr<apache::thrift::protocol::TProtocol> _protocol;
    thrift::Process _process;
    int _processByteSize;
};

}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_UDPTRANSPORT_H

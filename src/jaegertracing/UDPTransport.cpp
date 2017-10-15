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

#include "jaegertracing/UDPTransport.h"

namespace jaegertracing {
namespace {

template <typename ThriftType>
int calcSizeOfSerializedThrift(
    const ThriftType& base,
    const boost::shared_ptr<apache::thrift::protocol::TProtocol>& protocol,
    int maxPacketSize)
{
    apache::thrift::transport::TMemoryBuffer buffer(maxPacketSize);
    buffer.resetBuffer();
    base.write(protocol.get());
    uint8_t* data = nullptr;
    uint32_t size = 0;
    buffer.getBuffer(&data, &size);
    return size;
}

}  // anonymous namespace

UDPTransport::UDPTransport(const net::IPAddress& ip, int maxPacketSize)
    : _client(new utils::UDPClient(ip, maxPacketSize))
    , _maxSpanBytes(maxPacketSize - kEmitBatchOverhead)
    , _byteBufferSize(0)
    , _spanBuffer()
    , _protocol(_client->protocol())
    , _process()
    , _processByteSize(calcSizeOfSerializedThrift(
          _process, _protocol, _client->maxPacketSize()))
{
}

int UDPTransport::append(const Span& span)
{
    /* TODO: Set process */
    const auto jaegerSpan = span.thrift();
    const auto spanSize = calcSizeOfSerializedThrift(
        jaegerSpan, _protocol, _client->maxPacketSize());
    if (spanSize > _maxSpanBytes) {
        std::ostringstream oss;
        throw Transport::Exception("Span is too large", 1);
    }

    _byteBufferSize += spanSize;
    if (_byteBufferSize <= _maxSpanBytes) {
        _spanBuffer.push_back(jaegerSpan);
        if (_byteBufferSize < _maxSpanBytes) {
            return 0;
        }
        return flush();
    }

    // Flush currently full buffer, then append this span to buffer.
    const auto flushed = flush();
    _spanBuffer.push_back(jaegerSpan);
    _byteBufferSize = spanSize + _processByteSize;
    return flushed;
}

int UDPTransport::flush()
{
    if (_spanBuffer.empty()) {
        return 0;
    }

    thrift::Batch batch;
    batch.__set_process(_process);
    batch.__set_spans(_spanBuffer);
    _client->emitBatch(batch);

    resetBuffers();

    return batch.spans.size();
}

}  // namespace jaegertracing

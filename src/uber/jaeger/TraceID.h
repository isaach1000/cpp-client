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

#ifndef UBER_JAEGER_TRACEID_H
#define UBER_JAEGER_TRACEID_H

#include <iomanip>
#include <iostream>

namespace uber {
namespace jaeger {

class TraceID {
  public:
    static TraceID fromStream(std::istream& in);

    TraceID()
        : TraceID(0, 0)
    {
    }

    TraceID(uint64_t high, uint64_t low)
        : _high(high)
        , _low(low)
    {
    }

    bool isValid() const { return _high != 0 || _low != 0; }

    template <typename Stream>
    void print(Stream& out) const
    {
        if (_high == 0) {
            out << std::hex << _low;
        }
        else {
            out << std::hex << _high << std::setw(16) << std::setfill('0')
                << std::hex << _low;
        }
    }

    uint64_t high() const { return _high; }

    uint64_t low() const { return _low; }

  private:
    uint64_t _high;
    uint64_t _low;
};

}  // namespace jaeger
}  // namespace uber

template <typename Stream>
inline Stream& operator<<(Stream& out, const uber::jaeger::TraceID& traceID)
{
    traceID.print(out);
    return out;
}

#endif  // UBER_JAEGER_TRACEID_H

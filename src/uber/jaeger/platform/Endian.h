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

#ifndef UBER_JAEGER_PLATFORM_ENDIAN_H
#define UBER_JAEGER_PLATFORM_ENDIAN_H

#include <cstdint>

#if defined(__APPLE__)
#include <machine/endian.h>
#elif defined(__linux__)
#include <endian.h>
#else
#error "unsupported platform"
#endif

namespace uber {
namespace jaeger {
namespace platform {
namespace endian {

inline uint16_t toBigEndian(uint16_t value) { return htobe16(value); }

inline uint32_t toBigEndian(uint32_t value) { return htobe32(value); }

inline uint64_t toBigEndian(uint64_t value) { return htobe64(value); }

inline uint16_t fromBigEndian(uint16_t value) { return be16toh(value); }

inline uint32_t fromBigEndian(uint32_t value) { return be32toh(value); }

inline uint64_t fromBigEndian(uint64_t value) { return be64toh(value); }

}  // namespace endian
}  // namespace platform
}  // namespace jaeger
}  // namespace uber

#endif  // UBER_JAEGER_PLATFORM_ENDIAN_H

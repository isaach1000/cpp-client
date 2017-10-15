/*
 * Copyright (c) 2017 Uber Technologies, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "jaegertracing/Span.h"

#include "jaegertracing/Tracer.h"

namespace jaegertracing {

const opentracing::Tracer& Span::tracer() const noexcept
{
    std::lock_guard<std::mutex> lock(_mutex);
    std::shared_ptr<const opentracing::Tracer> tracer(_tracer.lock());
    if (tracer) {
        return *tracer;
    }
    tracer = opentracing::Tracer::Global();
    assert(tracer);
    return *tracer;
}

}  // namespace jaegertracing

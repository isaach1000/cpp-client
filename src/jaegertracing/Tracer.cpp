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

#include "jaegertracing/Tracer.h"

#include "jaegertracing/Reference.h"

namespace jaegertracing {

std::unique_ptr<opentracing::Span> Tracer::StartSpanWithOptions(
    string_view operationName,
    const opentracing::StartSpanOptions& options) const noexcept
{
    std::vector<Reference> references;
    SpanContext parent;
    auto hasParent = false;
    for (auto&& ref : options.references) {
        // TODO: See if we can avoid `dynamic_cast`.
        auto ctx = dynamic_cast<const SpanContext*>(ref.second);
        if (!ctx) {
            _logger->error(
                "Reference contains invalid type of SpanReference: {0}",
                ref.second);
            continue;
        }
        if (!ctx->isValid() || ctx->isDebugIDContainerOnly()) {
            continue;
        }
        references.push_back(Reference(*ctx, ref.first));

        if (!hasParent) {
            parent = *ctx;
            hasParent =
                (ref.first == opentracing::SpanReferenceType::ChildOfRef);
        }
    }

    if (!hasParent && parent.isValid()) {
        hasParent = true;
    }

    // TODO
}

}  // namespace jaegertracing

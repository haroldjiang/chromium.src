/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "core/css/resolver/StyleResolverStats.h"

#include <memory>

namespace blink {

void StyleResolverStats::reset() {
  sharedStyleLookups = 0;
  sharedStyleCandidates = 0;
  sharedStyleFound = 0;
  sharedStyleMissed = 0;
  sharedStyleRejectedByUncommonAttributeRules = 0;
  sharedStyleRejectedBySiblingRules = 0;
  sharedStyleRejectedByParent = 0;
  matchedPropertyApply = 0;
  matchedPropertyCacheHit = 0;
  matchedPropertyCacheInheritedHit = 0;
  matchedPropertyCacheAdded = 0;
  rulesFastRejected = 0;
  rulesRejected = 0;
  rulesMatched = 0;
  stylesChanged = 0;
  stylesUnchanged = 0;
  stylesAnimated = 0;
  elementsStyled = 0;
  pseudoElementsStyled = 0;
  baseStylesUsed = 0;
  independentInheritedStylesPropagated = 0;
  customPropertiesApplied = 0;
}

bool StyleResolverStats::allCountersEnabled() const {
  bool allCountersEnabled;
  TRACE_EVENT_CATEGORY_GROUP_ENABLED(TRACE_DISABLED_BY_DEFAULT("blink.style"),
                                     &allCountersEnabled);
  return allCountersEnabled;
}

std::unique_ptr<TracedValue> StyleResolverStats::toTracedValue() const {
  std::unique_ptr<TracedValue> tracedValue = TracedValue::create();
  tracedValue->setInteger("sharedStyleLookups", sharedStyleLookups);
  tracedValue->setInteger("sharedStyleCandidates", sharedStyleCandidates);
  tracedValue->setInteger("sharedStyleFound", sharedStyleFound);
  if (allCountersEnabled())
    tracedValue->setInteger("sharedStyleMissed", sharedStyleMissed);
  tracedValue->setInteger("sharedStyleRejectedByUncommonAttributeRules",
                          sharedStyleRejectedByUncommonAttributeRules);
  tracedValue->setInteger("sharedStyleRejectedBySiblingRules",
                          sharedStyleRejectedBySiblingRules);
  tracedValue->setInteger("sharedStyleRejectedByParent",
                          sharedStyleRejectedByParent);
  tracedValue->setInteger("matchedPropertyApply", matchedPropertyApply);
  tracedValue->setInteger("matchedPropertyCacheHit", matchedPropertyCacheHit);
  tracedValue->setInteger("matchedPropertyCacheInheritedHit",
                          matchedPropertyCacheInheritedHit);
  tracedValue->setInteger("matchedPropertyCacheAdded",
                          matchedPropertyCacheAdded);
  tracedValue->setInteger("rulesRejected", rulesRejected);
  tracedValue->setInteger("rulesFastRejected", rulesFastRejected);
  tracedValue->setInteger("rulesMatched", rulesMatched);
  tracedValue->setInteger("stylesChanged", stylesChanged);
  tracedValue->setInteger("stylesUnchanged", stylesUnchanged);
  tracedValue->setInteger("stylesAnimated", stylesAnimated);
  tracedValue->setInteger("elementsStyled", elementsStyled);
  tracedValue->setInteger("pseudoElementsStyled", pseudoElementsStyled);
  tracedValue->setInteger("baseStylesUsed", baseStylesUsed);
  tracedValue->setInteger("independentInheritedStylesPropagated",
                          independentInheritedStylesPropagated);
  tracedValue->setInteger("customPropertiesApplied", customPropertiesApplied);
  return tracedValue;
}

}  // namespace blink

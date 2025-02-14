/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "modules/accessibility/AXSpinButton.h"

#include "SkMatrix44.h"
#include "modules/accessibility/AXObjectCacheImpl.h"

namespace blink {

AXSpinButton* AXSpinButton::create(AXObjectCacheImpl& axObjectCache) {
  return new AXSpinButton(axObjectCache);
}

AXSpinButton::AXSpinButton(AXObjectCacheImpl& axObjectCache)
    : AXMockObject(axObjectCache), m_spinButtonElement(nullptr) {}

AXSpinButton::~AXSpinButton() {
  ASSERT(!m_spinButtonElement);
}

DEFINE_TRACE(AXSpinButton) {
  visitor->trace(m_spinButtonElement);
  AXMockObject::trace(visitor);
}

LayoutObject* AXSpinButton::layoutObjectForRelativeBounds() const {
  if (!m_spinButtonElement || !m_spinButtonElement->layoutObject())
    return nullptr;

  return m_spinButtonElement->layoutObject();
}

void AXSpinButton::detach() {
  AXObject::detach();
  m_spinButtonElement = nullptr;
}

void AXSpinButton::detachFromParent() {
  AXObject::detachFromParent();
  m_spinButtonElement = nullptr;
}

AccessibilityRole AXSpinButton::roleValue() const {
  return m_spinButtonElement ? SpinButtonRole : UnknownRole;
}

void AXSpinButton::addChildren() {
  ASSERT(!isDetached());
  m_haveChildren = true;

  AXSpinButtonPart* incrementor =
      toAXSpinButtonPart(axObjectCache().getOrCreate(SpinButtonPartRole));
  incrementor->setIsIncrementor(true);
  incrementor->setParent(this);
  m_children.push_back(incrementor);

  AXSpinButtonPart* decrementor =
      toAXSpinButtonPart(axObjectCache().getOrCreate(SpinButtonPartRole));
  decrementor->setIsIncrementor(false);
  decrementor->setParent(this);
  m_children.push_back(decrementor);
}

void AXSpinButton::step(int amount) {
  ASSERT(m_spinButtonElement);
  if (!m_spinButtonElement)
    return;

  m_spinButtonElement->step(amount);
}

// AXSpinButtonPart

AXSpinButtonPart::AXSpinButtonPart(AXObjectCacheImpl& axObjectCache)
    : AXMockObject(axObjectCache), m_isIncrementor(false) {}

AXSpinButtonPart* AXSpinButtonPart::create(AXObjectCacheImpl& axObjectCache) {
  return new AXSpinButtonPart(axObjectCache);
}

void AXSpinButtonPart::getRelativeBounds(
    AXObject** outContainer,
    FloatRect& outBoundsInContainer,
    SkMatrix44& outContainerTransform) const {
  *outContainer = nullptr;
  outBoundsInContainer = FloatRect();
  outContainerTransform.setIdentity();

  if (!parentObject())
    return;

  // FIXME: This logic should exist in the layout tree or elsewhere, but there
  // is no relationship that exists that can be queried.
  parentObject()->getRelativeBounds(outContainer, outBoundsInContainer,
                                    outContainerTransform);
  outBoundsInContainer = FloatRect(0, 0, outBoundsInContainer.width(),
                                   outBoundsInContainer.height());
  if (m_isIncrementor) {
    outBoundsInContainer.setHeight(outBoundsInContainer.height() / 2);
  } else {
    outBoundsInContainer.setY(outBoundsInContainer.y() +
                              outBoundsInContainer.height() / 2);
    outBoundsInContainer.setHeight(outBoundsInContainer.height() / 2);
  }
  *outContainer = parentObject();
}

bool AXSpinButtonPart::press() {
  if (!m_parent || !m_parent->isSpinButton())
    return false;

  AXSpinButton* spinButton = toAXSpinButton(parentObject());
  if (m_isIncrementor)
    spinButton->step(1);
  else
    spinButton->step(-1);

  return true;
}

}  // namespace blink

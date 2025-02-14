// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FrameClient_h
#define FrameClient_h

#include "core/CoreExport.h"
#include "platform/heap/Handle.h"
#include "public/platform/BlameContext.h"

namespace blink {

class Frame;
enum class FrameDetachType;

class CORE_EXPORT FrameClient : public GarbageCollectedFinalized<FrameClient> {
 public:
  virtual bool inShadowTree() const = 0;

  // TODO(dcheng): Move this into LocalFrameClient, since remote frames don't
  // need this.
  virtual void willBeDetached() = 0;
  virtual void detached(FrameDetachType) = 0;

  virtual Frame* opener() const = 0;
  virtual void setOpener(Frame*) = 0;

  virtual Frame* parent() const = 0;
  virtual Frame* top() const = 0;
  virtual Frame* nextSibling() const = 0;
  virtual Frame* firstChild() const = 0;

  virtual unsigned backForwardLength() = 0;

  virtual void frameFocused() const = 0;

  virtual ~FrameClient() {}

  DEFINE_INLINE_VIRTUAL_TRACE() {}
};

}  // namespace blink

#endif  // FrameClient_h

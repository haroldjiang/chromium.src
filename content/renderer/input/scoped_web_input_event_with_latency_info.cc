// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/input/scoped_web_input_event_with_latency_info.h"

using blink::WebInputEvent;

namespace content {

ScopedWebInputEventWithLatencyInfo::ScopedWebInputEventWithLatencyInfo(
    ui::WebScopedInputEvent event,
    const ui::LatencyInfo& latency_info)
    : event_(new blink::WebCoalescedInputEvent(*(event.get()))),
      latency_(latency_info) {}

ScopedWebInputEventWithLatencyInfo::~ScopedWebInputEventWithLatencyInfo() {}

bool ScopedWebInputEventWithLatencyInfo::CanCoalesceWith(
    const ScopedWebInputEventWithLatencyInfo& other) const {
  return ui::CanCoalesce(other.event(), event());
}

void ScopedWebInputEventWithLatencyInfo::CoalesceWith(
    const ScopedWebInputEventWithLatencyInfo& other) {
  // |other| should be a newer event than |this|.
  if (other.latency_.trace_id() >= 0 && latency_.trace_id() >= 0)
    DCHECK_GT(other.latency_.trace_id(), latency_.trace_id());

  // New events get coalesced into older events, and the newer timestamp
  // should always be preserved.
  const double time_stamp_seconds = other.event().timeStampSeconds();
  ui::Coalesce(other.event(), event_->eventPointer());
  event_->eventPointer()->setTimeStampSeconds(time_stamp_seconds);
  event_->addCoalescedEvent(other.event());

  // When coalescing two input events, we keep the oldest LatencyInfo
  // since it will represent the longest latency.
  other.latency_ = latency_;
  other.latency_.set_coalesced();
}

const blink::WebInputEvent& ScopedWebInputEventWithLatencyInfo::event() const {
  return event_->event();
}

blink::WebInputEvent& ScopedWebInputEventWithLatencyInfo::event() {
  return *event_->eventPointer();
}

const blink::WebCoalescedInputEvent&
ScopedWebInputEventWithLatencyInfo::coalesced_event() const {
  return *event_;
}

}  // namespace content

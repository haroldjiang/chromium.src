// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_PLAYBACK_COMPOSITING_DISPLAY_ITEM_H_
#define CC_PLAYBACK_COMPOSITING_DISPLAY_ITEM_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>

#include "base/memory/ptr_util.h"
#include "cc/base/cc_export.h"
#include "cc/playback/display_item.h"
#include "third_party/skia/include/core/SkColorFilter.h"
#include "third_party/skia/include/core/SkRect.h"
#include "third_party/skia/include/core/SkRefCnt.h"
#include "ui/gfx/geometry/rect_f.h"

class SkCanvas;

namespace cc {

class CC_EXPORT CompositingDisplayItem : public DisplayItem {
 public:
  CompositingDisplayItem(uint8_t alpha,
                         SkBlendMode xfermode,
                         SkRect* bounds,
                         sk_sp<SkColorFilter> color_filter,
                         bool lcd_text_requires_opaque_layer);
  ~CompositingDisplayItem() override;

  void Raster(SkCanvas* canvas,
              SkPicture::AbortCallback* callback) const override;
  void AsValueInto(const gfx::Rect& visual_rect,
                   base::trace_event::TracedValue* array) const override;

  size_t ExternalMemoryUsage() const {
    // TODO(pdr): Include color_filter's memory here.
    return 0;
  }
  int ApproximateOpCount() const { return 1; }

 private:
  void SetNew(uint8_t alpha,
              SkBlendMode xfermode,
              SkRect* bounds,
              sk_sp<SkColorFilter> color_filter,
              bool lcd_text_requires_opaque_layer);

  uint8_t alpha_;
  SkBlendMode xfermode_;
  bool has_bounds_;
  SkRect bounds_;
  sk_sp<SkColorFilter> color_filter_;
  bool lcd_text_requires_opaque_layer_;
};

class CC_EXPORT EndCompositingDisplayItem : public DisplayItem {
 public:
  EndCompositingDisplayItem();
  ~EndCompositingDisplayItem() override;

  static std::unique_ptr<EndCompositingDisplayItem> Create() {
    return base::MakeUnique<EndCompositingDisplayItem>();
  }

  void Raster(SkCanvas* canvas,
              SkPicture::AbortCallback* callback) const override;
  void AsValueInto(const gfx::Rect& visual_rect,
                   base::trace_event::TracedValue* array) const override;

  int ApproximateOpCount() const { return 0; }
};

}  // namespace cc

#endif  // CC_PLAYBACK_COMPOSITING_DISPLAY_ITEM_H_

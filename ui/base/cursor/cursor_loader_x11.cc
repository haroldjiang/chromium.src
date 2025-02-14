// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/base/cursor/cursor_loader_x11.h"

#include <float.h>
#include <X11/cursorfont.h>
#include <X11/Xlib.h>

#include "base/logging.h"
#include "build/build_config.h"
#include "skia/ext/image_operations.h"
#include "ui/base/cursor/cursor.h"
#include "ui/base/cursor/cursor_util.h"
#include "ui/base/cursor/cursors_aura.h"
#include "ui/base/x/x11_util.h"
#include "ui/display/display.h"
#include "ui/gfx/geometry/point_conversions.h"
#include "ui/gfx/geometry/size_conversions.h"
#include "ui/gfx/image/image.h"
#include "ui/gfx/skbitmap_operations.h"
#include "ui/gfx/skia_util.h"

namespace {

// Returns CSS cursor name from an Aura cursor ID.
const char* CursorCssNameFromId(int id) {
  switch (id) {
    case ui::kCursorMiddlePanning:
      return "all-scroll";
    case ui::kCursorEastPanning:
      return "e-resize";
    case ui::kCursorNorthPanning:
      return "n-resize";
    case ui::kCursorNorthEastPanning:
      return "ne-resize";
    case ui::kCursorNorthWestPanning:
      return "nw-resize";
    case ui::kCursorSouthPanning:
      return "s-resize";
    case ui::kCursorSouthEastPanning:
      return "se-resize";
    case ui::kCursorSouthWestPanning:
      return "sw-resize";
    case ui::kCursorWestPanning:
      return "w-resize";
    case ui::kCursorNone:
      return "none";
    case ui::kCursorGrab:
      return "grab";
    case ui::kCursorGrabbing:
      return "grabbing";

#if defined(OS_CHROMEOS)
    case ui::kCursorNull:
    case ui::kCursorPointer:
    case ui::kCursorNoDrop:
    case ui::kCursorNotAllowed:
    case ui::kCursorCopy:
    case ui::kCursorMove:
    case ui::kCursorEastResize:
    case ui::kCursorNorthResize:
    case ui::kCursorSouthResize:
    case ui::kCursorWestResize:
    case ui::kCursorNorthEastResize:
    case ui::kCursorNorthWestResize:
    case ui::kCursorSouthWestResize:
    case ui::kCursorSouthEastResize:
    case ui::kCursorIBeam:
    case ui::kCursorAlias:
    case ui::kCursorCell:
    case ui::kCursorContextMenu:
    case ui::kCursorCross:
    case ui::kCursorHelp:
    case ui::kCursorWait:
    case ui::kCursorNorthSouthResize:
    case ui::kCursorEastWestResize:
    case ui::kCursorNorthEastSouthWestResize:
    case ui::kCursorNorthWestSouthEastResize:
    case ui::kCursorProgress:
    case ui::kCursorColumnResize:
    case ui::kCursorRowResize:
    case ui::kCursorVerticalText:
    case ui::kCursorZoomIn:
    case ui::kCursorZoomOut:
    case ui::kCursorHand:
      // In some environments, the image assets are not set (e.g. in
      // content-browsertests, content-shell etc.).
      return "left_ptr";
#else  // defined(OS_CHROMEOS)
    case ui::kCursorNull:
      return "left_ptr";
    case ui::kCursorPointer:
      return "left_ptr";
    case ui::kCursorMove:
      // Returning "move" is the correct thing here, but Blink doesn't
      // make a distinction between move and all-scroll.  Other
      // platforms use a cursor more consistent with all-scroll, so
      // use that.
      return "all-scroll";
    case ui::kCursorCross:
      return "crosshair";
    case ui::kCursorHand:
      return "pointer";
    case ui::kCursorIBeam:
      return "text";
    case ui::kCursorProgress:
      return "progress";
    case ui::kCursorWait:
      return "wait";
    case ui::kCursorHelp:
      return "help";
    case ui::kCursorEastResize:
      return "e-resize";
    case ui::kCursorNorthResize:
      return "n-resize";
    case ui::kCursorNorthEastResize:
      return "ne-resize";
    case ui::kCursorNorthWestResize:
      return "nw-resize";
    case ui::kCursorSouthResize:
      return "s-resize";
    case ui::kCursorSouthEastResize:
      return "se-resize";
    case ui::kCursorSouthWestResize:
      return "sw-resize";
    case ui::kCursorWestResize:
      return "w-resize";
    case ui::kCursorNorthSouthResize:
      return "ns-resize";
    case ui::kCursorEastWestResize:
      return "ew-resize";
    case ui::kCursorColumnResize:
      return "col-resize";
    case ui::kCursorRowResize:
      return "row-resize";
    case ui::kCursorNorthEastSouthWestResize:
      return "nesw-resize";
    case ui::kCursorNorthWestSouthEastResize:
      return "nwse-resize";
    case ui::kCursorVerticalText:
      return "vertical-text";
    case ui::kCursorZoomIn:
      return "zoom-in";
    case ui::kCursorZoomOut:
      return "zoom-out";
    case ui::kCursorCell:
      return "cell";
    case ui::kCursorContextMenu:
      return "context-menu";
    case ui::kCursorAlias:
      return "alias";
    case ui::kCursorNoDrop:
      return "no-drop";
    case ui::kCursorCopy:
      return "copy";
    case ui::kCursorNotAllowed:
      return "not-allowed";
    case ui::kCursorDndNone:
      return "dnd-none";
    case ui::kCursorDndMove:
      return "dnd-move";
    case ui::kCursorDndCopy:
      return "dnd-copy";
    case ui::kCursorDndLink:
      return "dnd-link";
#endif  // defined(OS_CHROMEOS)
    case ui::kCursorCustom:
      NOTREACHED();
      return "left_ptr";
  }
  NOTREACHED() << "Case not handled for " << id;
  return "left_ptr";
}

static const struct {
  const char* css_name;
  const char* fallback_name;
  int fallback_shape;
} kCursorFallbacks[] = {
    { "pointer",     "hand",            XC_hand1 },
    { "progress",    "left_ptr_watch",  XC_watch },
    { "wait",        nullptr,           XC_watch },
    { "cell",        nullptr,           XC_plus },
    { "all-scroll",  nullptr,           XC_fleur},
    { "crosshair",   nullptr,           XC_cross },
    { "text",        nullptr,           XC_xterm },
    { "not-allowed", "crossed_circle",  None },
    { "grabbing",    nullptr,           XC_hand2 },
    { "col-resize",  nullptr,           XC_sb_h_double_arrow },
    { "row-resize",  nullptr,           XC_sb_v_double_arrow},
    { "n-resize",    nullptr,           XC_top_side},
    { "e-resize",    nullptr,           XC_right_side},
    { "s-resize",    nullptr,           XC_bottom_side},
    { "w-resize",    nullptr,           XC_left_side},
    { "ne-resize",   nullptr,           XC_top_right_corner},
    { "nw-resize",   nullptr,           XC_top_left_corner},
    { "se-resize",   nullptr,           XC_bottom_right_corner},
    { "sw-resize",   nullptr,           XC_bottom_left_corner},
    { "ew-resize",   nullptr,           XC_sb_h_double_arrow},
    { "ns-resize",   nullptr,           XC_sb_v_double_arrow},
    { "nesw-resize", "fd_double_arrow", None},
    { "nwse-resize", "bd_double_arrow", None},
    { "dnd-none",    "grabbing",        XC_hand2 },
    { "dnd-move",    "grabbing",        XC_hand2 },
    { "dnd-copy",    "grabbing",        XC_hand2 },
    { "dnd-link",    "grabbing",        XC_hand2 },
};

}  // namespace

namespace ui {

CursorLoader* CursorLoader::Create() {
  return new CursorLoaderX11;
}

CursorLoaderX11::ImageCursor::ImageCursor(XcursorImage* x_image,
                                          float scale,
                                          display::Display::Rotation rotation)
    : scale(scale), rotation(rotation) {
  cursor = CreateReffedCustomXCursor(x_image);
}

CursorLoaderX11::ImageCursor::~ImageCursor() {
  UnrefCustomXCursor(cursor);
}

CursorLoaderX11::CursorLoaderX11()
    : display_(gfx::GetXDisplay()),
      invisible_cursor_(CreateInvisibleCursor(), gfx::GetXDisplay()) {}

CursorLoaderX11::~CursorLoaderX11() {
  UnloadAll();
}

void CursorLoaderX11::LoadImageCursor(int id,
                                      int resource_id,
                                      const gfx::Point& hot) {
  SkBitmap bitmap;
  gfx::Point hotspot = hot;

  GetImageCursorBitmap(resource_id, scale(), rotation(), &hotspot, &bitmap);
  XcursorImage* x_image = SkBitmapToXcursorImage(&bitmap, hotspot);
  image_cursors_[id].reset(new ImageCursor(x_image, scale(), rotation()));
}

void CursorLoaderX11::LoadAnimatedCursor(int id,
                                         int resource_id,
                                         const gfx::Point& hot,
                                         int frame_delay_ms) {
  std::vector<SkBitmap> bitmaps;
  gfx::Point hotspot = hot;

  GetAnimatedCursorBitmaps(resource_id, scale(), rotation(), &hotspot,
                           &bitmaps);

  XcursorImages* x_images = XcursorImagesCreate(bitmaps.size());
  x_images->nimage = bitmaps.size();

  for (unsigned int frame = 0; frame < bitmaps.size(); ++frame) {
    XcursorImage* x_image = SkBitmapToXcursorImage(&bitmaps[frame], hotspot);
    x_image->delay = frame_delay_ms;
    x_images->images[frame] = x_image;
  }

  animated_cursors_[id] = std::make_pair(
      XcursorImagesLoadCursor(gfx::GetXDisplay(), x_images), x_images);
}

void CursorLoaderX11::UnloadAll() {
  image_cursors_.clear();

  // Free animated cursors and images.
  for (const auto& cursor : animated_cursors_) {
    XcursorImagesDestroy(
        cursor.second.second);  // also frees individual frames.
    XFreeCursor(gfx::GetXDisplay(), cursor.second.first);
  }
}

void CursorLoaderX11::SetPlatformCursor(gfx::NativeCursor* cursor) {
  DCHECK(cursor);

  if (*cursor == kCursorNone) {
    cursor->SetPlatformCursor(invisible_cursor_.get());
    return;
  }

  if (*cursor == kCursorCustom)
    return;

  cursor->set_device_scale_factor(scale());
  cursor->SetPlatformCursor(CursorFromId(cursor->native_type()));
}

const XcursorImage* CursorLoaderX11::GetXcursorImageForTest(int id) {
  return test::GetCachedXcursorImage(image_cursors_[id]->cursor);
}

bool CursorLoaderX11::IsImageCursor(gfx::NativeCursor native_cursor) {
  int type = native_cursor.native_type();
  return image_cursors_.count(type) || animated_cursors_.count(type);
}

::Cursor CursorLoaderX11::CursorFromId(int id) {
  const char* css_name = CursorCssNameFromId(id);

  auto font_it = font_cursors_.find(id);
  if (font_it != font_cursors_.end())
    return font_it->second;
  auto image_it = image_cursors_.find(id);
  if (image_it != image_cursors_.end()) {
    if (image_it->second->scale == scale() &&
        image_it->second->rotation == rotation()) {
      return image_it->second->cursor;
    } else {
      image_cursors_.erase(image_it);
    }
  }

  // First try to load the cursor directly.
  ::Cursor cursor = XcursorLibraryLoadCursor(display_, css_name);
  if (cursor == None) {
    // Try a similar cursor supplied by the native cursor theme.
    for (const auto& mapping : kCursorFallbacks) {
      if (strcmp(mapping.css_name, css_name) == 0) {
        if (mapping.fallback_name)
          cursor = XcursorLibraryLoadCursor(display_, mapping.fallback_name);
        if (cursor == None && mapping.fallback_shape)
          cursor = XCreateFontCursor(display_, mapping.fallback_shape);
      }
    }
  }
  if (cursor != None) {
    font_cursors_[id] = cursor;
    return cursor;
  }

  // If the theme is missing the desired cursor, use a chromium-supplied
  // fallback icon.
  int resource_id;
  gfx::Point point;
  if (ui::GetCursorDataFor(ui::CURSOR_SET_NORMAL, id, scale(), &resource_id,
                           &point)) {
    LoadImageCursor(id, resource_id, point);
    return image_cursors_[id]->cursor;
  }

  // As a last resort, return a left pointer.
  cursor = XCreateFontCursor(display_, XC_left_ptr);
  DCHECK(cursor);
  font_cursors_[id] = cursor;
  return cursor;
}

}  // namespace ui

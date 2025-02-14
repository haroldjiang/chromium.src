// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_TILES_TILE_MANAGER_SETTINGS_H_
#define CC_TILES_TILE_MANAGER_SETTINGS_H_

#include "cc/base/cc_export.h"

namespace cc {

struct CC_EXPORT TileManagerSettings {
  bool use_partial_raster = false;
  bool enable_checker_imaging = false;
};

}  // namespace cc

#endif  // CC_TILES_TILE_MANAGER_SETTINGS_H_

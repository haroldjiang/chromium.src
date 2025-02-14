// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "services/ui/ws/window_manager_display_root.h"

#include <string>
#include <vector>

#include "services/ui/public/interfaces/window_manager.mojom.h"
#include "services/ui/ws/display.h"
#include "services/ui/ws/display_manager.h"
#include "services/ui/ws/server_window.h"
#include "services/ui/ws/window_server.h"

namespace ui {
namespace ws {

WindowManagerDisplayRoot::WindowManagerDisplayRoot(Display* display)
    : display_(display) {
  std::string name = "WindowManagerRoot";
  ServerWindow::Properties properties;
  properties[mojom::WindowManager::kName_Property] =
      std::vector<uint8_t>(name.begin(), name.end());

  root_.reset(window_server()->CreateServerWindow(
      window_server()->display_manager()->GetAndAdvanceNextRootId(),
      properties));
  root_->set_event_targeting_policy(
      mojom::EventTargetingPolicy::DESCENDANTS_ONLY);
  // Our root is always a child of the Display's root. Do this
  // before the WindowTree has been created so that the client doesn't get
  // notified of the add, bounds change and visibility change.
  root_->SetBounds(gfx::Rect(display->root_window()->bounds().size()));
  root_->SetVisible(true);
  display->root_window()->Add(root_.get());
}

WindowManagerDisplayRoot::~WindowManagerDisplayRoot() {}

WindowServer* WindowManagerDisplayRoot::window_server() {
  return display_->window_server();
}

}  // namespace ws
}  // namespace ui

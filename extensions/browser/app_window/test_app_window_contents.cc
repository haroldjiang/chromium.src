// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/browser/app_window/test_app_window_contents.h"

#include "content/public/browser/web_contents.h"

namespace extensions {

TestAppWindowContents::TestAppWindowContents(content::WebContents* web_contents)
    : web_contents_(web_contents) {
}

TestAppWindowContents::~TestAppWindowContents() {
}

void TestAppWindowContents::Initialize(content::BrowserContext* context,
                                       content::RenderFrameHost* creator_frame,
                                       const GURL& url,
                                       const Extension* extension) {}

void TestAppWindowContents::LoadContents(int32_t creator_process_id) {}

void TestAppWindowContents::NativeWindowChanged(
    NativeAppWindow* native_app_window) {
}

void TestAppWindowContents::NativeWindowClosed() {
}

void TestAppWindowContents::OnWindowReady() {}

content::WebContents* TestAppWindowContents::GetWebContents() const {
  return web_contents_.get();
}

WindowController* TestAppWindowContents::GetWindowController() const {
  return nullptr;
}

}  // namespace extensions

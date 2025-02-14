// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SHELL_TEST_RUNNER_SPELL_CHECK_CLIENT_H_
#define CONTENT_SHELL_TEST_RUNNER_SPELL_CHECK_CLIENT_H_

#include <stdint.h>

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "content/shell/test_runner/mock_spell_check.h"
#include "third_party/WebKit/public/platform/WebString.h"
#include "third_party/WebKit/public/platform/WebVector.h"
#include "third_party/WebKit/public/web/WebSpellCheckClient.h"
#include "v8/include/v8.h"

namespace blink {
class WebTextCheckingCompletion;
}  // namespace blink

namespace test_runner {

class TestRunner;
class WebTestDelegate;

class SpellCheckClient : public blink::WebSpellCheckClient {
 public:
  explicit SpellCheckClient(TestRunner* test_runner);
  virtual ~SpellCheckClient();

  void SetDelegate(WebTestDelegate* delegate);
  void SetEnabled(bool enabled);

  // Sets a callback that will be invoked after each request is revoled.
  void SetSpellCheckResolvedCallback(v8::Local<v8::Function> callback);

  // Remove the above callback. Beware: don't call it inside the callback.
  void RemoveSpellCheckResolvedCallback();

  void Reset();

  // blink::WebSpellCheckClient implementation.
  void checkSpelling(
      const blink::WebString& text,
      int& offset,
      int& length,
      blink::WebVector<blink::WebString>* optional_suggestions) override;
  void requestCheckingOfText(
      const blink::WebString& text,
      blink::WebTextCheckingCompletion* completion) override;
  void cancelAllPendingRequests() override;

 private:
  void FinishLastTextCheck();

  void RequestResolved();

  // Do not perform any checking when |enabled_ == false|.
  // Tests related to spell checking should enable it manually.
  bool enabled_ = false;

  // The mock spellchecker used in checkSpelling().
  MockSpellCheck spell_check_;

  blink::WebString last_requested_text_check_string_;
  blink::WebTextCheckingCompletion* last_requested_text_checking_completion_;

  v8::Persistent<v8::Function> resolved_callback_;

  TestRunner* test_runner_;
  WebTestDelegate* delegate_;

  base::WeakPtrFactory<SpellCheckClient> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(SpellCheckClient);
};

}  // namespace test_runner

#endif  // CONTENT_SHELL_TEST_RUNNER_SPELL_CHECK_CLIENT_H_

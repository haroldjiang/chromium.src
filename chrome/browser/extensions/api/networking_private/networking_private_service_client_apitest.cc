// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <utility>

#include "base/bind.h"
#include "base/callback.h"
#include "base/command_line.h"
#include "base/macros.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/extensions/api/networking_private/networking_private_credentials_getter.h"
#include "chrome/browser/extensions/extension_apitest.h"
#include "components/user_manager/user.h"
#include "components/user_manager/user_manager.h"
#include "components/wifi/fake_wifi_service.h"
#include "content/public/test/test_utils.h"
#include "extensions/browser/api/networking_private/networking_private_delegate_factory.h"
#include "extensions/browser/api/networking_private/networking_private_event_router.h"
#include "extensions/browser/api/networking_private/networking_private_event_router_factory.h"
#include "extensions/browser/api/networking_private/networking_private_service_client.h"
#include "extensions/common/switches.h"
#include "testing/gmock/include/gmock/gmock.h"

// This tests the Windows / Mac implementation of the networkingPrivate API
// (NetworkingPrivateServiceClient). Note, only a subset of the
// networkingPrivate API is implemented in NetworkingPrivateServiceClient, so
// this uses its own set of test expectations to reflect that. The expectations
// should be kept similar to the ChromeOS (primary) implementation as much as
// possible. See also crbug.com/460119.

using testing::Return;
using testing::_;

using extensions::NetworkingPrivateDelegate;
using extensions::NetworkingPrivateDelegateFactory;
using extensions::NetworkingPrivateEventRouter;
using extensions::NetworkingPrivateEventRouterFactory;
using extensions::NetworkingPrivateServiceClient;

namespace {

// Stub Verify* methods implementation to satisfy expectations of
// networking_private_apitest.
class CryptoVerifyStub : public NetworkingPrivateDelegate::VerifyDelegate {
  void VerifyDestination(
      const VerificationProperties& verification_properties,
      const BoolCallback& success_callback,
      const FailureCallback& failure_callback) override {
    success_callback.Run(true);
  }

  void VerifyAndEncryptCredentials(
      const std::string& guid,
      const VerificationProperties& verification_properties,
      const StringCallback& success_callback,
      const FailureCallback& failure_callback) override {
    success_callback.Run("encrypted_credentials");
  }

  void VerifyAndEncryptData(
      const VerificationProperties& verification_properties,
      const std::string& data,
      const StringCallback& success_callback,
      const FailureCallback& failure_callback) override {
    success_callback.Run("encrypted_data");
  }
};

class NetworkingPrivateServiceClientApiTest : public ExtensionApiTest {
 public:
  NetworkingPrivateServiceClientApiTest() {}

  bool RunNetworkingSubtest(const std::string& subtest) {
    return RunExtensionSubtest("networking_private/service_client",
                               "main.html?" + subtest,
                               kFlagEnableFileAccess | kFlagLoadAsComponent);
  }

  void SetUpCommandLine(base::CommandLine* command_line) override {
    ExtensionApiTest::SetUpCommandLine(command_line);
    // Whitelist the extension ID of the test extension.
    command_line->AppendSwitchASCII(
        extensions::switches::kWhitelistedExtensionID,
        "epcifkihnkjgphfkloaaleeakhpmgdmn");
  }

  static std::unique_ptr<KeyedService> CreateNetworkingPrivateServiceClient(
      content::BrowserContext* context) {
    std::unique_ptr<wifi::FakeWiFiService> wifi_service(
        new wifi::FakeWiFiService());
    std::unique_ptr<CryptoVerifyStub> crypto_verify(new CryptoVerifyStub);
    return std::unique_ptr<KeyedService>(new NetworkingPrivateServiceClient(
        std::move(wifi_service), std::move(crypto_verify)));
  }

  void SetUpOnMainThread() override {
    ExtensionApiTest::SetUpOnMainThread();
    content::RunAllPendingInMessageLoop();
    NetworkingPrivateDelegateFactory::GetInstance()->SetTestingFactory(
        profile(), &CreateNetworkingPrivateServiceClient);
  }

  void TearDownOnMainThread() override {
    content::RunAllPendingInMessageLoop();
    ExtensionApiTest::TearDownOnMainThread();
  }

 protected:
  DISALLOW_COPY_AND_ASSIGN(NetworkingPrivateServiceClientApiTest);
};

// Place each subtest into a separate browser test so that the stub networking
// library state is reset for each subtest run. This way they won't affect each
// other.

IN_PROC_BROWSER_TEST_F(NetworkingPrivateServiceClientApiTest, StartConnect) {
  EXPECT_TRUE(RunNetworkingSubtest("startConnect")) << message_;
}

IN_PROC_BROWSER_TEST_F(NetworkingPrivateServiceClientApiTest, StartDisconnect) {
  EXPECT_TRUE(RunNetworkingSubtest("startDisconnect")) << message_;
}

IN_PROC_BROWSER_TEST_F(NetworkingPrivateServiceClientApiTest,
                       StartConnectNonexistent) {
  EXPECT_TRUE(RunNetworkingSubtest("startConnectNonexistent")) << message_;
}

IN_PROC_BROWSER_TEST_F(NetworkingPrivateServiceClientApiTest,
                       StartDisconnectNonexistent) {
  EXPECT_TRUE(RunNetworkingSubtest("startDisconnectNonexistent")) << message_;
}

IN_PROC_BROWSER_TEST_F(NetworkingPrivateServiceClientApiTest,
                       StartGetPropertiesNonexistent) {
  EXPECT_TRUE(RunNetworkingSubtest("startGetPropertiesNonexistent"))
      << message_;
}

// TODO(stevenjb/mef): Implement |limit| to fix this, crbug.com/371442.
IN_PROC_BROWSER_TEST_F(NetworkingPrivateServiceClientApiTest,
                       DISABLED_GetNetworks) {
  EXPECT_TRUE(RunNetworkingSubtest("getNetworks")) << message_;
}

IN_PROC_BROWSER_TEST_F(NetworkingPrivateServiceClientApiTest,
                       GetVisibleNetworks) {
  EXPECT_TRUE(RunNetworkingSubtest("getVisibleNetworks")) << message_;
}

IN_PROC_BROWSER_TEST_F(NetworkingPrivateServiceClientApiTest,
                       GetVisibleNetworksWifi) {
  EXPECT_TRUE(RunNetworkingSubtest("getVisibleNetworksWifi")) << message_;
}

IN_PROC_BROWSER_TEST_F(NetworkingPrivateServiceClientApiTest,
                       RequestNetworkScan) {
  EXPECT_TRUE(RunNetworkingSubtest("requestNetworkScan")) << message_;
}

IN_PROC_BROWSER_TEST_F(NetworkingPrivateServiceClientApiTest, GetProperties) {
  EXPECT_TRUE(RunNetworkingSubtest("getProperties")) << message_;
}

IN_PROC_BROWSER_TEST_F(NetworkingPrivateServiceClientApiTest, GetState) {
  EXPECT_TRUE(RunNetworkingSubtest("getState")) << message_;
}

IN_PROC_BROWSER_TEST_F(NetworkingPrivateServiceClientApiTest,
                       GetStateNonExistent) {
  EXPECT_TRUE(RunNetworkingSubtest("getStateNonExistent")) << message_;
}

IN_PROC_BROWSER_TEST_F(NetworkingPrivateServiceClientApiTest, SetProperties) {
  EXPECT_TRUE(RunNetworkingSubtest("setProperties")) << message_;
}

IN_PROC_BROWSER_TEST_F(NetworkingPrivateServiceClientApiTest, CreateNetwork) {
  EXPECT_TRUE(RunNetworkingSubtest("createNetwork")) << message_;
}

// TODO(stevenjb/mef): Fix this, crbug.com/371442.
IN_PROC_BROWSER_TEST_F(NetworkingPrivateServiceClientApiTest,
                       DISABLED_GetManagedProperties) {
  EXPECT_TRUE(RunNetworkingSubtest("getManagedProperties")) << message_;
}

IN_PROC_BROWSER_TEST_F(NetworkingPrivateServiceClientApiTest,
                       OnNetworksChangedEventConnect) {
  EXPECT_TRUE(RunNetworkingSubtest("onNetworksChangedEventConnect"))
      << message_;
}

IN_PROC_BROWSER_TEST_F(NetworkingPrivateServiceClientApiTest,
                       OnNetworksChangedEventDisconnect) {
  EXPECT_TRUE(RunNetworkingSubtest("onNetworksChangedEventDisconnect"))
      << message_;
}

IN_PROC_BROWSER_TEST_F(NetworkingPrivateServiceClientApiTest,
                       OnNetworkListChangedEvent) {
  EXPECT_TRUE(RunNetworkingSubtest("onNetworkListChangedEvent")) << message_;
}

IN_PROC_BROWSER_TEST_F(NetworkingPrivateServiceClientApiTest,
                       VerifyDestination) {
  EXPECT_TRUE(RunNetworkingSubtest("verifyDestination")) << message_;
}

IN_PROC_BROWSER_TEST_F(NetworkingPrivateServiceClientApiTest,
                       VerifyAndEncryptCredentials) {
  EXPECT_TRUE(RunNetworkingSubtest("verifyAndEncryptCredentials")) << message_;
}

IN_PROC_BROWSER_TEST_F(NetworkingPrivateServiceClientApiTest,
                       VerifyAndEncryptData) {
  EXPECT_TRUE(RunNetworkingSubtest("verifyAndEncryptData")) << message_;
}

}  // namespace

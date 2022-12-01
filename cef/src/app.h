// SPDX-License-Identifier: GPL-2.0+

// Copyright © 2022 Collabora Ltd
// Copyright © 2022 Valve Corporation

#ifndef APP_H_
#define APP_H_

#include "include/cef_app.h"
#include "include/wrapper/cef_message_router.h"

#include "custom_scheme.h"

class App : public CefApp, public CefBrowserProcessHandler, public CefRenderProcessHandler {
 public:
  App();

  void OnBeforeCommandLineProcessing(
      const CefString& process_type,
      CefRefPtr<CefCommandLine> command_line) override {
    command_line->AppendSwitch("--disable-web-security");
    //command_line->AppendSwitch("--single-process");
    command_line->AppendSwitch("--disable-sync");
    command_line->AppendSwitch("--ignore-gpu-blacklist");
    command_line->AppendSwitch("--start-fullscreen"); // FIXME: Not working
  }

  CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
    return this;
  }
    // CefApp methods:
  CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override {
    return this;
  }

  void OnContextCreated(CefRefPtr<CefBrowser> browser,
                        CefRefPtr<CefFrame> frame,
                        CefRefPtr<CefV8Context> context) override {
    message_router_->OnContextCreated(browser, frame, context);
  }

  void OnContextReleased(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame,
                         CefRefPtr<CefV8Context> context) override {
    message_router_->OnContextReleased(browser, frame, context);
  }

  bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                CefProcessId source_process,
                                CefRefPtr<CefProcessMessage> message) override {
    return message_router_->OnProcessMessageReceived(browser, frame,
                                                     source_process, message);
  }

   // CefRenderProcessHandler methods:
  void OnWebKitInitialized() override {
    // Create the renderer-side router for query handling.
    CefMessageRouterConfig config;
    message_router_ = CefMessageRouterRendererSide::Create(config);
  }

  void OnRegisterCustomSchemes(CefRawPtr<CefSchemeRegistrar> registrar) override {
    registrar->AddCustomScheme(kScheme, kSchemeRegistrationOptions);
  }

  void OnContextInitialized() override;
  CefRefPtr<CefClient> GetDefaultClient() override;

 private:
    // Handles the renderer side of query routing.
  CefRefPtr<CefMessageRouterRendererSide> message_router_;

  IMPLEMENT_REFCOUNTING(App);
};

#endif

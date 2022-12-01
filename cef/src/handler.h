// SPDX-License-Identifier: GPL-2.0+

// Copyright © 2022 Collabora Ltd
// Copyright © 2022 Valve Corporation

#ifndef HANDLER_H_
#define HANDLER_H_

#include "include/cef_client.h"
#include "include/wrapper/cef_message_router.h"

#include <list>

class Handler : public CefClient,
                public CefDisplayHandler,
                public CefLifeSpanHandler,
                public CefRequestHandler,
                public CefLoadHandler {
 public:
  explicit Handler(bool use_views);
  ~Handler();

  static Handler* GetInstance();

  // CefClient methods:
  virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() override {
    return this;
  }
  virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override {
    return this;
  }
  virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }

  // CefDisplayHandler methods:
  virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
                             const CefString& title) override;
  virtual void OnFullscreenModeChange(CefRefPtr<CefBrowser> browser,
                                      bool fullscreen) override;

  bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                              CefRefPtr<CefFrame> frame,
                              CefProcessId source_process,
                              CefRefPtr<CefProcessMessage> message) override;
  // CefLifeSpanHandler methods:
  virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
  virtual bool DoClose(CefRefPtr<CefBrowser> browser) override;
  virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

  // CefRequestHandler methods:
  bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
                      CefRefPtr<CefFrame> frame,
                      CefRefPtr<CefRequest> request,
                      bool user_gesture,
                      bool is_redirect) override;

  // CefLoadHandler methods:
  virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           ErrorCode errorCode,
                           const CefString& errorText,
                           const CefString& failedUrl) override;
  void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
                                 TerminationStatus status) override;

  void CloseAllBrowsers(bool force_close);

  bool IsClosing() const { return is_closing_; }

  static bool IsChromeRuntimeEnabled();

 private:

  const bool use_views_;
   // Handles the browser side of query routing.
  CefRefPtr<CefMessageRouterBrowserSide> message_router_;
  std::unique_ptr<CefMessageRouterBrowserSide::Handler> message_handler_;

  typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
  BrowserList browser_list_;

  bool is_closing_;

  IMPLEMENT_REFCOUNTING(Handler);
};

#endif

// SPDX-License-Identifier: GPL-2.0+

// Copyright © 2022 Collabora Ltd
// Copyright © 2022 Valve Corporation

#include "app.h"

#include <string>
#include <unistd.h>
#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"
#include "handler.h"

namespace {

std::string GetStartupURL() {
  std::stringstream ss;
  ss << kScheme << "://" << kDomain << "/" << kFileName;
  return ss.str();
}

class WindowDelegate : public CefWindowDelegate {
 public:
  explicit WindowDelegate(CefRefPtr<CefBrowserView> browser_view)
      : browser_view_(browser_view) {
    CefRefPtr<CefCommandLine> command_line =
        CefCommandLine::GetGlobalCommandLine();
    maximized_mode = command_line->HasSwitch("maximized");
    fullscreen_mode = command_line->HasSwitch("fullscreen");
  }

  void OnWindowCreated(CefRefPtr<CefWindow> window) override {
    window->AddChildView(browser_view_);
    const cef_color_t color = CefColorSetARGB(0, 0, 0, 0);
    window->SetBackgroundColor(color);
    window->Show();

    browser_view_->RequestFocus();
  }

  CefRect GetInitialBounds(CefRefPtr<CefWindow> window) override {
    if (fullscreen_mode)
      return CefDisplay::GetPrimaryDisplay()->GetBounds();
    return CefRect(0, 0, 800, 600);
  }

  void OnWindowDestroyed(CefRefPtr<CefWindow> window) override {
    browser_view_ = nullptr;
  }

  bool CanResize(CefRefPtr<CefWindow> window) override {
    return true;
  }

  bool CanClose(CefRefPtr<CefWindow> window) override {
    if (fullscreen_mode)
      return false;
    return true;
  }

  bool CanMinimize(CefRefPtr<CefWindow> window) override {
    if (fullscreen_mode)
      return false;
    return true;
  }

  bool CanMaximize(CefRefPtr<CefWindow> window) override {
    if (fullscreen_mode)
      return false;
    return true;
  }

  cef_show_state_t GetInitialShowState(CefRefPtr<CefWindow> window) override {
    if (maximized_mode) return CEF_SHOW_STATE_MAXIMIZED;
    if (fullscreen_mode) return CEF_SHOW_STATE_FULLSCREEN;

    return CEF_SHOW_STATE_NORMAL;
  }

  bool IsFrameless(CefRefPtr<CefWindow> window) override {
    if (fullscreen_mode)
      return true;
    return false;
  }

 private:
  CefRefPtr<CefBrowserView> browser_view_;
  bool fullscreen_mode = false;
  bool maximized_mode = false;

  IMPLEMENT_REFCOUNTING(WindowDelegate);
  DISALLOW_COPY_AND_ASSIGN(WindowDelegate);
};

class BrowserViewDelegate : public CefBrowserViewDelegate {
 public:
  BrowserViewDelegate() {}

  bool OnPopupBrowserViewCreated(CefRefPtr<CefBrowserView> browser_view,
                                 CefRefPtr<CefBrowserView> popup_browser_view,
                                 bool is_devtools) override {
    return false;
  }

 private:
  IMPLEMENT_REFCOUNTING(BrowserViewDelegate);
  DISALLOW_COPY_AND_ASSIGN(BrowserViewDelegate);
};

}  // namespace

App::App() {}

void App::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();

  RegisterSchemeHandlerFactory();

  CefRefPtr<CefCommandLine> command_line =
      CefCommandLine::GetGlobalCommandLine();

  // Run in views mode: has support for enter/exit full screen mode
  // Also views mode are supported to run on both x11 and wayland.
  const bool use_views = true; //command_line->HasSwitch("use-views");

  CefRefPtr<Handler> handler(new Handler(use_views));

  CefBrowserSettings browser_settings;
  browser_settings.javascript_close_windows = STATE_ENABLED;
  browser_settings.plugins = STATE_DISABLED;
  browser_settings.text_area_resize = STATE_DISABLED;
  browser_settings.background_color = CefColorSetARGB(0, 0, 0, 0);

  std::string url = GetStartupURL();

  if (use_views) {
    CefRefPtr<CefBrowserView> browser_view = CefBrowserView::CreateBrowserView(
        handler, url, browser_settings, nullptr, nullptr,
        new BrowserViewDelegate());

    CefWindow::CreateTopLevelWindow(new WindowDelegate(browser_view));
  } else {
    CefWindowInfo window_info;
    CefString(&window_info.window_name) = "SteamOS Factory Reset";

    CefBrowserHost::CreateBrowser(window_info, handler, url, browser_settings,
                                  nullptr, nullptr);
  }
}

CefRefPtr<CefClient> App::GetDefaultClient() {
  return Handler::GetInstance();
}

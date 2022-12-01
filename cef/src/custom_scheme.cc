// SPDX-License-Identifier: GPL-2.0+

// Copyright © 2022 Collabora Ltd
// Copyright © 2022 Valve Corporation

#include <cmrc/cmrc.hpp>

#include "include/cef_browser.h"
#include "include/cef_callback.h"
#include "include/cef_frame.h"
#include "include/cef_request.h"
#include "include/cef_resource_handler.h"
#include "include/cef_response.h"
#include "include/cef_scheme.h"
#include "include/wrapper/cef_helpers.h"
#include "custom_scheme.h"


const char kScheme[] = "client";
const char kDomain[] = "web";
const char kFileName[] = "index.html";
const int kSchemeRegistrationOptions =
    CEF_SCHEME_OPTION_STANDARD | CEF_SCHEME_OPTION_SECURE |
    CEF_SCHEME_OPTION_CORS_ENABLED | CEF_SCHEME_OPTION_FETCH_ENABLED;


CMRC_DECLARE(app);
namespace {

// Implementation of the scheme handler for client:// requests.
class ClientSchemeHandler : public CefResourceHandler {
 public:
  ClientSchemeHandler() : offset_(0) {}

  bool Open(CefRefPtr<CefRequest> request,
                    bool& handled,
                    CefRefPtr<CefCallback> callback) override {
    CEF_REQUIRE_IO_THREAD();
    auto fs = cmrc::app::get_filesystem();

    std::string url = request->GetURL();
    std::string host = std::string(kScheme) + "://" + kDomain + "/";
    std::string filename = url.erase(0, host.length());

    std::string resource = std::string("web/") + filename;
    auto fd1 = fs.open(resource);
    data_ = std::string(fd1.begin(), fd1.end());
    mime_type_ = GetMimeTypeFromFileName(filename);

    if (!data_.empty() && !mime_type_.empty()) {
      handled = true;
    }

    if (handled) {
      callback->Continue();
      return true;
    }

    return false;
  }

  void GetResponseHeaders(CefRefPtr<CefResponse> response,
                          int64& response_length,
                          CefString& redirectUrl) override {
    CEF_REQUIRE_IO_THREAD();

    DCHECK(!data_.empty());

    response->SetMimeType(mime_type_);
    response->SetStatus(200);

    // Set the resulting response length.
    response_length = data_.length();
  }

  void Cancel() override { CEF_REQUIRE_IO_THREAD(); }

  bool Read(void* data_out,
            int bytes_to_read,
            int& bytes_read,
            CefRefPtr<CefResourceReadCallback> callback) override {
    CEF_REQUIRE_IO_THREAD();

    bool has_data = false;
    bytes_read = 0;

    if (offset_ < data_.length()) {
      // Copy the next block of data into the buffer.
      int transfer_size =
          std::min(bytes_to_read, static_cast<int>(data_.length() - offset_));
      memcpy(data_out, data_.c_str() + offset_, transfer_size);
      offset_ += transfer_size;

      bytes_read = transfer_size;
      has_data = true;
    }

    return has_data;
  }

 private:
  std::string data_;
  std::string mime_type_;
  size_t offset_;

  std::map<std::string, std::string> mimeInfo = {
      { "gif", "image/gif",},
      {"jpeg", "image/jpeg"},
      {"jpg", "image/jpeg"},
      {"png", "image/png"},
      {"webp", "image/webp"},
      {"mhtml", "multipart/related"},
      {"mht", "multipart/related"},
      {"css", "text/css"},
      {"html", "text/html"},
      {"htm", "text/html"},
      {"mjs", "text/javascript"},
      {"js", "text/javascript"},
      {"xml", "text/xml"}
  };

  std::string GetMimeTypeFromFileName(std::string& filename) {
    size_t dot_pos = filename.find_last_of(".");
    if (dot_pos != std::string::npos) {
      std::string filetype = std::string(filename, dot_pos + 1);
      if (mimeInfo.find(filetype.c_str()) != mimeInfo.end()){
        return mimeInfo[filetype.c_str()];
      }
    }
    return "";
  }

  IMPLEMENT_REFCOUNTING(ClientSchemeHandler);
  DISALLOW_COPY_AND_ASSIGN(ClientSchemeHandler);
};

// Implementation of the factory for creating scheme handlers.
class ClientSchemeHandlerFactory : public CefSchemeHandlerFactory {
 public:
  ClientSchemeHandlerFactory() {}

  // Return a new scheme handler instance to handle the request.
  CefRefPtr<CefResourceHandler> Create(CefRefPtr<CefBrowser> browser,
                                       CefRefPtr<CefFrame> frame,
                                       const CefString& scheme_name,
                                       CefRefPtr<CefRequest> request) override {
    CEF_REQUIRE_IO_THREAD();
    return new ClientSchemeHandler();
  }

 private:
  IMPLEMENT_REFCOUNTING(ClientSchemeHandlerFactory);
  DISALLOW_COPY_AND_ASSIGN(ClientSchemeHandlerFactory);
};

}  // namespace

void RegisterSchemeHandlerFactory() {
  CefRegisterSchemeHandlerFactory(kScheme, kDomain,
                                  new ClientSchemeHandlerFactory());
}

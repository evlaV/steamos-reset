# SPDX-License-Identifier: GPL-2.0+

# Copyright © 2022 Collabora Ltd
# Copyright © 2022 Valve Corporation
CEF_VERSION := 98.1.19+g57be9e2+chromium-98.0.4758.80
CEF_PLATFORM:= linux64_minimal
CEF_DIST    := cef_binary_$(CEF_VERSION)_$(CEF_PLATFORM)
CEF_DLFROM  := https://cef-builds.spotifycdn.com
CEF_3PARTY  := cef/third_party/cef/$(CEF_DIST)
CEF_ARCHIVE := $(CEF_3PARTY).tar.bz2
CEF_BUILD   := cef/CMakeLists.txt $(wilcard cef/cmake/*.cmake)
CEF_BIN     := cef/build/Release/bin/steamos-reset$(EXEEXT)
CEF_LIBDIR  := cef/build/Release/lib/steamos-reset
CEF_LOCALES  = $(wildcard $(CEF_LIBDIR)/locales/*.pak)
CEF_SHADER  := $(CEF_LIBDIR)/swiftshader/libEGL.so \
               $(CEF_LIBDIR)/swiftshader/libGLESv2.so
CEF_LIBINST := $(pkglibdir)/lib/steamos-reset
CEF_LIBDATA  = \
  $(wildcard $(CEF_LIBDIR)/*.dat)  \
  $(wildcard $(CEF_LIBDIR)/*.pak)  \
  $(wildcard $(CEF_LIBDIR)/*.bin)  \
  $(wildcard $(CEF_LIBDIR)/*.json)
CEF_LIB      = \
  $(wildcard $(CEF_LIBDIR)/*.so)   \
  $(wildcard $(CEF_LIBDIR)/*.so.*) \
  $(CEF_LIBDIR)/chrome-sandbox
VARIANTS += cef

$(CEF_ARCHIVE).sha1 $(CEF_ARCHIVE):
	mkdir -p $(@D)
	curl "$(CEF_DLFROM)/$(subst +,%2B,$(@F))" -o "$@"

%.sha1.ok: % %.sha1
	(S1=$$(cat $<.sha1); \
	 read S2 X < <(sha1sum $<); \
	 echo "$$S1"; \
	 if [ "$$S1" = "$$S2" ]; then touch $@; fi)

SR_WEBSRC := \
  cef/web/index.html \
  cef/web/style.css \
  cef/web/main.js \
  cef/web/backend.js

SR_CCSRC := \
  cef/src/app.cc \
  cef/src/app.h \
  cef/src/custom_scheme.cc \
  cef/src/custom_scheme.h \
  cef/src/handler.cc \
  cef/src/handler.h \
  cef/src/main.cc

$(CEF_3PARTY)/.stamp: $(CEF_ARCHIVE).sha1.ok
	@echo $@ :: "[$^]"
	@(mkdir -p "$(@D)" && \
	  tar -C "$(@D)" --strip-components=1 -xavf "$(subst .sha1.ok,,$<)" && \
	  touch "$@")

STEAMOS_RESET_SRC = $(SR_CCSRC) $(SR_WEBSRC)

$(STEAMOS_RESET_SRC) $(CEF_BUILD) $(CEF_ARCHIVE) $(CEF_ARCHIVE).sha1:

cef/build/Release/bin/steamos-reset$(EXEEXT): $(CEF_BUILD)
cef/build/Release/bin/steamos-reset$(EXEEXT): $(CEF_3PARTY)/.stamp
cef/build/Release/bin/steamos-reset$(EXEEXT): $(CEF_ARCHIVE)
cef/build/Release/bin/steamos-reset$(EXEEXT): $(STEAMOS_RESET_SRC)
	@mkdir -p cef/build
	@cd cef/build/ && cmake ../ && make

distclean-cef:
	rm -rvf cef/build
	rm -rvf $(CEF_3PARTY)

clean-cef:
	rm -rvf $(CEF_LIB) $(CEF_SHADER) $(CEF_LOCALES) $(CEF_BIN)
	@cd cef/build && [ -f Makefile ] && make clean

install-cef:
	install -d $(DESTDIR)$(pkglibdir)
	install -s -m 0755 -T \
	    $(CEF_BIN) $(DESTDIR)$(pkglibdir)/bin/steamos-reset-cef
	install -d $(DESTDIR)$(CEF_LIBINST)
	install -d $(DESTDIR)$(CEF_LIBINST)/locales
	install -d $(DESTDIR)$(CEF_LIBINST)/swiftshader
	install -s -m 0755 -t $(DESTDIR)$(CEF_LIBINST) $(CEF_LIB)
	install -m 0644 -t $(DESTDIR)$(CEF_LIBINST) $(CEF_LIBDATA)
	install -m 0644 -t $(DESTDIR)$(CEF_LIBINST)/locales $(CEF_LOCALES)
	install -s -m 0755 -t $(DESTDIR)$(CEF_LIBINST)/swiftshader $(CEF_SHADER)

uninstall-cef:
	rm -vf $(DESTDIR)$(pkglibdir)/bin/steamos-reset-cef
	rm -vf $(DESTDIR)$(CEF_LIBINST)/locales/*.pak
	rm -vf $(DESTDIR)$(CEF_LIBINST)/swiftshader/libEGL.so
	rm -vf $(DESTDIR)$(CEF_LIBINST)/swiftshader/libGLESv2.so
	(cd $(DESTDIR)$(CEF_LIBINST) && \
	 rm -vf *.dat *.pak *.so *.so.* *.bin *.json chrome-sandbox)

clean-local: clean-cef
distclean-local: distclean-cef
install-exec-hook: install-cef
uninstall-hook: uninstall-cef

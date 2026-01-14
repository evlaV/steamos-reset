# SPDX-License-Identifier: GPL-2.0+

# Copyright © 2022-2023,2026 Collabora Ltd
# Copyright © 2022-2023,2026 Valve Corporation

PKGSHAREDIR:= ${prefix}/share/$(PACKAGE_NAME)
# where the wrapper hardlinks live - would be under var,
# but var is special on SteamOS:
CGI_BINDIR := $(PKGSHAREDIR)/cgi
# systemd service file location
UNITDIR    := $(libdir)/systemd/system
# lighttpd config
LIGHTTPDIR := $(PKGSHAREDIR)/lighttpd

VARIANTS   := 
BACKENDS   := debug status os-status boot-status factory-reset undo-reset clear
SH_LIBS    := steamos-reset.sh
WRAPPER    := steamos-reset-wrapper
SRVCTL     := steamos-reset-service
CLI        := steamos-reset-tool
LIGHTTPD   := lighttpd/steamos-reset.conf
SERVICE    := lighttpd/steamos-reset.service
LIGHTTPCFG := $(LIGHTTPDIR)/$(notdir $(LIGHTTPD))
BUILT      := $(WRAPPER) steamos-reset.sh $(LIGHTTPD) holo-reset $(SRVCTL)
DEFS        = -DVERSION=\"$(PACKAGE_VERSION)\"

# where the actual scripts invoked by the wrapper live:
DEFS       += -DCGI_LIBDIR=\"${pkglibexecdir}\"
DEFS       += -DWRAPPER=\"$(WRAPPER)\"

CFLAGS      = -Wall -Werror -std=gnu99 $(DEFS)
CLEANFILES  = $(BUILT)

noinst_PROGRAMS    = $(WRAPPER) $(SRVCTL)
pkglibexec_SCRIPTS = $(BACKENDS) $(SH_LIBS)
bin_SCRIPTS = $(ifneq ,$(DESKTOPS),holo-reset)

 ifneq (,$(filter qml all,$(UI)))
 include qml.mk
 endif

 ifneq (,$(filter cef all,$(UI)))
 include cef.mk
 endif

DESKTOPS = $(foreach v,$(VARIANTS),steamos-factory-reset.$(v).desktop)
CLEANFILES += $(DESKTOPS)

.PHONY: install clean debug-build

all: $(BUILT) $(CEF_BIN) $(QML_BIN) $(DESKTOPS)
binaries: $(BUILT) $(CEF_BIN) $(QML_BIN) $(DESKTOPS)

debug-build:
	@echo built: $(BUILT) $(CEF_BIN) $(QML_BIN)
	@echo ui: $(VARIANTS)
	@echo .desktop: $(DESKTOPS)

steamos_reset_wrapper_SOURCES = wrapper.c
steamos_reset_service_SOURCES = srvctl.c

steamos-factory-reset.%.desktop: data/steamos-factory-reset.desktop
	@sed -e 's/@VARIANT@/\U$(*)/g; s/@variant@/$(*)/g' < $^ > $@

$(BACKENDS):

install-data-hook: all
	install -d $(DESTDIR)$(LIGHTTPDIR)
	install -m 0644 -T $(LIGHTTPD) $(DESTDIR)$(LIGHTTPCFG)
	install -d $(DESTDIR)$(UNITDIR)
	install -m 0644 -T $(SERVICE) $(DESTDIR)$(UNITDIR)/$(notdir $(SERVICE))
	install -d $(DESTDIR)$(PKGSHAREDIR)
	install -m 0644 -t $(DESTDIR)$(PKGSHAREDIR) data/steamos-reset.svg
	install -d $(DESTDIR)$(PKGDATADIR)/applications
	if [ -n "$(DESKTOPS)" ]; then \
	  install -m 0644 -t $(DESTDIR)$(PKGDATADIR)/applications $(DESKTOPS); \
	fi

install-exec-hook: all
	install -d $(DESTDIR)$(CGI_BINDIR)
	install -s -m 4755 -t $(DESTDIR)$(CGI_BINDIR) $(WRAPPER)
	install -d $(DESTDIR)$(sbindir)
	install -s -m 4755 -t $(DESTDIR)$(sbindir) $(SRVCTL)
	install -m 0755 -t $(DESTDIR)$(sbindir) $(CLI)
	(cd $(DESTDIR)$(CGI_BINDIR); for s in $(BACKENDS); \
	 do ln -f $(WRAPPER) $$s; done)

uninstall-hook:
	rm -vf $(DESTDIR)$(LIGHTTPCFG)
	rm -vf $(DESTDIR)$(UNITDIR)/$(notdir $(SERVICE))
	rm -vf $(DESTDIR)$(CGI_BINDIR)/$(notdir $(WRAPPER))
	(for d in $(DESKTOPS); \
	 do rm -vf $(DESTDIR)$(PKGDATADIR)/applications/$$d; done)
	rm -vf $(DESTDIR)$(PKGSHAREDIR)/steamos-reset.svg
	rm -vf $(DESTDIR)$(sbindir)/$(notdir $(SRVCTL))
	rm -vf $(DESTDIR)$(sbindir)/$(notdir $(CLI))
	(for s in $(BACKENDS); do rm -vf $(DESTDIR)$(CGI_BINDIR)/$$s; done)

clean-local:
distclean-local:

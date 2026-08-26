# SPDX-License-Identifier: GPL-2.0+

# Copyright © 2022-2023,2026 Collabora Ltd
# Copyright © 2022-2023,2026 Valve Corporation

SHELL      := /bin/bash
PKGSHAREDIR:= ${prefix}/share/$(PACKAGE_NAME)
# where the wrapper hardlinks live - would be under var,
# but var is special on Holo:
CGI_BINDIR := $(PKGSHAREDIR)/cgi
# systemd service file location
UNITDIR    := $(libdir)/systemd/system
# lighttpd config
LIGHTTPDIR := $(PKGSHAREDIR)/lighttpd

BACKENDS   := debug status os-status boot-status factory-reset undo-reset clear
SH_LIBS    := holo-reset.sh
WRAPPER    := holo-reset-wrapper
SRVCTL     := holo-reset-service
CLI        := holo-reset-tool
LIGHTTPD   := lighttpd/holo-reset.conf
SERVICE    := lighttpd/holo-reset.service
LIGHTTPCFG := $(LIGHTTPDIR)/$(notdir $(LIGHTTPD))
BUILT      := $(WRAPPER) holo-reset.sh $(LIGHTTPD) holo-reset $(SRVCTL)
DEFS        = -DVERSION=\"$(PACKAGE_VERSION)\"

# where the actual scripts invoked by the wrapper live:
DEFS       += -DCGI_LIBDIR=\"${pkglibexecdir}\"
DEFS       += -DWRAPPER=\"$(WRAPPER)\"

CFLAGS      = -Wall -Werror -std=gnu99 $(DEFS)
CLEANFILES  = $(BUILT)

noinst_PROGRAMS    = $(WRAPPER) $(SRVCTL)
pkglibexec_SCRIPTS = $(BACKENDS) $(SH_LIBS)
bin_SCRIPTS = holo-reset

.PHONY: install clean debug-build

all: $(BUILT)
binaries: $(BUILT)

debug-build:
	@echo built: $(BUILT)

holo_reset_wrapper_SOURCES = wrapper.c
holo_reset_service_SOURCES = srvctl.c

$(BACKENDS):

install-data-hook: all
	install -d $(DESTDIR)$(LIGHTTPDIR)
	install -m 0644 -T $(LIGHTTPD) $(DESTDIR)$(LIGHTTPCFG)
	install -d $(DESTDIR)$(UNITDIR)
	install -m 0644 -T $(SERVICE) $(DESTDIR)$(UNITDIR)/$(notdir $(SERVICE))

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
	rm -vf $(DESTDIR)$(sbindir)/$(notdir $(SRVCTL))
	rm -vf $(DESTDIR)$(sbindir)/$(notdir $(CLI))
	(for s in $(BACKENDS); do rm -vf $(DESTDIR)$(CGI_BINDIR)/$$s; done)

clean-local:
distclean-local:

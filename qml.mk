# SPDX-License-Identifier: GPL-2.0+

# Copyright © 2022 Collabora Ltd
# Copyright © 2022 Valve Corporation
QML_BIN := qml/steamos-reset
QML_SRC := qml/steamos-reset.cpp \
           qml/steamos-reset.qml \
           qml/steamos-reset.pro \
           qml/qtquickcontrols2.conf \
           qml/js/reset.js   \
           qml/js/backend.js \
           qml/icons/activate.png \
           qml/icons/spinner.png \
           data/steamos-reset.svg
QML_ICON := qml/icons/steamos-reset.svg
VARIANTS += qml


qml/icons/%.svg: data/%.svg
	@cp -v $^ $@

$(QML_BIN): $(QML_SRC) $(QML_ICON)
	@cd qml && qmake -makefile -o Makefile && make

clean-qml:
	@if [ -f qml/Makefile ]; then cd qml && make clean; fi
	@rm -vf qml/Makefile
	@rm -vf $(QML_ICON)
	@rm -vf $(QML_BIN)

distclean-qml:
	@rm -rvf qml/Makefile qml/*.qrc qml/*.o qml/*.stash $(QML_BIN)

install-qml:
	install -d $(DESTDIR)$(pkglibdir)/bin
	install -s -m 0755 -T \
	    $(QML_BIN) $(DESTDIR)$(pkglibdir)/bin/steamos-reset-qml

uninstall-qml:
	@rm -vf $(DESTDIR)$(pkglibdir)/bin/steamos-reset-qml

clean-local: clean-qml
distclean-local: distclean-qml
install-exec-hook: install-qml
uninstall-hook: uninstall-qml

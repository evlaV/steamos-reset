SOURCES   += holo-reset.cpp
CONFIG    += release
QT        += quick qml gui

resources.prefix = /
resources.files  = \
    holo-reset.qml       \
    qtquickcontrols2.conf   \
    icons/spinner.png       \
    icons/activate.png      \
    icons/steamos-reset.svg \
    js/reset.js             \
    js/backend.js
RESOURCES = resources

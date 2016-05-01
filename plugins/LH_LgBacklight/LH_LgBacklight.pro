TARGET = LH_LgBacklight
TEMPLATE = lib
CONFIG += lh_hid
DEFINES += LGBACKLIGHT_LIBRARY

include(../Plugins.pri)

SOURCES += \
    $$PLUGIN_SOURCES \
    LH_LgBacklight.cpp \
    LgBacklightDevice.cpp

HEADERS += \
    $$PLUGIN_HEADERS \
    LH_LgBacklight.h\
    LgBacklightDevice.h

RESOURCES += \
    resources.qrc

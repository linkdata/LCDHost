TARGET = LH_Lg160x43
TEMPLATE = lib
DEFINES += LG160X43_LIBRARY
CONFIG += lh_hid
CONFIG += device wow64

include(../Plugins.pri)

SOURCES += \
    $$PLUGIN_SOURCES \
    LH_Lg160x43.cpp \
    Lg160x43Device.cpp

HEADERS += \
    $$PLUGIN_HEADERS \
    LH_Lg160x43.h \
    Lg160x43Device.h

win32: LIBS += -luser32

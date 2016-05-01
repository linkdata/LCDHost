TARGET = LH_Lg320x240
TEMPLATE = lib
DEFINES += LG320x240_LIBRARY
CONFIG += lh_plugin lh_api5plugin lh_libusbx

SOURCES += \
    LH_Lg320x240.cpp \
    LogitechG19.cpp

HEADERS += \
    LH_Lg320x240.h \
    LogitechG19.h

DESTDIR=$$lh_destdir()

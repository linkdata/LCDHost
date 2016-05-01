TARGET = lh_hid
TEMPLATE = lib
CONFIG += shared lh_shared
QT = core
DEFINES += LH_HID_LIBRARY

HIDAPI=$$PWD/hidapi
QMAKE_CFLAGS_WARN_ON = -w
INCLUDEPATH += $$HIDAPI

LH_SHARED_HEADERS += LH_HidDevice.h

HEADERS += \
    $$HIDAPI/hidapi.h \
    LH_HidWorker.h \
    LH_HidDevice.h \
    LH_HidRequest.h \
    LH_HidPublisher.h \
    LH_HidThread.h

SOURCES += \
    LH_HidWorker.cpp \
    LH_HidDevice.cpp \
    LH_HidRequest.cpp \
    LH_HidPublisher.cpp \
    LH_HidThread.cpp

win32 {
    #!contains(QT_ARCH, x86_64) {
    #    INCLUDEPATH *= C:/WinDDK/7600.16385.1/inc
    #    LIBS *= -LC:/WinDDK/7600.16385.1/lib/win7/i386
    #}
    # INCLUDEPATH *= C:/WinDDK/7600.16385.1/inc/api
    # LIBS *= -LC:/WinDDK/7600.16385.1/lib/win7/i386 -lhid
    # DEFINES += HIDAPI_USE_DDK
    SOURCES += $$HIDAPI/windows/hid.c
    LIBS += -lsetupapi
}

macx {
    SOURCES += $$HIDAPI/mac/hid.c
    LIBS += -framework CoreFoundation
    LIBS += -framework IOKit
}

unix:!macx {
        SOURCES += $$HIDAPI/linux/hid.c
        LIBS += -ludev
}

DESTDIR = $$lh_destdir()

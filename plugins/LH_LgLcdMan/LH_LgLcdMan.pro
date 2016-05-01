TARGET = LH_LgLcdMan
TEMPLATE = lib
DEFINES += LGLCDMAN_LIBRARY
CONFIG += device wow64

include(../Plugins.pri)

SOURCES += \
    $$PLUGIN_SOURCES \
    LH_LgLcdMan.cpp \
    LogitechDevice.cpp \
    LH_LgLcdLegacyThread.cpp \
    LH_LgLcdCallbackThread.cpp \
    LH_LgLcdThread.cpp

HEADERS += \
    $$PLUGIN_HEADERS \
    LH_LgLcdMan.h \
    LogitechDevice.h \
    EventLgLcdNotification.h \
    LH_LgLcdLegacyThread.h \
    EventLgLcdButton.h \
    LH_LgLcdCallbackThread.h \
    LH_LgLcdThread.h

win32 {
    contains(QT_ARCH, x86_64): LIBS += $$PWD/win/x64/lglcd.lib
    else: LIBS += $$PWD/win/x86/lglcd.lib
    LIBS += -ladvapi32
}
macx:LIBS += $$PWD/mac/liblgLcd.a -framework CoreFoundation

RESOURCES += LgLcdManResources.qrc

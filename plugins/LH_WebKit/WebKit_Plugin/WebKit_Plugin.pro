TARGET = LH_WebKit
TEMPLATE = lib
QT += network \
    xml
DEFINES += LH_WEBKIT_LIBRARY
CONFIG += cf

include(../../Plugins.pri)

INCLUDEPATH *= $$dirname(PWD)
LIBS *= -lLH_Text

SOURCES += \
    ../WebKitCommand.cpp \
    LH_QtPlugin_WebKit.cpp \
    LH_WebKit.cpp \
    LH_WebKitHTML.cpp \
    LH_WebKitURL.cpp \
    LH_WebKitFile.cpp \
    LH_RSSText.cpp \
    LH_RSSFeed.cpp \
    LH_RSSInterface.cpp \
    LH_RSSBody.cpp

HEADERS += \
    ../WebKitCommand.h \
    LH_QtPlugin_WebKit.h \
    LH_WebKit.h \
    LH_WebKitHTML.h \
    LH_WebKitURL.h \
    LH_WebKitFile.h \
    LH_RSSText.h \
    LH_RSSFeed.h \
    LH_RSSInterface.h \
    LH_RSSBody.h

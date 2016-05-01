TARGET = WebKitServer
TEMPLATE = app
QT += network webengine
CONFIG -= app_bundle
INCLUDEPATH *= $$dirname(PWD)
# CONFIG += lh_logger
# LIBS *= -llh_logger

include(../../Plugins.pri)

SOURCES += \
    ../WebKitCommand.cpp \
    WebKitServerMain.cpp \
    WebKitRequest.cpp \
    WebKitServerWindow.cpp

HEADERS += \
    ../WebKitCommand.h \
    WebKitRequest.h \
    WebKitServerWindow.h \
    EventWebKitHeartbeat.h

FORMS += WebKitServerWindow.ui

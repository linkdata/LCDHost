TARGET = LH_Cursor
TEMPLATE = lib
QT += network \
    xml
DEFINES += LH_CURSOR_LIBRARY
DEFINES += VERSION=2.10
CONFIG += cf json

include(../Plugins.pri)

LIBS *= -lLH_Decor -lLH_Image

HEADERS += \
    actionType.h \
    LH_QtPlugin_Cursor.h \
    LH_CursorData.h \
    LH_CursorController.h \
    LH_CursorRectangle.h \
    LH_CursorPage.h \
    LH_CursorImage.h \
    LH_QuickAction.h \
    LH_CursorReceiver.h \
    LH_CursorAction.h

SOURCES += \
    actionType.cpp \
    LH_QtPlugin_Cursor.cpp \
    LH_CursorData.cpp \
    LH_CursorController.cpp \
    LH_CursorRectangle.cpp \
    LH_CursorPage.cpp \
    LH_CursorImage.cpp \
    LH_QuickAction.cpp \
    LH_CursorReceiver.cpp \
    LH_CursorAction.cpp



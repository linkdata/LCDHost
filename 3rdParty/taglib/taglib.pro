TEMPLATE = lib
QT -= gui
DEFINES += BUILD_TAGLIB

include(taglib.pri)

CONFIG(debug, debug|release) {
    DEFINES += _DEBUG
    DESTDIR = $$PWD/taglib-debug
    OBJECTS_DIR = ./debug-o
} else {
    DESTDIR = $$PWD/taglib-release
    OBJECTS_DIR = ./release-o
}

win32 {
    CONFIG += dll
}

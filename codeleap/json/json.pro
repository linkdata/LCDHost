TEMPLATE = lib
CONFIG += shared lh_shared
TARGET = lh_json
DEFINES += JSON LH_JSON_LIBRARY
SOURCES += \
    json.cpp

HEADERS += \
    json.h

DESTDIR=$$lh_destdir()

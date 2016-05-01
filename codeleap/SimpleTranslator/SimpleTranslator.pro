TEMPLATE = lib
CONFIG += shared lh_shared lh_api5plugin
TARGET = lh_simpletranslator
QT += network
DEFINES += SIMPLETRANSLATOR LH_SIMPLETRANSLATOR_LIBRARY
SOURCES += SimpleTranslator.cpp
HEADERS += SimpleTranslator.h
DESTDIR=$$lh_destdir()

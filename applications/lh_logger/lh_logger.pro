TEMPLATE = lib
TARGET = lh_logger
CONFIG += staticlib

DEFINES += LH_LOGGER_LIBRARY
SOURCES += LH_Logger.cpp
HEADERS += LH_Logger.h

# LH_SHARED_HEADERS += LH_Logger.h
# system($${QMAKE_COPY} LH_Logger.h \"$${LH_DIR_SOURCES}/include\")
# DESTDIR = $$lh_destdir()

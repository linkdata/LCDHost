TARGET = LH_Image
TEMPLATE = lib
CONFIG += lh_cf
DEFINES += LH_IMAGE_LIBRARY

include(../Plugins.pri)

SOURCES += \
    LH_Image.c \
    LH_QImage.cpp

HEADERS += \
    LH_Image.h \
    LH_QImage.h

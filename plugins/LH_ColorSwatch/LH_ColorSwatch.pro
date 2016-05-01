TARGET = LH_ColorSwatch
TEMPLATE = lib
DEFINES += VERSION=1.00
DEFINES += LH_COLORSWATCH_LIBRARY
CONFIG += systemstats cf

include(../Plugins.pri)

LIBS *= -lLH_Decor

SOURCES += \
    LH_QtPlugin_ColorSwatch.cpp \
    LH_ColorSwatch.cpp \
    LH_ColorSwatchCPUAverage.cpp \
    LH_ColorSwatchMemPhysical.cpp \
    LH_ColorSwatchMemVirtual.cpp \
    LH_ColorSwatchNetIn.cpp \
    LH_ColorSwatchNetOut.cpp

HEADERS += \
    LH_QtPlugin_ColorSwatch.h \
    LH_ColorSwatch.h

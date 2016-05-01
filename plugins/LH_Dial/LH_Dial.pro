TARGET = LH_Dial
TEMPLATE = lib
CONFIG += systemstats
DEFINES += LH_DIAL_LIBRARY
DEFINES += VERSION=1.05

include(../Plugins.pri)

SOURCES += \
    LH_QtPlugin_Dial.cpp \
    LH_Dial.cpp \
    LH_DialCPUAverage.cpp \
    LH_DialCPUHistogram.cpp \
    LH_DialNetOut.cpp \
    LH_DialNetIn.cpp \
    LH_DialMemVirtual.cpp \
    LH_DialMemPhysical.cpp \
    LH_DialTime.cpp \
    LH_DialTest.cpp \
    LH_PieCPUAverage.cpp \
    LH_PieCPUHistogram.cpp \
    LH_PieMem.cpp
	
HEADERS += \
    LH_QtPlugin_Dial.h \
    LH_Dial.h \
    LH_DialTime.h




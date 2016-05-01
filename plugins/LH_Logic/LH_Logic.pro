TARGET = LH_Logic
TEMPLATE = lib
# CONFIG  += qaxcontainer
QT += network xml
DEFINES += LH_LOGIC_LIBRARY
DEFINES += VERSION=1.10

CONFIG += cf

include(../Plugins.pri)

SOURCES += \
    LH_QtPlugin_Logic.cpp \
    LH_LogicBox.cpp \
    LH_Ticker.cpp

HEADERS += \
    LH_QtPlugin_Logic.h \ 
    LH_LogicBox.h \
    LH_Ticker.h




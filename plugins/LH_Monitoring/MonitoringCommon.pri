DEFINES += MONITORING_CLASS
DEFINES += DATA_CACHE_MAX_POINTS=2048

CONFIG += cf json datacollection
win32: LIBS *= -luser32

LIBS *= -lLH_Image -lLH_Text -lLH_Bar -lLH_Graph -lLH_Dial -lLH_Decor -lLH_ColorSwatch

INCLUDEPATH += $$PWD/Sources \
               $$PWD/Objects

MONSOURCES_HEADERS = \
    $$PWD/Sources/LH_MonitoringSources.h \
    $$PWD/Sources/LH_MonitoringSource.h

MONSOURCES_SOURCES = \
    $$PWD/Sources/LH_MonitoringSources.cpp \
    $$PWD/Sources/LH_MonitoringSource.cpp

MONOBJECTS_HEADERS = \
    $$PWD/LH_QtMonitoringPlugin.h \
    $$PWD/Objects/LH_MonitoringObject.h \
    $$PWD/Objects/LH_MonitoringText.h \
    $$PWD/Objects/LH_MonitoringImage.h \
    $$PWD/Objects/LH_MonitoringGraph.h \
    $$PWD/Objects/LH_MonitoringDial.h \
    $$PWD/Objects/LH_MonitoringBar.h \
    $$PWD/Objects/LH_MonitoringColorSwatch.h
    #$$PWD/Objects/LH_MonitoringPie.h \

MONOBJECTS_SOURCES = \
    $$PWD/LH_QtMonitoringPlugin.cpp \
    $$PWD/Objects/LH_MonitoringObject.cpp \
    $$PWD/Objects/LH_MonitoringText.cpp \
    $$PWD/Objects/LH_MonitoringImage.cpp \
    $$PWD/Objects/LH_MonitoringGraph.cpp \
    $$PWD/Objects/LH_MonitoringDial.cpp \
    $$PWD/Objects/LH_MonitoringBar.cpp \
    $$PWD/Objects/LH_MonitoringColorSwatch.cpp
    #$$PWD/Objects/LH_MonitoringPie.cpp \


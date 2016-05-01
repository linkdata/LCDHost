TARGET = LH_DriveStats
TEMPLATE = lib
DEFINES += LH_DRIVESTATS_LIBRARY
DEFINES += "COMMON_OBJECT_NAME=\"Drive Performance\""
DEFINES += "MONITORING_FOLDER=\"System/Drives\""
DEFINES += VERSION=2.00

QT += xml

include($$PWD/../LH_Monitoring/MonitoringCommon.pri)

include(../Plugins.pri)

INCLUDEPATH += \
    $$PWD/../LH_Monitoring

MONTYPES_HEADERS = \
    LH_MonitoringTypes_DriveStats.h

MONSOURCES_HEADERS += \
    #LH_DriveStatsData.h \
    LH_MonitoringSource_DriveStats.h

MONSOURCES_SOURCES += \
    #LH_DriveStatsData.cpp \
    LH_MonitoringSource_DriveStats.cpp

HEADERS += \
    $$PLUGIN_HEADERS \
    LH_QtPlugin_DriveStats.h \
    $$MONOBJECTS_HEADERS \
    $$MONTYPES_HEADERS \
    $$MONSOURCES_HEADERS
    #../LH_Monitoring/Objects/LH_MonitoringPie.h \

SOURCES += \
    $$PLUGIN_SOURCES \
    LH_QtPlugin_DriveStats.cpp \
    $$MONOBJECTS_SOURCES \
    $$MONSOURCES_SOURCES
    #../LH_Monitoring/Objects/LH_MonitoringPie.cpp \











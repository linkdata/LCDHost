TARGET = LH_Monitoring
TEMPLATE = lib
DEFINES += LH_MONITORING_LIBRARY
DEFINES += COMMON_OBJECT_NAME="Monitoring"
DEFINES += "MONITORING_FOLDER=\"3rdParty/System Monitoring\""
DEFINES += VERSION=3.00

QT += xml

include($$PWD/MonitoringCommon.pri)

include(../Plugins.pri)

INCLUDEPATH += $$PWD/Types

MONTYPES_HEADERS = \
    Types/LH_MonitoringTypes_Afterburner.h \
    Types/LH_MonitoringTypes_ATITrayTools.h \
    Types/LH_MonitoringTypes_CoreTemp.h \
    Types/LH_MonitoringTypes_Fraps.h \
    Types/LH_MonitoringTypes_GPUZ.h \
    Types/LH_MonitoringTypes_HWiNFO.h \
    Types/LH_MonitoringTypes_RivaTuner.h \
    Types/LH_MonitoringTypes_SpeedFan.h

MONSOURCES_HEADERS += \
    Sources/LH_MonitoringSource_Afterburner.h \
    Sources/LH_MonitoringSource_Aida64.h \
    Sources/LH_MonitoringSource_ATITrayTools.h \
    Sources/LH_MonitoringSource_CoreTemp.h \
    Sources/LH_MonitoringSource_Fraps.h \
    Sources/LH_MonitoringSource_GPUZ.h \
    Sources/LH_MonitoringSource_HWiNFO.h \
    Sources/LH_MonitoringSource_HWMonitor.h \
    Sources/LH_MonitoringSource_Logitech.h \
    Sources/LH_MonitoringSource_RivaTuner.h \
    Sources/LH_MonitoringSource_SpeedFan.h

MONSOURCES_SOURCES += \
    Sources/LH_MonitoringSource_Afterburner.cpp \
    Sources/LH_MonitoringSource_Aida64.cpp \
    Sources/LH_MonitoringSource_ATITrayTools.cpp \
    Sources/LH_MonitoringSource_CoreTemp.cpp \
    Sources/LH_MonitoringSource_Fraps.cpp \
    Sources/LH_MonitoringSource_GPUZ.cpp \
    Sources/LH_MonitoringSource_HWiNFO.cpp \
    Sources/LH_MonitoringSource_HWMonitor.cpp \
    Sources/LH_MonitoringSource_Logitech.cpp \
    Sources/LH_MonitoringSource_RivaTuner.cpp \
    Sources/LH_MonitoringSource_SpeedFan.cpp

HEADERS += \
    LH_QtPlugin_Monitoring.h \
    $$MONOBJECTS_HEADERS \
    $$MONTYPES_HEADERS \
    $$MONSOURCES_HEADERS \
    LH_AidaWriter.h \
    LH_RivaWriter.h

SOURCES += \
    LH_QtPlugin_Monitoring.cpp \
    $$MONOBJECTS_SOURCES \
    $$MONSOURCES_SOURCES \
    LH_AidaWriter.cpp \
    LH_RivaWriter.cpp

RESOURCES += \
    LH_Monitoring.qrc

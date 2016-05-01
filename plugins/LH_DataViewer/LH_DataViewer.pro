TARGET = LH_DataViewer
TEMPLATE = lib
QT += network \
    xml
DEFINES += LH_DATAVIEWER_LIBRARY
DEFINES += VERSION=1.05
CONFIG += cf

win32:LIBS += -L"C:/Program Files/Microsoft SDKs/Windows/v7.0/Lib" -lversion

LIBS *= -lLH_Text -lLH_Bar -lLH_Dial

include(../Plugins.pri)

SOURCES += \
    LH_QtPlugin_DataViewer.cpp \
    LH_DataViewerConnector.cpp \
    LH_DataViewerText.cpp \
    LH_DataViewerImage.cpp \
    LH_DataViewerBar.cpp \
    LH_DataViewerExpiredImage.cpp \
    LH_DataViewerData.cpp \
    LH_DataViewerDial.cpp \
    LH_DataViewerDataTypes.cpp

HEADERS += \
    LH_QtPlugin_DataViewer.h \
    LH_DataViewerConnector.h \
    LH_DataViewerText.h \
    LH_DataViewerImage.h \
    LH_DataViewerBar.h \
    LH_DataViewerData.h \
    LH_DataViewerExpiredImage.h \
    LH_DataViewerDataTypes.h \
    LH_DataViewerDial.h

TARGET = LH_Weather
TEMPLATE = lib
QT += network \
    xml
DEFINES += LH_WEATHER_LIBRARY
DEFINES += VERSION=1.52
CONFIG += cf translator json

include(../Plugins.pri)

LIBS *= -lLH_Text -lLH_Image

SOURCES += \
    LH_WeatherData.cpp \
    LH_QtPlugin_Weather.cpp \
    LH_WeatherText.cpp \
    LH_WeatherImage.cpp \
    LH_WeatherBrowserOpener.cpp

HEADERS += \
    LH_WeatherData.h \
    LH_QtPlugin_Weather.h \
    LH_WeatherText.h \
    LH_WeatherImage.h \
    LH_WeatherBrowserOpener.h

RESOURCES += \
    LH_Weather.qrc

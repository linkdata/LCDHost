TARGET = LH_TS3
TEMPLATE = lib
DEFINES += LH_TS3_LIBRARY
DEFINES += VERSION=1.01
QT += network
CONFIG += cf

include(../Plugins.pri)

LIBS *= -lLH_Image -lLH_Text

SOURCES += \
    LH_QtPlugin_TS3.cpp \
    LH_TS3SpeakingImage.cpp \
    LH_TS3MuteImage.cpp \
    LH_TS3ConnectionImage.cpp \
    LH_TS3SpeakingText.cpp \
    LH_TS3NicknameText.cpp \
    LH_TS3ChannelNameText.cpp \
    LH_TS3CombinedStatusImage.cpp

HEADERS += \
    LH_QtPlugin_TS3.h \
    LH_TS3_ClientList.h \
    LH_TS3_ChannelList.h \
    LH_TS3MuteImage.h \
    LH_TS3CombinedStatusImage.h

RESOURCES += \
     LH_TS3.qrc

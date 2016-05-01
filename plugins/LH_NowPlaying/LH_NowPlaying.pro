TARGET = LH_NowPlaying
TEMPLATE = lib
CONFIG  += QAxContainer
QT += network xml
DEFINES += LH_NOWPLAYING_LIBRARY
DEFINES += VERSION=3.03
DEFINES += USEINTERNET


DEFINES += WINVER=0x0501
DEFINES += _WIN32_WINNT=0x0501 # 0x0501 WinXP, 0x0601 WinVista

CONFIG += cf

include(../Plugins.pri)
include(../../3rdParty/taglib/taglib.pri)

# We don't want warnings from 3rd party C code
QMAKE_CFLAGS_WARN_ON = -w

LIBS += -L"C:/Program Files/Microsoft SDKs/Windows/v7.0/Lib" -lwininet -lshell32 -lole32 -loleaut32 -luuid -ladvapi32

INCLUDEPATH += \
    SDKs \
    SDKs/iTunes \
    Players \
    Helpers

LIBS *= -lLH_Image -lLH_Bar -lLH_Text

SOURCES += \
    LH_QtPlugin_NowPlaying.cpp \
    SDKs/iTunes/iTunesCOMInterface_i.c \
    Players/Player.cpp \
    Players/PlayerITunes.cpp \
    Players/PlayerWinamp.cpp \
    Players/PlayerWLM.cpp \
    Players/PlayerFoobar.cpp \
    Players/PlayerVLC.cpp \
    Players/PlayerSpotify.cpp \
    Helpers/Lyrics.cpp \
    Helpers/Internet.cpp \
    Helpers/Cover.cpp \
    Objects/LH_NowPlayingText.cpp \
    Objects/LH_NowPlayingBar.cpp \
    Objects/LH_NowPlayingEmulator.cpp \
    Objects/LH_NowPlayingAlbumArt.cpp \
    Objects/LH_NowPlayingRemoteControl.cpp \
    Objects/LH_NowPlayingImage.cpp

HEADERS += \
    LH_QtPlugin_NowPlaying.h \
    SDKs/iTunes/iTunesCOMInterface.h \
    SDKs/iTunes/DispEx.h \
    Players/Player.h \
    Players/PlayerITunes.h \
    Players/PlayerWinamp.h \
    Players/PlayerWLM.h \
    Players/PlayerFoobar.h \
    Players/PlayerVLC.h \
    Players/PlayerSpotify.h \
    Helpers/Lyrics.h \
    Helpers/Internet.h \
    Helpers/Cover.h \
    Objects/LH_NowPlayingText.h \
    Objects/LH_NowPlayingBar.h \
    Objects/LH_NowPlayingEmulator.h \
    Objects/LH_NowPlayingAlbumArt.h \
    Objects/LH_NowPlayingRemoteControl.h \
    Objects/LH_NowPlayingImage.h

RESOURCES += \
    LH_NowPlaying.qrc































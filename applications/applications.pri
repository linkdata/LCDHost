#
# applications.pri
#

not_enabled {

macx-g++|linux-g++-64:exists("$(HOME)/.ccache") {
    QMAKE_CXX="ccache g++"
    QMAKE_CC="ccache gcc"
}

contains(QT, gui): greaterThan(QT_MAJOR_VERSION, 4): QT *= widgets

CONFIG(debug, debug|release): RELDEB = Debug
else: RELDEB = Release

CHANGESET_REVISION = $$system($${HG_BINARY} log -l 1 --template {node}/{rev} $$PWD/$$TARGET)
CHANGESET = $$section(CHANGESET_REVISION,"/",0,0)
REVISION = $$section(CHANGESET_REVISION,"/",1,1)
DEFINES += CHANGESET=\"\\\"$$CHANGESET\\\"\"
DEFINES += REVISION=$$REVISION

INCLUDEPATH += $$PWD/../Plugins/linkdata
HEADERS += $$PWD/../Plugins/linkdata/lh_plugin.h
HEADERS += $$PWD/../Plugins/linkdata/lh_systemstate.h
SOURCES += $$PWD/../Plugins/linkdata/lh_plugin.c

win32 {
    LCDHOST_PLUGINS = $$PWD/../$$RELDEB/plugins
    LH_DIR_BINARIES = $$PWD/../$$RELDEB
}

win32-msvc2010 {
    DEFINES *= _CRT_SECURE_NO_WARNINGS
    LIBS += -luser32 -lgdi32 -ladvapi32
    WINDDK = C:/WinDDK/7600.16385.1
    exists($$WINDDK) {
        DEFINES += HAVE_WINDDK
        LIBS += -L$$WINDDK/lib/wlh/i386
        INCLUDEPATH += $$WINDDK/inc
    }
}

macx {
    LCDHOST_PLUGINS = $$PWD/../$$RELDEB/LCDHost.app/Contents/PlugIns
    LH_DIR_BINARIES = $$PWD/../$$RELDEB/LCDHost.app/Contents/MacOS
    contains(TEMPLATE, app): QMAKE_LFLAGS += -Wl,-rpath,@loader_path,-rpath,@loader_path/../Frameworks
    contains(TEMPLATE, lib): QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/
}

unix:!macx {
    LCDHOST_PLUGINS = $$PWD/../$$RELDEB/plugins
    LH_DIR_BINARIES = $$PWD/../$$RELDEB
    contains(TEMPLATE, app): QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN\''
}

# DESTDIR = $$LH_DIR_BINARIES
}

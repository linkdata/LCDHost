TEMPLATE = lib
CONFIG += shared lh_shared
TARGET = lh_libusbx

QMAKE_CFLAGS_WARN_ON = -w
HEADERS += libusb.h config.h
INCLUDEPATH *= $$PWD
DEFINES += HAVE_STDLIB_H=1 HAVE_SIGNAL_H=1 HAVE_STDINT_H=1 HAVE_STRING_H=1 HAVE_SYS_STAT_H=1 HAVE_SYS_TYPES_H=1 STDC_HEADERS=1

win32 {
    DEFINES += OS_WINDOWS=1 HAVE_MEMORY_H=1
    SOURCES += os/windows_usb.c os/threads_windows.c os/poll_windows.c
    LIBS += -luser32 -lole32 -lsetupapi
    win32-msvc* {
        DEF_FILE = libusb-1.0.def
        OTHER_FILES += libusb-1.0.def
    }
}

macx {
    DEFINES += OS_DARWIN=1 THREADS_POSIX=1 HAVE_SYS_TIME_H=1 HAVE_POLL_H=1
    DEFINES += MAC_OS_X_VERSION_MIN_REQUIRED=1060
    SOURCES += os/darwin_usb.c os/threads_posix.c
    LIBS += -framework CoreFoundation -framework IOKit -lobjc
}

unix:!macx {
    DEFINES += OS_LINUX=1 _GNU_SOURCE=1 THREADS_POSIX=1 USBI_TIMERFD_AVAILABLE=1 HAVE_SYS_TIME_H=1 HAVE_POLL_H=1 HAVE_UNISTD_H=1
    SOURCES += os/linux_usbfs.c os/threads_posix.c
}

SOURCES += core.c descriptor.c io.c sync.c

DESTDIR = $$lh_destdir()

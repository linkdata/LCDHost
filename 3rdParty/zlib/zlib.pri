
ZLIB=zlib-1.2.5
QMAKE_CFLAGS_WARN_ON = -w
DEFINES += USE_FILE32API
DEFINES += HAVE_ZLIB
INCLUDEPATH += $$PWD/$$ZLIB
VPATH += $$PWD/$$ZLIB
HEADERS += zip.h unzip.h
SOURCES += adler32.c compress.c crc32.c deflate.c gzclose.c gzlib.c gzread.c \
    gzwrite.c infback.c inffast.c inflate.c inftrees.c trees.c uncompr.c zutil.c \
    zip.c unzip.c ioapi.c

TEMPLATE = subdirs
CONFIG += ordered

system(echo $$[QT_HOST_BINS]>$$PWD$${QMAKE_DIR_SEP}QT_HOST_BINS)

win32: DEFINES += _CRT_SECURE_NO_WARNINGS

SUBDIRS += qmakecache applications
SUBDIRS += linkdata codeleap plugins
OTHER_FILES += .qmake.cache.in applications/LCDHost/whatsnew.html
OTHER_FILES += lh_features/*.pri

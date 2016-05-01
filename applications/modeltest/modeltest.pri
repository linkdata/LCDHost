greaterThan(QT_MAJOR_VERSION, 4) {
    CONFIG += testcase
    QT += widgets testlib
    INCLUDEPATH += $$PWD
    SOURCES += $$PWD/modeltest.cpp $$PWD/dynamictreemodel.cpp
    HEADERS += $$PWD/modeltest.h $$PWD/dynamictreemodel.h
    DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
}
else {
    INCLUDEPATH += $$PWD/qt4
    SOURCES += $$PWD/qt4/modeltest.cpp
    HEADERS += $$PWD/qt4/modeltest.h
}



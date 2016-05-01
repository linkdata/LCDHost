TARGET = LH_Mailcount
TEMPLATE = lib
DEFINES += LH_MAILCOUNT_LIBRARY
CONFIG += cf

include(../Plugins.pri)

LIBS *= -lLH_Text

SOURCES += \
    LH_Mailcount.cpp \
    LH_QtPlugin_Mailcount.cpp \
    LH_MailcountText.cpp

HEADERS += \
    LH_Mailcount.h \
    LH_QtPlugin_Mailcount.h \
    LH_MailcountText.h

RESOURCES += LH_Mailcount.qrc

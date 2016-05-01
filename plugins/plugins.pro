#
# plugins.pro
#

TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += \
    LH_Bar \
    LH_Decor \
    LH_Dial \
    LH_Graph \
    LH_Image \
    LH_Text

SUBDIRS += \
    LH_Lg160x43 \
    LH_Lg320x240 \
    LH_LgBacklight \
    LH_Mailcount \
    LH_TS3 \
    LH_VirtualLCD \
    LH_Weather \
    LH_WebKit

SUBDIRS += \
    LH_ColorSwatch \
    LH_Cursor \
    LH_DataViewer \
    LH_Logic

win32: SUBDIRS += \
    LH_DriveStats \
    LH_LCoreReboot \
    LH_LgLcdMan \
    LH_Monitoring \
    LH_NowPlaying

OTHER_FILES += Plugins.pri

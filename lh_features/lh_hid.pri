LH_FEATURES += lh_hid
defineTest(lh_hid) {
    LIBS += -llh_hid
    win32: LIBS += -lsetupapi
    macx: LIBS += -framework CoreFoundation
    macx: LIBS += -framework IOKit
    unix:!macx: LIBS += -ludev
    INCLUDEPATH*=$$LH_DIR_INCLUDE/lh_hid
    export(LIBS)
    export(INCLUDEPATH)
    return(true)
}

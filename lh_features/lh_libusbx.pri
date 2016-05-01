LH_FEATURES += lh_libusbx
defineTest(lh_libusbx) {
    LIBS *= -llh_libusbx
    win32: LIBS *= -luser32
    INCLUDEPATH *= $$LH_DIR_LINKDATA/lh_libusbx
    export(LIBS)
    export(INCLUDEPATH)
    return(true)
}

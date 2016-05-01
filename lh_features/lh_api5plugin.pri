LH_FEATURES += lh_api5plugin
defineTest(lh_api5plugin) {
    LIBS *= -llh_api5plugin
    INCLUDEPATH *= $$LH_DIR_LINKDATA/lh_api5plugin
    export(LIBS)
    export(INCLUDEPATH)
    return(true)
}

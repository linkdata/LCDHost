LH_FEATURES += lh_logger
defineTest(lh_logger) {
    LIBS*=-llh_logger
    export(LIBS)
    INCLUDEPATH*=$$LH_DIR_INCLUDE/lh_logger
    export(INCLUDEPATH)
    return(true)
}

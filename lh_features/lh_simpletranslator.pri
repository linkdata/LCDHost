LH_FEATURES += lh_simpletranslator
defineTest(lh_simpletranslator) {
    LIBS *= -llh_simpletranslator
    INCLUDEPATH *= $$LH_DIR_CODELEAP/SimpleTranslator
    export(LIBS)
    export(INCLUDEPATH)
    return(true)
}

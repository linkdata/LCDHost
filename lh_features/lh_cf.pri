LH_FEATURES += lh_cf

defineTest(lh_cf) {
    DEFINES *= LH_CF
    LIBS *= -llh_cf
    INCLUDEPATH *= $$LH_DIR_CODELEAP/ConditionalFormatting
    export(DEFINES)
    export(LIBS)
    export(INCLUDEPATH)
    return(true)
}

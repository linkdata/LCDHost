LH_FEATURES += lh_datacollection

defineTest(lh_datacollection) {
    INCLUDEPATH *= $$LH_DIR_CODELEAP/DataCollection
    export(INCLUDEPATH)
    return(true)
}

LH_FEATURES += lh_log_config
defineTest(lh_log_config) {
    cfgtext += $$quote(TEMPLATE=$$TEMPLATE)
    cfgtext += $$quote(CONFIG=$$CONFIG)
    cfgtext += $$quote(QT=$$QT)
    cfgtext += $$quote(LH_DIR_INSTALL=$$LH_DIR_INSTALL)
    cfgtext += $$quote(LH_DIR_BINARIES=$$LH_DIR_BINARIES)
    cfgtext += $$quote(LH_DIR_PLUGINS=$$LH_DIR_PLUGINS)
    !isEmpty(TARGET): cfgtext += $$quote(TARGET=$$TARGET)
    !isEmpty(DESTDIR): cfgtext += $$quote(DESTDIR=$$DESTDIR)
    for(x, DEFINES): cfgtext += $$quote(DEFINES: \"$$x\")
    for(x, INCLUDEPATH): cfgtext += $$quote(INCLUDEPATH: \"$$x\")
    for(x, LIBS): cfgtext += $$quote(LIBS: \"$$x\")
    for(x, INSTALLS): cfgtext += $$quote(INSTALLS: \"$$x\")

    cfgsuffix=$$quote($$1)
    isEmpty(cfgsuffix): cfgsuffix=.config
    else: cfgsuffix=$$quote(.$${cfgsuffix}.config)
    cfgfile=$$quote($${_PRO_FILE_}$${cfgsuffix})
    exists($$cfgfile) {
        system($$QMAKE_DEL_FILE \"$$cfgfile\")
    }
    for(x, cfgtext) {
        win32: system(echo $$quote($$x)>>\"$$cfgfile\")
        else: system(echo \'$$quote($$x)\'>>\"$$cfgfile\")
    }
    return(true)
}

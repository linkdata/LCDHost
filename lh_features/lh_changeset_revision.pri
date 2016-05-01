LH_FEATURES += lh_changeset_revision
defineTest(lh_changeset_revision) {
    LH_REVISION=0
    LH_CHANGESET="unknown"
    DEFINES+=$$quote(CHANGESET=\\\"$$LH_CHANGESET\\\")
    DEFINES+=$$quote(REVISION=$$LH_REVISION)
    export(DEFINES)
    return(true)
    isEmpty(HG_BINARY):!win32: HG_BINARY=$$system(which hg)
    isEmpty(HG_BINARY):!win32:exists("/usr/local/bin/hg"): HG_BINARY=/usr/local/bin/hg
    isEmpty(HG_BINARY): HG_BINARY=hg
    hg_output=$$system($${HG_BINARY} log -l 1 --template {node}/{rev} \"$$_PRO_FILE_PWD_\")
    LH_CHANGESET=$$section(hg_output,"/",0,0)
    isEmpty(LH_CHANGESET): return(false)
    LH_REVISION=$$section(hg_output,"/",1,1)
    isEmpty(LH_REVISION): LH_REVISION=1
    DEFINES+=$$quote(CHANGESET=\\\"$$LH_CHANGESET\\\")
    DEFINES+=$$quote(REVISION=$$LH_REVISION)
    export(DEFINES)
    return(true)
}

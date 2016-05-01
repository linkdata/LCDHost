LH_FEATURES += lh_shared
defineTest(lh_shared) {
    contains(TEMPLATE, lib): !contains(CONFIG, staticlib) {
        !isEmpty(LH_SHARED_HEADERS) {
            hdr_destdir=$$lh_systempath($${LH_DIR_INCLUDE}/$${TARGET})
            lh_mkdir($$hdr_destdir)
            for(hdr_file, LH_SHARED_HEADERS) {
                hdr_file=$$lh_systempath($${_PRO_FILE_PWD_}/$${hdr_file})
                exists($$hdr_file): lh_copy($$hdr_file, $$hdr_destdir)
            }
        }
    }
    return(true)
}

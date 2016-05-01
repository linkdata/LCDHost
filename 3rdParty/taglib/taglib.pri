BUILD_TAGLIB {
} else {
#    DEFINES += TAGLIB
#    LIBS += -L$$PWD/taglib-release -ltaglib
}

include($$PWD/../zlib/zlib.pri)

DEFINES += TAGLIB
DEFINES += TAGLIB_STATIC
DEFINES += TAGLIB_NO_CONFIG
DEFINES += WITH_ASF
DEFINES += WITH_MP4
DEFINES += TAGLIB_WITH_ASF
DEFINES += TAGLIB_WITH_MP4

INCLUDEPATH += \
    $$PWD/ \
    $$PWD/ape \
    $$PWD/asf \
    $$PWD/flac \
    $$PWD/mp4 \
    $$PWD/mpc \
    $$PWD/mpeg \
    $$PWD/mpeg/id3v1 \
    $$PWD/mpeg/id3v2 \
    $$PWD/mpeg/id3v2/frames \
    $$PWD/ogg \
    $$PWD/ogg/flac \
    $$PWD/ogg/speex \
    $$PWD/ogg/vorbis \
    $$PWD/riff \
    $$PWD/riff/aiff \
    $$PWD/riff/wav \
    $$PWD/toolkit \
    $$PWD/trueaudio \
    $$PWD/wavpack

HEADERS += \
    $$PWD/ape/apetag.h \
    $$PWD/ape/apeproperties.h \
    $$PWD/ape/apeitem.h \
    $$PWD/ape/apefooter.h \
    $$PWD/ape/apefile.h \
    $$PWD/asf/asftag.h \
    $$PWD/asf/asfproperties.h \
    $$PWD/asf/asfpicture.h \
    $$PWD/asf/asffile.h \
    $$PWD/asf/asfattribute.h \
    $$PWD/flac/flacunknownmetadatablock.h \
    $$PWD/flac/flacproperties.h \
    $$PWD/flac/flacpicture.h \
    $$PWD/flac/flacmetadatablock.h \
    $$PWD/flac/flacfile.h \
    $$PWD/mp4/mp4tag.h \
    $$PWD/mp4/mp4properties.h \
    $$PWD/mp4/mp4item.h \
    $$PWD/mp4/mp4file.h \
    $$PWD/mp4/mp4coverart.h \
    $$PWD/mp4/mp4atom.h \
    $$PWD/mpc/mpcproperties.h \
    $$PWD/mpc/mpcfile.h \
    $$PWD/mpeg/xingheader.h \
    $$PWD/mpeg/mpegproperties.h \
    $$PWD/mpeg/mpegheader.h \
    $$PWD/mpeg/mpegfile.h \
    $$PWD/mpeg/id3v1/id3v1tag.h \
    $$PWD/mpeg/id3v1/id3v1genres.h \
    $$PWD/mpeg/id3v2/id3v2tag.h \
    $$PWD/mpeg/id3v2/id3v2synchdata.h \
    $$PWD/mpeg/id3v2/id3v2header.h \
    $$PWD/mpeg/id3v2/id3v2framefactory.h \
    $$PWD/mpeg/id3v2/id3v2frame.h \
    $$PWD/mpeg/id3v2/id3v2footer.h \
    $$PWD/mpeg/id3v2/id3v2extendedheader.h \
    $$PWD/mpeg/id3v2/frames/urllinkframe.h \
    $$PWD/mpeg/id3v2/frames/unsynchronizedlyricsframe.h \
    $$PWD/mpeg/id3v2/frames/unknownframe.h \
    $$PWD/mpeg/id3v2/frames/uniquefileidentifierframe.h \
    $$PWD/mpeg/id3v2/frames/textidentificationframe.h \
    $$PWD/mpeg/id3v2/frames/relativevolumeframe.h \
    $$PWD/mpeg/id3v2/frames/privateframe.h \
    $$PWD/mpeg/id3v2/frames/popularimeterframe.h \
    $$PWD/mpeg/id3v2/frames/generalencapsulatedobjectframe.h \
    $$PWD/mpeg/id3v2/frames/commentsframe.h \
    $$PWD/mpeg/id3v2/frames/attachedpictureframe.h \
    $$PWD/ogg/xiphcomment.h \
    $$PWD/ogg/oggpageheader.h \
    $$PWD/ogg/oggpage.h \
    $$PWD/ogg/oggfile.h \
    $$PWD/ogg/flac/oggflacfile.h \
    $$PWD/ogg/speex/speexproperties.h \
    $$PWD/ogg/speex/speexfile.h \
    $$PWD/ogg/vorbis/vorbisproperties.h \
    $$PWD/ogg/vorbis/vorbisfile.h \
    $$PWD/riff/rifffile.h \
    $$PWD/riff/aiff/aiffproperties.h \
    $$PWD/riff/aiff/aifffile.h \
    $$PWD/riff/wav/wavproperties.h \
    $$PWD/riff/wav/wavfile.h \
    $$PWD/toolkit/unicode.h \
    $$PWD/toolkit/tstringlist.h \
    $$PWD/toolkit/tstring.h \
    $$PWD/toolkit/tmap.h \
    $$PWD/toolkit/tlist.h \
    $$PWD/toolkit/tfile.h \
    $$PWD/toolkit/tdebug.h \
    $$PWD/toolkit/tbytevectorlist.h \
    $$PWD/toolkit/tbytevector.h \
    $$PWD/toolkit/taglib.h \
    $$PWD/trueaudio/trueaudioproperties.h \
    $$PWD/trueaudio/trueaudiofile.h \
    $$PWD/wavpack/wavpackproperties.h \
    $$PWD/wavpack/wavpackfile.h \
    $$PWD/tagunion.h \
    $$PWD/taglib_export.h \
    $$PWD/tag.h \
    $$PWD/fileref.h \
    $$PWD/audioproperties.h

SOURCES += \
    $$PWD/ape/apetag.cpp \
    $$PWD/ape/apeproperties.cpp \
    $$PWD/ape/apeitem.cpp \
    $$PWD/ape/apefooter.cpp \
    $$PWD/ape/apefile.cpp \
    $$PWD/asf/asftag.cpp \
    $$PWD/asf/asfproperties.cpp \
    $$PWD/asf/asfpicture.cpp \
    $$PWD/asf/asffile.cpp \
    $$PWD/asf/asfattribute.cpp \
    $$PWD/flac/flacunknownmetadatablock.cpp \
    $$PWD/flac/flacproperties.cpp \
    $$PWD/flac/flacpicture.cpp \
    $$PWD/flac/flacmetadatablock.cpp \
    $$PWD/flac/flacfile.cpp \
    $$PWD/mp4/mp4tag.cpp \
    $$PWD/mp4/mp4properties.cpp \
    $$PWD/mp4/mp4item.cpp \
    $$PWD/mp4/mp4file.cpp \
    $$PWD/mp4/mp4coverart.cpp \
    $$PWD/mp4/mp4atom.cpp \
    $$PWD/mpc/mpcproperties.cpp \
    $$PWD/mpc/mpcfile.cpp \
    $$PWD/mpeg/xingheader.cpp \
    $$PWD/mpeg/mpegproperties.cpp \
    $$PWD/mpeg/mpegheader.cpp \
    $$PWD/mpeg/mpegfile.cpp \
    $$PWD/mpeg/id3v1/id3v1tag.cpp \
    $$PWD/mpeg/id3v1/id3v1genres.cpp \
    $$PWD/mpeg/id3v2/id3v2tag.cpp \
    $$PWD/mpeg/id3v2/id3v2synchdata.cpp \
    $$PWD/mpeg/id3v2/id3v2header.cpp \
    $$PWD/mpeg/id3v2/id3v2framefactory.cpp \
    $$PWD/mpeg/id3v2/id3v2frame.cpp \
    $$PWD/mpeg/id3v2/id3v2footer.cpp \
    $$PWD/mpeg/id3v2/id3v2extendedheader.cpp \
    $$PWD/mpeg/id3v2/frames/urllinkframe.cpp \
    $$PWD/mpeg/id3v2/frames/unsynchronizedlyricsframe.cpp \
    $$PWD/mpeg/id3v2/frames/unknownframe.cpp \
    $$PWD/mpeg/id3v2/frames/uniquefileidentifierframe.cpp \
    $$PWD/mpeg/id3v2/frames/textidentificationframe.cpp \
    $$PWD/mpeg/id3v2/frames/relativevolumeframe.cpp \
    $$PWD/mpeg/id3v2/frames/privateframe.cpp \
    $$PWD/mpeg/id3v2/frames/popularimeterframe.cpp \
    $$PWD/mpeg/id3v2/frames/generalencapsulatedobjectframe.cpp \
    $$PWD/mpeg/id3v2/frames/commentsframe.cpp \
    $$PWD/mpeg/id3v2/frames/attachedpictureframe.cpp \
    $$PWD/ogg/xiphcomment.cpp \
    $$PWD/ogg/oggpageheader.cpp \
    $$PWD/ogg/oggpage.cpp \
    $$PWD/ogg/oggfile.cpp \
    $$PWD/ogg/flac/oggflacfile.cpp \
    $$PWD/ogg/speex/speexproperties.cpp \
    $$PWD/ogg/speex/speexfile.cpp \
    $$PWD/ogg/vorbis/vorbisproperties.cpp \
    $$PWD/ogg/vorbis/vorbisfile.cpp \
    $$PWD/riff/rifffile.cpp \
    $$PWD/riff/aiff/aiffproperties.cpp \
    $$PWD/riff/aiff/aifffile.cpp \
    $$PWD/riff/wav/wavproperties.cpp \
    $$PWD/riff/wav/wavfile.cpp \
    $$PWD/toolkit/unicode.cpp \
    $$PWD/toolkit/tstringlist.cpp \
    $$PWD/toolkit/tstring.cpp \
    $$PWD/toolkit/tmap.tcc \
    $$PWD/toolkit/tlist.tcc \
    $$PWD/toolkit/tfile.cpp \
    $$PWD/toolkit/tdebug.cpp \
    $$PWD/toolkit/tbytevectorlist.cpp \
    $$PWD/toolkit/tbytevector.cpp \
    $$PWD/trueaudio/trueaudioproperties.cpp \
    $$PWD/trueaudio/trueaudiofile.cpp \
    $$PWD/wavpack/wavpackproperties.cpp \
    $$PWD/wavpack/wavpackfile.cpp \
    $$PWD/tagunion.cpp \
    $$PWD/tag.cpp \
    $$PWD/fileref.cpp \
    $$PWD/audioproperties.cpp

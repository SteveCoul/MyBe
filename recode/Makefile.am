
bin_PROGRAMS=recode

recode_CXXFLAGS=
recode_CXXFLAGS+=-I$(FFMPEG_INCLUDE_DIR)
recode_SOURCES=
recode_SOURCES+=AlternateVideoTask.cpp
recode_SOURCES+=entrypoint.cpp
recode_SOURCES+=ImagePipeline.cpp
recode_SOURCES+=ImageWriter.cpp
recode_SOURCES+=Main.cpp
recode_SOURCES+=Misc.cpp
recode_SOURCES+=Options.cpp
recode_SOURCES+=PAT.cpp
recode_SOURCES+=PES.cpp
recode_SOURCES+=PMT.cpp
recode_SOURCES+=TS.cpp
recode_SOURCES+=TSPacket.cpp
recode_SOURCES+=VideoDebugTask.cpp
recode_SOURCES+=VideoDecoder.cpp
recode_SOURCES+=VideoEncoder.cpp
recode_SOURCES+=xlog.cpp

recode_LDADD=-lavformat -lavcodec -lavutil -lswresample -lx264

recode_LDFLAGS=-L$(FFMPEG_LIBRARY_DIR)

lib_LTLIBRARIES=libMyBeLoader.la

libMyBeLoader_la_CXXFLAGS=-I.
libMyBeLoader_la_SOURCES=MyBeLoader.cpp

bin_PROGRAMS+=loader_test

loader_test_CXXFLAGS=
loader_test_SOURCES=loader_test.cpp
loader_test_LDADD=libMyBeLoader.la
loader_test_LDFLAGS=-lcurl

bin_PROGRAMS+=referencedecoder_test
referencedecoder_test_SOURCES=referencedecoder_test.cpp


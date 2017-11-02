
THIRD_PARTY=$(PWD)/3rdParty

# ---------------

all: ffmpeg
	$(CXX) -I. -o recode main.cpp xlog.cpp TS.cpp TSPacket.cpp Options.cpp PAT.cpp

clean: clean_deps
	rm -rf $(THIRD_PARTY)
	rm -rf *.dSYM
	rm -f recode

# ---------------

UNAME := $(shell uname)

ifeq ($(UNAME), Darwin)
HOST=OSX
endif

ifeq (x$(HOST), xOSX)
DEPS_PATH=$(PWD)/$(HOST).deps/bin:$(PATH)
HOST_DEPS=$(PWD)/$(HOST).deps/.built

$(HOST_DEPS):
	$(MAKE) DEP_DIR=$(PWD)/$(HOST).deps -f Makefile.$(HOST).deps

clean_deps:
	rm -rf $(HOST).deps

else
DEPS_PATH=$(PATH)
HOST_DEPS=
clean_deps:
endif

# ---------------

.PHONY: x264
x264: $(THIRD_PARTY)/lib/libx264.a

X264_DIR=$(THIRD_PARTY)/x264

$(THIRD_PARTY)/lib/libx264.a: $(X264_DIR)/libx264.a

$(X264_DIR)/libx264.a: $(X264_DIR)/.configured 
	PATH=$(DEPS_PATH) && make -C $(X264_DIR) install

$(X264_DIR)/.configured: $(X264_DIR)/configure $(HOST_DEPS)
	cd $(X264_DIR) && PATH=$(DEPS_PATH) ./configure --prefix=$(THIRD_PARTY) --enable-static --enable-shared
	touch $@

$(X264_DIR)/configure:
	mkdir -p $(dir $@)
	cd $(THIRD_PARTY) && git clone http://git.videolan.org/git/x264.git
	cd $(X264_DIR) && git checkout ba24899b0bf23345921da022f7a51e0c57dbe73d

# ---------------

.PHONY: ffmpeg
ffmpeg: $(THIRD_PARTY)/bin/ffmpeg

FFMPEG_DIR=$(THIRD_PARTY)/ffmpeg

$(THIRD_PARTY)/bin/ffmpeg: $(FFMPEG_DIR)/.configured
	PATH=$(DEPS_PATH) $(MAKE) -C $(FFMPEG_DIR) install

$(FFMPEG_DIR)/.configured: $(FFMPEG_DIR)/configure $(HOST_DEPS) $(THIRD_PARTY)/lib/libx264.a
	cd $(FFMPEG_DIR) && PATH=$(DEPS_PATH) CFLAGS="-g2" CXXFLAGS="-g2" LDFLAGS="-g2" ./configure --prefix=$(THIRD_PARTY) --enable-static --enable-shared --enable-gpl --enable-libx264 --extra-ldflags=-L$(THIRD_PARTY)/lib --extra-cflags=-I$(THIRD_PARTY)/include  --disable-stripping
	touch $@

$(FFMPEG_DIR)/configure:
	mkdir -p $(dir $@)
	cd $(THIRD_PARTY) && git clone https://git.ffmpeg.org/ffmpeg.git
	cd $(FFMPEG_DIR) && git checkout 858db4b01fa2b55ee55056c033054ca54ac9b0fd

# ---------------


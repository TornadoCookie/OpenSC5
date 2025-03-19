# Generated using Helium v2.1.1 (https://github.com/tornadocookie/he)

PLATFORM?=linux64-debug
DISTDIR?=build

.PHONY: all

RAYLIB_NAME=raylib5.5-$(PLATFORM)

ifeq ($(PLATFORM), linux64-debug)
EXEC_EXTENSION=-debug
LIB_EXTENSION=-debug.so
LIB_EXTENSION_STATIC=(null)
CC=gcc
RAYLIB_DLL=-lraylib
CFLAGS+=-g
CFLAGS+=-D DEBUG
CFLAGS+=-D EXEC_EXTENSION=\"-debug\"
CFLAGS+=-D LIB_EXTENSION=\"-debug.so\"
endif

ifeq ($(PLATFORM), win64)
EXEC_EXTENSION=.exe
LIB_EXTENSION=.dll
LIB_EXTENSION_STATIC=(null)
CC=x86_64-w64-mingw32-gcc
CXX=x86_64-w64-mingw32-g++
RAYLIB_DLL=-lraylibdll
CFLAGS+=-O2
CFLAGS+=-D RELEASE
CFLAGS+=-D EXEC_EXTENSION=\".exe\"
CFLAGS+=-D LIB_EXTENSION=\".dll\"
LDFLAGS+=-lws2_32
LDFLAGS+=-static-libstdc++
LDFLAGS+=-static-libgcc
endif

PROGRAMS=test_package test_update test_crcbin test_prop test_rast test_rw4 test_sdelta test_heightmap test_rules test_statefile test_hash opensc5_editor opensc5 test_dbpf
LIBRARIES=

curl_NAME=libcurl-$(PLATFORM)
CFLAGS+=-Ilib/$(curl_NAME)/include
LDFLAGS+=-Llib/$(curl_NAME)/lib
LDFLAGS+=-lcurl
LDFLAGS+=-Wl,-rpath,lib/$(curl_NAME)/lib


EAWebKitd_NAME=libEAWebKitd-$(PLATFORM)
CFLAGS+=-Ilib/$(EAWebKitd_NAME)/include
LDFLAGS+=-Llib/$(EAWebKitd_NAME)/lib
LDFLAGS+=-lEAWebKitd
LDFLAGS+=-Wl,-rpath,lib/$(EAWebKitd_NAME)/lib


all: $(DISTDIR) $(DISTDIR)/src $(DISTDIR)/src/rlWebKit $(DISTDIR)/src/filetypes $(DISTDIR)/src/ww2ogg $(DISTDIR)/src/../tests $(foreach prog, $(PROGRAMS), $(DISTDIR)/$(prog)$(EXEC_EXTENSION)) $(foreach lib, $(LIBRARIES), $(DISTDIR)/$(lib)$(LIB_EXTENSION) $(DISTDIR)/$(lib)$(LIB_EXTENSION_STATIC)) deps

ifneq ($(DISTDIR), .)
deps:
	mkdir -p $(DISTDIR)/lib
	if [ -d lib/$(curl_NAME) ] && [ ! -d $(DISTDIR)/lib/$(curl_NAME) ]; then cp -r lib/$(curl_NAME) $(DISTDIR)/lib; fi
	if [ -d lib/$(EAWebKitd_NAME) ] && [ ! -d $(DISTDIR)/lib/$(EAWebKitd_NAME) ]; then cp -r lib/$(EAWebKitd_NAME) $(DISTDIR)/lib; fi
	if [ -d lib/$(RAYLIB_NAME) ] && [ ! -d $(DISTDIR)/lib/$(RAYLIB_NAME) ]; then cp -r lib/$(RAYLIB_NAME) $(DISTDIR)/lib; fi
	cp -r packed_codebooks_aoTuV_603.bin $(DISTDIR)
	cp -r README.md $(DISTDIR)
	cp -r LICENSE $(DISTDIR)
	cp -r Properties.txt $(DISTDIR)
	cp -r launcher.bat $(DISTDIR)
else
deps:
endif


$(DISTDIR)/src $(DISTDIR)/src/rlWebKit $(DISTDIR)/src/filetypes $(DISTDIR)/src/ww2ogg $(DISTDIR)/src/../tests:
	mkdir -p $@

$(DISTDIR):
	mkdir -p $@

CFLAGS+=-Isrc
CFLAGS+=-Iinclude
CFLAGS+=-D PLATFORM=\"$(PLATFORM)\"
CFLAGS+=-Isrc/ww2ogg


CFLAGS+=-Ilib/$(RAYLIB_NAME)/include

LDFLAGS+=-lm
LDFLAGS+=-Llib/$(RAYLIB_NAME)/lib
LDFLAGS+=$(RAYLIB_DLL)
LDFLAGS+=-Wl,-rpath,lib/$(RAYLIB_NAME)/lib

shared_SOURCES+=$(DISTDIR)/src/tracelog.o
shared_SOURCES+=$(DISTDIR)/src/hash.o
shared_SOURCES+=$(DISTDIR)/src/memstream.o

rlWebKit_CXX_SOURCES+=$(DISTDIR)/src/rlWebKit/rlWebKit.o
rlWebKit_CXX_SOURCES+=$(DISTDIR)/src/rlWebKit/rlWebkitClient.o
rlWebKit_CXX_SOURCES+=$(DISTDIR)/src/rlWebKit/rlWebkitRenderer.o
rlWebKit_CXX_SOURCES+=$(DISTDIR)/src/rlWebKit/rlWebkitThreading.o
rlWebKit_CXX_SOURCES+=$(DISTDIR)/src/rlWebKit/rlWebkitUtils.o

dbpf_all_SOURCES+=$(DISTDIR)/src/filetypes/package.o
dbpf_all_SOURCES+=$(DISTDIR)/src/filetypes/prop.o
dbpf_all_SOURCES+=$(DISTDIR)/src/filetypes/rules.o
dbpf_all_SOURCES+=$(DISTDIR)/src/filetypes/rast.o
dbpf_all_SOURCES+=$(DISTDIR)/src/filetypes/bnk.o
dbpf_all_SOURCES+=$(DISTDIR)/src/filetypes/rw4.o
dbpf_all_CXX_SOURCES+=$(DISTDIR)/src/filetypes/wwriff.o
dbpf_all_CXX_SOURCES+=$(DISTDIR)/src/ww2ogg/wwriff.o
dbpf_all_CXX_SOURCES+=$(DISTDIR)/src/ww2ogg/codebook.o
dbpf_all_SOURCES+=$(DISTDIR)/src/ww2ogg/crc.o
dbpf_all_SOURCES+=$(DISTDIR)/src/threadpool.o
dbpf_all_CXX_SOURCES+=$(shared_CXX_SOURCES)
dbpf_all_SOURCES+=$(shared_SOURCES)

test_package_SOURCES+=$(DISTDIR)/src/../tests/test_package.o
test_package_CXX_SOURCES+=$(dbpf_all_CXX_SOURCES)
test_package_SOURCES+=$(dbpf_all_SOURCES)

$(DISTDIR)/test_package$(EXEC_EXTENSION): $(test_package_SOURCES) $(test_package_CXX_SOURCES)
	$(CXX) -o $@ $^ $(LDFLAGS)

test_update_SOURCES+=$(DISTDIR)/src/../tests/test_update.o

$(DISTDIR)/test_update$(EXEC_EXTENSION): $(test_update_SOURCES)
	$(CC) -o $@ $^ $(LDFLAGS)

test_crcbin_SOURCES+=$(DISTDIR)/src/../tests/test_crcbin.o
test_crcbin_SOURCES+=$(DISTDIR)/src/filetypes/crcbin.o
test_crcbin_SOURCES+=$(DISTDIR)/src/crc32.o

$(DISTDIR)/test_crcbin$(EXEC_EXTENSION): $(test_crcbin_SOURCES)
	$(CC) -o $@ $^ $(LDFLAGS)

test_prop_SOURCES+=$(DISTDIR)/src/../tests/test_prop.o
test_prop_SOURCES+=$(DISTDIR)/src/filetypes/prop.o
test_prop_SOURCES+=$(shared_SOURCES)

$(DISTDIR)/test_prop$(EXEC_EXTENSION): $(test_prop_SOURCES)
	$(CC) -o $@ $^ $(LDFLAGS)

test_rast_SOURCES+=$(DISTDIR)/src/../tests/test_rast.o
test_rast_SOURCES+=$(DISTDIR)/src/filetypes/rast.o
test_rast_SOURCES+=$(shared_SOURCES)

$(DISTDIR)/test_rast$(EXEC_EXTENSION): $(test_rast_SOURCES)
	$(CC) -o $@ $^ $(LDFLAGS)

test_rw4_SOURCES+=$(DISTDIR)/src/../tests/test_rw4.o
test_rw4_SOURCES+=$(DISTDIR)/src/filetypes/rw4.o
test_rw4_SOURCES+=$(shared_SOURCES)

$(DISTDIR)/test_rw4$(EXEC_EXTENSION): $(test_rw4_SOURCES)
	$(CC) -o $@ $^ $(LDFLAGS)

test_sdelta_SOURCES+=$(DISTDIR)/src/../tests/test_sdelta.o
test_sdelta_SOURCES+=$(DISTDIR)/src/filetypes/sdelta.o
test_sdelta_SOURCES+=$(shared_SOURCES)

$(DISTDIR)/test_sdelta$(EXEC_EXTENSION): $(test_sdelta_SOURCES)
	$(CC) -o $@ $^ $(LDFLAGS)

test_heightmap_SOURCES+=$(DISTDIR)/src/../tests/test_heightmap.o
test_heightmap_SOURCES+=$(DISTDIR)/src/filetypes/heightmap.o
test_heightmap_SOURCES+=$(shared_SOURCES)

$(DISTDIR)/test_heightmap$(EXEC_EXTENSION): $(test_heightmap_SOURCES)
	$(CC) -o $@ $^ $(LDFLAGS)

test_rules_SOURCES+=$(DISTDIR)/src/../tests/test_rules.o
test_rules_SOURCES+=$(DISTDIR)/src/filetypes/rules.o

$(DISTDIR)/test_rules$(EXEC_EXTENSION): $(test_rules_SOURCES)
	$(CC) -o $@ $^ $(LDFLAGS)

test_statefile_SOURCES+=$(DISTDIR)/src/../tests/test_statefile.o
test_statefile_SOURCES+=$(DISTDIR)/src/filetypes/statefile.o

$(DISTDIR)/test_statefile$(EXEC_EXTENSION): $(test_statefile_SOURCES)
	$(CC) -o $@ $^ $(LDFLAGS)

test_hash_SOURCES+=$(DISTDIR)/src/../tests/test_hash.o
test_hash_SOURCES+=$(DISTDIR)/src/hash.o

$(DISTDIR)/test_hash$(EXEC_EXTENSION): $(test_hash_SOURCES)
	$(CC) -o $@ $^ $(LDFLAGS)

opensc5_editor_SOURCES+=$(DISTDIR)/src/editor.o
opensc5_editor_SOURCES+=$(DISTDIR)/src/getopt.o
opensc5_editor_CXX_SOURCES+=$(dbpf_all_CXX_SOURCES)
opensc5_editor_SOURCES+=$(dbpf_all_SOURCES)

$(DISTDIR)/opensc5_editor$(EXEC_EXTENSION): $(opensc5_editor_SOURCES) $(opensc5_editor_CXX_SOURCES)
	$(CXX) -o $@ $^ $(LDFLAGS)

opensc5_SOURCES+=$(DISTDIR)/src/game.o
opensc5_CXX_SOURCES+=$(dbpf_all_CXX_SOURCES)
opensc5_SOURCES+=$(dbpf_all_SOURCES)
opensc5_CXX_SOURCES+=$(rlWebKit_CXX_SOURCES)
opensc5_SOURCES+=$(rlWebKit_SOURCES)

$(DISTDIR)/opensc5$(EXEC_EXTENSION): $(opensc5_SOURCES) $(opensc5_CXX_SOURCES)
	$(CXX) -o $@ $^ $(LDFLAGS)

test_dbpf_SOURCES+=$(DISTDIR)/src/../tests/test_dbpf.o
test_dbpf_CXX_SOURCES+=$(dbpf_all_CXX_SOURCES)
test_dbpf_SOURCES+=$(dbpf_all_SOURCES)

$(DISTDIR)/test_dbpf$(EXEC_EXTENSION): $(test_dbpf_SOURCES) $(test_dbpf_CXX_SOURCES)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(DISTDIR)/%.o: %.c
	$(CC) -c $^ $(CFLAGS) -o $@

$(DISTDIR)/%.o: %.cpp
	$(CXX) -c $^ $(CFLAGS) -o $@

clean:
	rm -f $(DISTDIR)/src/tracelog.o
	rm -f $(DISTDIR)/src/hash.o
	rm -f $(DISTDIR)/src/memstream.o
	rm -f $(DISTDIR)/src/rlWebKit/rlWebKit.o
	rm -f $(DISTDIR)/src/rlWebKit/rlWebkitClient.o
	rm -f $(DISTDIR)/src/rlWebKit/rlWebkitRenderer.o
	rm -f $(DISTDIR)/src/rlWebKit/rlWebkitThreading.o
	rm -f $(DISTDIR)/src/rlWebKit/rlWebkitUtils.o
	rm -f $(DISTDIR)/src/filetypes/package.o
	rm -f $(DISTDIR)/src/filetypes/prop.o
	rm -f $(DISTDIR)/src/filetypes/rules.o
	rm -f $(DISTDIR)/src/filetypes/rast.o
	rm -f $(DISTDIR)/src/filetypes/bnk.o
	rm -f $(DISTDIR)/src/filetypes/rw4.o
	rm -f $(DISTDIR)/src/filetypes/wwriff.o
	rm -f $(DISTDIR)/src/ww2ogg/wwriff.o
	rm -f $(DISTDIR)/src/ww2ogg/codebook.o
	rm -f $(DISTDIR)/src/ww2ogg/crc.o
	rm -f $(DISTDIR)/src/threadpool.o
	rm -f $(DISTDIR)/src/../tests/test_package.o
	rm -f $(DISTDIR)/test_package$(EXEC_EXTENSION)
	rm -f $(DISTDIR)/src/../tests/test_update.o
	rm -f $(DISTDIR)/test_update$(EXEC_EXTENSION)
	rm -f $(DISTDIR)/src/../tests/test_crcbin.o
	rm -f $(DISTDIR)/src/filetypes/crcbin.o
	rm -f $(DISTDIR)/src/crc32.o
	rm -f $(DISTDIR)/test_crcbin$(EXEC_EXTENSION)
	rm -f $(DISTDIR)/src/../tests/test_prop.o
	rm -f $(DISTDIR)/src/filetypes/prop.o
	rm -f $(DISTDIR)/test_prop$(EXEC_EXTENSION)
	rm -f $(DISTDIR)/src/../tests/test_rast.o
	rm -f $(DISTDIR)/src/filetypes/rast.o
	rm -f $(DISTDIR)/test_rast$(EXEC_EXTENSION)
	rm -f $(DISTDIR)/src/../tests/test_rw4.o
	rm -f $(DISTDIR)/src/filetypes/rw4.o
	rm -f $(DISTDIR)/test_rw4$(EXEC_EXTENSION)
	rm -f $(DISTDIR)/src/../tests/test_sdelta.o
	rm -f $(DISTDIR)/src/filetypes/sdelta.o
	rm -f $(DISTDIR)/test_sdelta$(EXEC_EXTENSION)
	rm -f $(DISTDIR)/src/../tests/test_heightmap.o
	rm -f $(DISTDIR)/src/filetypes/heightmap.o
	rm -f $(DISTDIR)/test_heightmap$(EXEC_EXTENSION)
	rm -f $(DISTDIR)/src/../tests/test_rules.o
	rm -f $(DISTDIR)/src/filetypes/rules.o
	rm -f $(DISTDIR)/test_rules$(EXEC_EXTENSION)
	rm -f $(DISTDIR)/src/../tests/test_statefile.o
	rm -f $(DISTDIR)/src/filetypes/statefile.o
	rm -f $(DISTDIR)/test_statefile$(EXEC_EXTENSION)
	rm -f $(DISTDIR)/src/../tests/test_hash.o
	rm -f $(DISTDIR)/src/hash.o
	rm -f $(DISTDIR)/test_hash$(EXEC_EXTENSION)
	rm -f $(DISTDIR)/src/editor.o
	rm -f $(DISTDIR)/src/getopt.o
	rm -f $(DISTDIR)/opensc5_editor$(EXEC_EXTENSION)
	rm -f $(DISTDIR)/src/game.o
	rm -f $(DISTDIR)/opensc5$(EXEC_EXTENSION)
	rm -f $(DISTDIR)/src/../tests/test_dbpf.o
	rm -f $(DISTDIR)/test_dbpf$(EXEC_EXTENSION)

all_dist:
	DISTDIR=$(DISTDIR)/dist/linux64-debug PLATFORM=linux64-debug $(MAKE)
	DISTDIR=$(DISTDIR)/dist/win64 PLATFORM=win64 $(MAKE)

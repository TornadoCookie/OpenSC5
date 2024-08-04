# Generated using Helium v1.3.2 (https://github.com/tornadocookie/he)

PLATFORM?=linux64-debug
DISTDIR?=.

.PHONY: all

RAYLIB_NAME=raylib5-$(PLATFORM)

ifeq ($(PLATFORM), linux64-debug)
EXEC_EXTENSION=-debug
LIB_EXTENSION=-debug.so
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
CC=x86_64-w64-mingw32-gcc
RAYLIB_DLL=-lraylibdll
CFLAGS+=-O2
CFLAGS+=-D RELEASE
CFLAGS+=-D EXEC_EXTENSION=\".exe\"
CFLAGS+=-D LIB_EXTENSION=\".dll\"
CFLAGS+=-lws2_32
endif

PROGRAMS=test_package test_update test_crcbin test_prop test_rast test_rw4 test_sdelta test_heightmap test_rules opensc5_editor opensc5
LIBRARIES=

curl_NAME=libcurl-$(PLATFORM)
CFLAGS+=-Ilib/$(curl_NAME)/include
CFLAGS+=-Wl,-rpath,lib/$(curl_NAME)/lib
LDFLAGS+=-Llib/$(curl_NAME)/lib
LDFLAGS+=-lcurl

wwriff_NAME=libwwriff-$(PLATFORM)
CFLAGS+=-Ilib/$(wwriff_NAME)/include
CFLAGS+=-Wl,-rpath,lib/$(wwriff_NAME)/lib
LDFLAGS+=-Llib/$(wwriff_NAME)/lib
LDFLAGS+=-lwwriff

all: $(DISTDIR) $(foreach prog, $(PROGRAMS), $(DISTDIR)/$(prog)$(EXEC_EXTENSION)) $(foreach lib, $(LIBRARIES), $(DISTDIR)/$(lib)$(LIB_EXTENSION)) deps

ifneq ($(DISTDIR), .)
deps:
	mkdir -p $(DISTDIR)/lib
	if [ -d lib/$(curl_NAME) ]; then cp -r lib/$(curl_NAME) $(DISTDIR)/lib; fi
	if [ -d lib/$(wwriff_NAME) ]; then cp -r lib/$(wwriff_NAME) $(DISTDIR)/lib; fi
	if [ -d lib/$(RAYLIB_NAME) ]; then cp -r lib/$(RAYLIB_NAME) $(DISTDIR)/lib; fi
	cp -r packed_codebooks_aoTuV_603.bin $(DISTDIR)
	cp -r README.md $(DISTDIR)
	cp -r LICENSE $(DISTDIR)
	cp -r Properties.txt $(DISTDIR)
	cp -r launcher.bat $(DISTDIR)
else
deps:
endif

$(DISTDIR):
	mkdir -p $@

CFLAGS+=-Isrc
CFLAGS+=-Iinclude
CFLAGS+=-D PLATFORM=\"$(PLATFORM)\"

CFLAGS+=-Ilib/$(RAYLIB_NAME)/include
CFLAGS+=-Wl,-rpath,lib/$(RAYLIB_NAME)/lib

LDFLAGS+=-lm
LDFLAGS+=-Llib/$(RAYLIB_NAME)/lib
LDFLAGS+=$(RAYLIB_DLL)

dbpf_all_SOURCES+=src/filetypes/package.c
dbpf_all_SOURCES+=src/filetypes/prop.c
dbpf_all_SOURCES+=src/filetypes/rules.c
dbpf_all_SOURCES+=src/filetypes/rast.c
dbpf_all_SOURCES+=src/filetypes/bnk.c
dbpf_all_SOURCES+=src/threadpool.c
dbpf_all_SOURCES+=src/hash.c

test_package_SOURCES+=src/../tests/test_package.c
test_package_SOURCES+=$(dbpf_all_SOURCES)

$(DISTDIR)/test_package$(EXEC_EXTENSION): $(test_package_SOURCES)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

test_update_SOURCES+=src/../tests/test_update.c

$(DISTDIR)/test_update$(EXEC_EXTENSION): $(test_update_SOURCES)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

test_crcbin_SOURCES+=src/../tests/test_crcbin.c
test_crcbin_SOURCES+=src/filetypes/crcbin.c
test_crcbin_SOURCES+=src/crc32.c

$(DISTDIR)/test_crcbin$(EXEC_EXTENSION): $(test_crcbin_SOURCES)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

test_prop_SOURCES+=src/../tests/test_prop.c
test_prop_SOURCES+=src/filetypes/prop.c
test_prop_SOURCES+=src/hash.c

$(DISTDIR)/test_prop$(EXEC_EXTENSION): $(test_prop_SOURCES)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

test_rast_SOURCES+=src/../tests/test_rast.c
test_rast_SOURCES+=src/filetypes/rast.c

$(DISTDIR)/test_rast$(EXEC_EXTENSION): $(test_rast_SOURCES)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

test_rw4_SOURCES+=src/../tests/test_rw4.c
test_rw4_SOURCES+=src/filetypes/rw4.c

$(DISTDIR)/test_rw4$(EXEC_EXTENSION): $(test_rw4_SOURCES)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

test_sdelta_SOURCES+=src/../tests/test_sdelta.c
test_sdelta_SOURCES+=src/filetypes/sdelta.c

$(DISTDIR)/test_sdelta$(EXEC_EXTENSION): $(test_sdelta_SOURCES)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

test_heightmap_SOURCES+=src/../tests/test_heightmap.c
test_heightmap_SOURCES+=src/filetypes/heightmap.c

$(DISTDIR)/test_heightmap$(EXEC_EXTENSION): $(test_heightmap_SOURCES)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

test_rules_SOURCES+=src/../tests/test_rules.c
test_rules_SOURCES+=src/filetypes/rules.c

$(DISTDIR)/test_rules$(EXEC_EXTENSION): $(test_rules_SOURCES)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

opensc5_editor_SOURCES+=src/editor.c
opensc5_editor_SOURCES+=src/getopt.c
opensc5_editor_SOURCES+=$(dbpf_all_SOURCES)

$(DISTDIR)/opensc5_editor$(EXEC_EXTENSION): $(opensc5_editor_SOURCES)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

opensc5_SOURCES+=src/game.c
opensc5_SOURCES+=$(dbpf_all_SOURCES)

$(DISTDIR)/opensc5$(EXEC_EXTENSION): $(opensc5_SOURCES)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(DISTDIR)/test_package$(EXEC_EXTENSION)
	rm -f $(DISTDIR)/test_update$(EXEC_EXTENSION)
	rm -f $(DISTDIR)/test_crcbin$(EXEC_EXTENSION)
	rm -f $(DISTDIR)/test_prop$(EXEC_EXTENSION)
	rm -f $(DISTDIR)/test_rast$(EXEC_EXTENSION)
	rm -f $(DISTDIR)/test_rw4$(EXEC_EXTENSION)
	rm -f $(DISTDIR)/test_sdelta$(EXEC_EXTENSION)
	rm -f $(DISTDIR)/test_heightmap$(EXEC_EXTENSION)
	rm -f $(DISTDIR)/test_rules$(EXEC_EXTENSION)
	rm -f $(DISTDIR)/opensc5_editor$(EXEC_EXTENSION)
	rm -f $(DISTDIR)/opensc5$(EXEC_EXTENSION)

all_dist:
	DISTDIR=$(DISTDIR)/dist/linux64-debug PLATFORM=linux64-debug $(MAKE)
	DISTDIR=$(DISTDIR)/dist/win64 PLATFORM=win64 $(MAKE)

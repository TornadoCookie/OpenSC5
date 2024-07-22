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
CFLAGS+=-Llib/libcurl-win64/lib
CFLAGS+=-lws2_32
CFLAGS+=-Ilib/libcurl-win64/include
endif

PROGRAMS=test_package test_update test_crcbin test_prop test_rast test_rw4 test_sdelta test_heightmap opensc5_editor
LIBRARIES=

all: $(DISTDIR) $(foreach prog, $(PROGRAMS), $(DISTDIR)/$(prog)$(EXEC_EXTENSION)) $(foreach lib, $(LIBRARIES), $(DISTDIR)/$(lib)$(LIB_EXTENSION))

$(DISTDIR):
	mkdir -p $@

CFLAGS+=-Isrc
CFLAGS+=-Iinclude
CFLAGS+=-D PLATFORM=\"$(PLATFORM)\"
CFLAGS+=-lcurl

CFLAGS+=-Ilib/$(RAYLIB_NAME)/include
CFLAGS+=-Wl,-rpath,lib/$(RAYLIB_NAME)/lib

LDFLAGS+=-lm
LDFLAGS+=-Llib/$(RAYLIB_NAME)/lib
LDFLAGS+=$(RAYLIB_DLL)

wwriff_NAME=libwwriff-$(PLATFORM)
CFLAGS+=-Ilib/$(wwriff_NAME)/include
CFLAGS+=-Wl,-rpath,lib/$(wwriff_NAME)/lib
LDFLAGS+=-Llib/$(wwriff_NAME)/lib
LDFLAGS+=-lwwriff

dbpf_all_SOURCES+=src/filetypes/package.c
dbpf_all_SOURCES+=src/filetypes/prop.c
dbpf_all_SOURCES+=src/filetypes/rules.c
dbpf_all_SOURCES+=src/filetypes/rast.c
dbpf_all_SOURCES+=src/filetypes/bnk.c

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

opensc5_editor_SOURCES+=src/editor.c
opensc5_editor_SOURCES+=$(dbpf_all_SOURCES)

$(DISTDIR)/opensc5_editor$(EXEC_EXTENSION): $(opensc5_editor_SOURCES)
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
	rm -f $(DISTDIR)/opensc5_editor$(EXEC_EXTENSION)

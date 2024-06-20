PLATFORM?=linux64-debug
DISTDIR?=.

.PHONY: all

RAYLIB_NAME=raylib5-$(PLATFORM)

ifeq ($(PLATFORM), linux64)
EXEC_EXTENSION=
LIB_EXTENSION=.so
CC=gcc
RAYLIB_DLL=-lraylib
CFLAGS+=-O2
CFLAGS+=-D RELEASE
CFLAGS+=-D EXEC_EXTENSION=\"\"
CFLAGS+=-D LIB_EXTENSION=\".so\"
endif

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
endif

PROGRAMS=test_update
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

test_update_SOURCES+=src/../tests/test_update.c

$(DISTDIR)/test_update$(EXEC_EXTENSION): $(test_update_SOURCES)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(DISTDIR)/test_update$(EXEC_EXTENSION)

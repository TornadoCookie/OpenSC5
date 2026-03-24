# Generated using Helium v2.2.0 (https://github.com/tornadocookie/he)

PLATFORM?=linux64
DISTDIR?=build

.PHONY: all

ifeq ($(PLATFORM), win64)
EXEC_EXTENSION=64.exe
LIB_EXTENSION=64.dll
LIB_EXTENSION_STATIC=(null)
CC=x86_64-w64-mingw32-gcc
CFLAGS+=-g
CFLAGS+=-D DEBUG
CFLAGS+=-D EXEC_EXTENSION=\"64.exe\"
CFLAGS+=-D LIB_EXTENSION=\"64.dll\"
LDFLAGS+=Activation.def
endif

ifeq ($(PLATFORM), win32)
EXEC_EXTENSION=.exe
LIB_EXTENSION=.dll
LIB_EXTENSION_STATIC=(null)
CC=i686-w64-mingw32-gcc
CFLAGS+=-g
CFLAGS+=-D DEBUG
CFLAGS+=-D EXEC_EXTENSION=\".exe\"
CFLAGS+=-D LIB_EXTENSION=\".dll\"
LDFLAGS+=Activation.def
endif

ifeq ($(PLATFORM), linux64)
EXEC_EXTENSION=
LIB_EXTENSION=.so
LIB_EXTENSION_STATIC=(null)
CC=gcc
CFLAGS+=-g
CFLAGS+=-D DEBUG
CFLAGS+=-D EXEC_EXTENSION=\"\"
CFLAGS+=-D LIB_EXTENSION=\".so\"
endif

PROGRAMS=decryptdlf
LIBRARIES=

all: $(DISTDIR) $(DISTDIR)/src $(foreach prog, $(PROGRAMS), $(DISTDIR)/$(prog)$(EXEC_EXTENSION)) $(foreach lib, $(LIBRARIES), $(DISTDIR)/$(lib)$(LIB_EXTENSION) $(DISTDIR)/$(lib)$(LIB_EXTENSION_STATIC))
$(DISTDIR)/src:
	mkdir -p $@

$(DISTDIR):
	mkdir -p $@

CFLAGS+=-Isrc
CFLAGS+=-Iinclude
CFLAGS+=-D PLATFORM=\"$(PLATFORM)\"


decryptdlf_SOURCES+=$(DISTDIR)/src/decryptdlf.o
decryptdlf_SOURCES+=$(DISTDIR)/src/aes.o
decryptdlf_SOURCES+=$(DISTDIR)/src/base64.o

$(DISTDIR)/decryptdlf$(EXEC_EXTENSION): $(decryptdlf_SOURCES)
	$(CC) -o $@ $^ $(LDFLAGS)

$(DISTDIR)/%.o: %.c
	$(CC) -c $^ $(CFLAGS) -o $@

clean:
	rm -rf $(DISTDIR)/*

all_dist:
	DISTDIR=$(DISTDIR)/dist/win64 PLATFORM=win64 $(MAKE)
	DISTDIR=$(DISTDIR)/dist/win32 PLATFORM=win32 $(MAKE)
	DISTDIR=$(DISTDIR)/dist/linux64 PLATFORM=linux64 $(MAKE)

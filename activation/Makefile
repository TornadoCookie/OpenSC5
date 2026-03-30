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
CXX=i686-w64-mingw32-g++
CFLAGS+=-g
CFLAGS+=-D DEBUG
CFLAGS+=-D EXEC_EXTENSION=\".exe\"
CFLAGS+=-D LIB_EXTENSION=\".dll\"
LDFLAGS+=EAWebKit.def
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

PROGRAMS=
LIBRARIES=EAWebKit

all: $(DISTDIR) $(DISTDIR)/src $(foreach prog, $(PROGRAMS), $(DISTDIR)/$(prog)$(EXEC_EXTENSION)) $(foreach lib, $(LIBRARIES), $(DISTDIR)/$(lib)$(LIB_EXTENSION) $(DISTDIR)/$(lib)$(LIB_EXTENSION_STATIC))
$(DISTDIR)/src:
	mkdir -p $@

$(DISTDIR):
	mkdir -p $@

CFLAGS+=-Isrc
CFLAGS+=-Iinclude
CFLAGS+=-D PLATFORM=\"$(PLATFORM)\"
CFLAGS+=-I../lib/libEAWebKitd-win32/include


LDFLAGS+=-lstdc++
EAWebKit_CXX_SOURCES+=$(DISTDIR)/src/eawebkit_fake.o

$(DISTDIR)/EAWebKit$(LIB_EXTENSION): $(EAWebKit_SOURCES) $(EAWebKit_CXX_SOURCES)
	$(CC) -o $@ $^ -fPIC -shared $(LDFLAGS)


$(DISTDIR)/EAWebKit$(LIB_EXTENSION_STATIC): $(EAWebKit_SOURCES) $(EAWebKit_CXX_SOURCES)
	$(AR) rcs $@ $^

$(DISTDIR)/%.o: %.c
	$(CC) -c $^ $(CFLAGS) -o $@

$(DISTDIR)/%.o: %.cpp
	$(CXX) -c $^ $(CFLAGS) $(CXXFLAGS) -o $@

clean:
	rm -rf $(DISTDIR)/*

all_dist:
	DISTDIR=$(DISTDIR)/dist/win64 PLATFORM=win64 $(MAKE)
	DISTDIR=$(DISTDIR)/dist/win32 PLATFORM=win32 $(MAKE)
	DISTDIR=$(DISTDIR)/dist/linux64 PLATFORM=linux64 $(MAKE)

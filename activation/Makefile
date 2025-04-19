# Generated using Helium v2.1.1 (https://github.com/tornadocookie/he)

PLATFORM?=win64
DISTDIR?=build

.PHONY: all

ifeq ($(PLATFORM), linux64)
EXEC_EXTENSION=
LIB_EXTENSION=.so
LIB_EXTENSION_STATIC=.a
CC=gcc
CXX=g++
CFLAGS+=-O2
CFLAGS+=-D RELEASE
CFLAGS+=-D EXEC_EXTENSION=\"\"
CFLAGS+=-D LIB_EXTENSION=\".so\"
endif

ifeq ($(PLATFORM), linux64-debug)
EXEC_EXTENSION=-debug
LIB_EXTENSION=.so
LIB_EXTENSION_STATIC=.a
CC=gcc
CXX=g++
CFLAGS+=-g
CFLAGS+=-D DEBUG
CFLAGS+=-D EXEC_EXTENSION=\"-debug\"
CFLAGS+=-D LIB_EXTENSION=\".so\"
endif

ifeq ($(PLATFORM), win64)
EXEC_EXTENSION=.exe
LIB_EXTENSION=.dll
LIB_EXTENSION_STATIC=.a
CC=x86_64-w64-mingw32-gcc
CXX=x86_64-w64-mingw32-g++
CFLAGS+=-O2
CFLAGS+=-D RELEASE
CFLAGS+=-D EXEC_EXTENSION=\".exe\"
CFLAGS+=-D LIB_EXTENSION=\".dll\"
endif

ifeq ($(PLATFORM), web)
EXEC_EXTENSION=.html
LIB_EXTENSION=.a
LIB_EXTENSION_STATIC=.a
CC=emcc
CXX=em++
CFLAGS+=-O2
CFLAGS+=-D RELEASE
CFLAGS+=-D EXEC_EXTENSION=\".html\"
CFLAGS+=-D LIB_EXTENSION=\".a\"
endif

PROGRAMS=
LIBRARIES=Activation64

all: $(DISTDIR) $(DISTDIR)/src $(foreach prog, $(PROGRAMS), $(DISTDIR)/$(prog)$(EXEC_EXTENSION)) $(foreach lib, $(LIBRARIES), $(DISTDIR)/$(lib)$(LIB_EXTENSION) $(DISTDIR)/$(lib)$(LIB_EXTENSION_STATIC))

$(DISTDIR)/src:
	mkdir -p $@

$(DISTDIR):
	mkdir -p $@

CFLAGS+=-Isrc
CFLAGS+=-Iinclude
CFLAGS+=-D PLATFORM=\"$(PLATFORM)\"


Activation64_SOURCES+=$(DISTDIR)/src/activation.o

$(DISTDIR)/Activation64$(LIB_EXTENSION): $(Activation64_SOURCES)
	$(CC) -o $@ $^ -fPIC -shared $(LDFLAGS)


$(DISTDIR)/Activation64$(LIB_EXTENSION_STATIC): $(Activation64_SOURCES)
	$(AR) rcs $@ $^

$(DISTDIR)/%.o: %.c
	$(CC) -c $^ $(CFLAGS) -o $@

clean:
	rm -f $(DISTDIR)/src/activation.o
	rm -f $(DISTDIR)/Activation64$(EXEC_EXTENSION)

all_dist:
	DISTDIR=$(DISTDIR)/dist/linux64 PLATFORM=linux64 $(MAKE)
	DISTDIR=$(DISTDIR)/dist/linux64-debug PLATFORM=linux64-debug $(MAKE)
	DISTDIR=$(DISTDIR)/dist/win64 PLATFORM=win64 $(MAKE)
	DISTDIR=$(DISTDIR)/dist/web PLATFORM=web $(MAKE)

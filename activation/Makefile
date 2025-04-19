# Generated using Helium v2.1.1 (https://github.com/tornadocookie/he)

PLATFORM?=win64
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
endif

PROGRAMS=
LIBRARIES=Activation

all: $(DISTDIR) $(DISTDIR)/src $(foreach prog, $(PROGRAMS), $(DISTDIR)/$(prog)$(EXEC_EXTENSION)) $(foreach lib, $(LIBRARIES), $(DISTDIR)/$(lib)$(LIB_EXTENSION) $(DISTDIR)/$(lib)$(LIB_EXTENSION_STATIC))

$(DISTDIR)/src:
	mkdir -p $@

$(DISTDIR):
	mkdir -p $@

CFLAGS+=-Isrc
CFLAGS+=-Iinclude
CFLAGS+=-D PLATFORM=\"$(PLATFORM)\"

LDFLAGS+=Activation.def

Activation_SOURCES+=$(DISTDIR)/src/activation.o

$(DISTDIR)/Activation$(LIB_EXTENSION): $(Activation_SOURCES)
	$(CC) -o $@ $^ -fPIC -shared $(LDFLAGS)


$(DISTDIR)/Activation$(LIB_EXTENSION_STATIC): $(Activation_SOURCES)
	$(AR) rcs $@ $^

$(DISTDIR)/%.o: %.c
	$(CC) -c $^ $(CFLAGS) -o $@

clean:
	rm -f $(DISTDIR)/src/activation.o
	rm -f $(DISTDIR)/Activation$(EXEC_EXTENSION)

all_dist:
	DISTDIR=$(DISTDIR)/dist/win64 PLATFORM=win64 $(MAKE)
	DISTDIR=$(DISTDIR)/dist/win32 PLATFORM=win32 $(MAKE)

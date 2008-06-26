##  Makefile to build Lua Buit-In program (L-Bia)

# Your version. See VERSIONS for possible values.
VERSION=none

# Where are the things
#  PREFIX is get from the lua directory.
#  PREFIX  = /usr/local
BINDIR  = $(PREFIX)/bin
DATDIR  = $(PREFIX)/share
INCDIR  = $(PREFIX)/include
LIBDIR  = $(PREFIX)/lib
DOCDIR  = $(DATDIR)/doc
# Lua source
SRCDIR  = $(PREFIX)/src/lua-5.1.3/src

# Utilities.
MAKE=make -s
STRIP=strip
UPX=upx
UPXFLAGS=--best
LUA=lua

MYCCFLAGS=
MYLDFLAGS=
MYLIBS=-llua

# Compiler.
CC=gcc
CCFLAGS=$(MYCCFLAGS) -I $(INCDIR) -I $(SRCDIR) -L $(LIBDIR)
LIBS=$(MYLIBS)

# == END OF USER SETTINGS. NO NEED TO CHANGE ANYTHING BELOW THIS LINE =========

# Get PREFIX and EXEEXT
THELUA=$(shell which lua.exe)
ifeq ($(THELUA),)
THELUA=$(shell which lua)
ifeq ($(THELUA),)
$(error You need Lua to compile L-Bia.)
else
EXEEXT=
THEPREFIX=$(dir $(THELUA))
endif
else
EXEEXT=.exe
THEPREFIX=$(dir $(THELUA))
endif
PREFIX=$(THEPREFIX:/bin/=)

THEUPX=$(shell which $(UPX))
ifeq ($(THEUPX),)
UPX=
endif

ifeq ($(EXEEXT),)
LBAUX=lbaux.so
MYLIBS=-llua -lm -ldl
endif
ifeq ($(EXEEXT),.exe)
LBAUX=lbaux.dll
endif

# Convenience versions targets.
VERSIONS=debug release

default: $(VERSION)

none:
	@echo "Please do"
	@echo "   make VERSION"
	@echo "where VERSION is one of these:"
	@echo "   $(VERSIONS)"

TARGET=l-bia$(EXEEXT)
OBJECTS=l-bia.o minilzo.o

all: $(TARGET) $(LBAUX)
	$(LUA) l-bia.lua -i l-bia$(EXEEXT) l-bia.lua --best

$(TARGET): $(OBJECTS)

$(OBJECTS): $(OBJECTS:.o=.c) lbconf.h

clean-obj:
	@echo $(RM) $(OBJECTS) lbaux.o
	@$(RM) $(OBJECTS) lbaux.o

clean-target:
	@echo $(RM) $(TARGET) $(LBAUX)
	@$(RM) $(TARGET) $(LBAUX)

clean: clean-obj clean-target


erase: clean
ifneq ($(EXEEXT),)
	@echo $(RM) *$(EXEEXT)
	@$(RM) *$(EXEEXT)
endif

debug:
	@$(MAKE) all MYCCFLAGS="$(MYCCFLAGS) -g" STRIP="" UPX=""

release:
	@$(MAKE) all MYCCLAGS="$(MYCCFLAGS) -O2 -Wall"

$(LBAUX): lbaux.o minilzo.o
	echo Linking $@...
	$(CC) -shared $(CCFLAGS) $^ $(MYLDFLAGS) $(LIBS) -o $@
ifneq ($(STRIP),)
	$(STRIP) --strip-unneeded $@
endif
ifeq ($(LBAUX),lbaux.dll)
ifneq ($(UPX),)
	$(UPX) $(UPXFLAGS) $@
endif
endif

.PHONY: default none all $(VERSIONS) clean-obj clean-target clean erase

.SUFFIXES: .h .c .o $(EXEEXT)

.o$(EXEEXT):
	echo Linking $@...
	$(CC) $(CCFLAGS) $^ $(MYLDFLAGS) $(LIBS) -o $@
ifneq ($(STRIP),)
	$(STRIP) $@
endif
ifneq ($(UPX),)
	$(UPX) $(UPXFLAGS) $@
endif

.c.o:
	echo Compiling $<...
	$(CC) $(CCFLAGS) -c $<

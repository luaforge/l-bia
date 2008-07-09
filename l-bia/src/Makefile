##  Makefile to build Lua Buit-In program (L-Bia)

# Your version. See VERSIONS for possible values.
VERSION=none

# Where are the things.
#  PREFIX is get from the lua directory.
## PREFIX  = /usr/local
BINDIR  = $(PREFIX)/bin
DATDIR  = $(PREFIX)/share
INCDIR  = $(PREFIX)/include
LIBDIR  = $(PREFIX)/lib
DOCDIR  = $(DATDIR)/doc
# Lua source.
SRCDIR  = $(PREFIX)/src/lua-5.1.3/src

# Utilities.
MAKE=make -s
STRIP=strip
STRIPFLAGS=
UPX=upx
UPXFLAGS=--best
LUA=lua

# Compiler.
CC=gcc
CCFLAGS=$(MYCCFLAGS) -I $(INCDIR) -I $(SRCDIR)
LIBS=$(MYLIBS)

# Compiler flags.
MYCCFLAGS=
MYLDFLAGS=
MYLIBS=

# == END OF USER SETTINGS. NO NEED TO CHANGE ANYTHING BELOW THIS LINE =========

# Get PREFIX and EXEEXT.
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

# Set LBAUX (.so or .dll), STRIPFLAGS and MYLIBS.
ifeq ($(EXEEXT),)
LBAUX=lbaux.so
MYLIBS=-lm -ldl
STRIPFLAGS=--strip-unneeded
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

L_BIA=l-bia$(EXEEXT)
LBDEP=lbconf.h lzoconf.h lzodefs.h minilzo.c minilzo.h

all: $(L_BIA) $(LBAUX)
	$(LUA) l-bia.lua -i $(L_BIA) l-bia.lua

$(L_BIA): l-bia.c $(LBDEP)
	echo Linking $@...
	$(CC) $(CCFLAGS) $< $(MYLDFLAGS) $(LIBS) -o $@
ifneq ($(STRIP),)
	$(STRIP) $@
endif
ifneq ($(UPX),)
	$(UPX) $(UPXFLAGS) $@
endif

$(LBAUX): lbaux.c $(LBDEP)
	echo Linking $@...
	$(CC) -shared $(CCFLAGS) $< -o $@
ifneq ($(STRIP),)
	$(STRIP) $@
endif
ifeq ($(LBAUX),lbaux.dll)
ifneq ($(UPX),)
	$(UPX) $(UPXFLAGS) $@
endif
endif

clean:
	@echo $(RM) $(L_BIA) $(LBAUX)
	@$(RM) $(L_BIA) $(LBAUX)

erase: clean
ifneq ($(EXEEXT),)
	@echo $(RM) *$(EXEEXT)
	@$(RM) *$(EXEEXT)
endif

debug:
	@$(MAKE) all MYCCFLAGS="$(MYCCFLAGS) -g" STRIP="" UPX=""

release:
	@$(MAKE) all MYCCLAGS="$(MYCCFLAGS) -O -Wall"


.PHONY: default none all $(VERSIONS) clean erase

##
##  "$Id: Makefile,v 1.1 2008-06-18 15:53:17 br_lemes Exp $"
##  Makefile to build Lua Buit-In program (L-Bia)
##  A self-running Lua interpreter. Use it to get your Lua program, your
##  C/C++ user code and a Lua interpreter into a single, stand-alone program.
##  Copyright (c) 2007,2008 Breno Ramalho Lemes
##
##  This program is free software; you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation; either version 2 of the License, or
##  (at your option) any later version.

##  This program is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.

##  You should have received a copy of the GNU General Public License along
##  with this program; if not, write to the Free Software Foundation, Inc.,
##  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

##  Breno Ramalho Lemes
##  <br_lemes@yahoo.com.br>
##  http://l-bia.luaforge.net/

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
MYLIBS=-llua -lm

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

clean:
	$(RM) $(OBJECTS) lbaux.o

erase: clean
ifneq ($(EXEEXT),)
	$(RM) *$(EXEEXT)
endif
	$(RM) $(TARGET)
	$(RM) $(LBAUX)

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

.PHONY: all $(VERSIONS) clean none

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

#
#  End of "$Id: Makefile,v 1.1 2008-06-18 15:53:17 br_lemes Exp $".
#

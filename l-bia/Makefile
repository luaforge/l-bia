##  Makefile to build L-Bia binaries, docs and packages

# L-Bia name and version
LB_NAME=l-bia
LB_VERSION=0.1.10

all: release docs

release:
	@$(MAKE) -sC src release

docs:
	@$(MAKE) -sC doc

clean:
	@$(MAKE) -sC src clean
	@$(MAKE) -sC doc clean
	@$(MAKE) -sC doc/htdocs clean

erase:
	@$(MAKE) -sC src erase
	@$(MAKE) -sC doc erase
	@$(MAKE) -sC doc/htdocs erase

# packages
packages: pkg-htdocs pkg-src

LB_HTDOCS=$(LB_NAME)-$(LB_VERSION)-htdocs
LB_HFILES=index.html copying.html history.html lbstyle.css lblogo.png \
          luapowered-white.png gnubanner-2.png t2tpowered-white.png
pkg-htdocs:
	@echo Building $(LB_HTDOCS).zip package...
	@mkdir $(LB_HTDOCS)
	@cd doc/htdocs && $(MAKE) -s && cp $(LB_HFILES) ../../$(LB_HTDOCS)
	@zip -q -9 -r ../$(LB_HTDOCS).zip $(LB_HTDOCS)
	@rm -rf $(LB_HTDOCS)

LB_SRC=$(LB_NAME)-$(LB_VERSION)
LB_SFILES=COPYING HISTORY README README.LZO Makefile doc src
pkg-src: erase
	@echo Building $(LB_SRC).tar.bz2 package...
	@mkdir $(LB_SRC)
	@cp -r $(LB_SFILES) $(LB_SRC)
	@rm -rf $(LB_SRC)/doc/CVS $(LB_SRC)/doc/htdocs/CVS \
                $(LB_SRC)/lib/CVS $(LB_SRC)/src/CVS
	@tar cjf ../$(LB_SRC).tar.bz2 $(LB_SRC)
	@rm -rf $(LB_SRC)

LB_WIN32=$(LB_NAME)-$(LB_VERSION)-win32
LB_WFILES=README.LZO doc/lbdoc.pdf src/l-bia.exe src/hello.lua
pkg-win32: release docs
	@echo Building $(LB_WIN32).zip package...
	@mkdir $(LB_WIN32)
	@cp -r $(LB_WFILES) $(LB_WIN32)
	@zip -q -9 -r ../$(LB_WIN32).zip $(LB_WIN32)
	@rm -rf $(LB_WIN32)

LB_LINUX=$(LB_NAME)-$(LB_VERSION)-linux
LB_LFILES=README.LZO doc/lbdoc.pdf src/l-bia src/hello.lua
pkg-linux: release docs
	@echo Building $(LB_LINUX).zip package...
	@mkdir $(LB_LINUX)
	@cp -r $(LB_LFILES) $(LB_LINUX)
	@tar cjf ../$(LB_LINUX).tar.bz2 $(LB_LINUX)
	@rm -rf $(LB_LINUX)

.PHONY: all release docs clean packages htdocs src win32 erase

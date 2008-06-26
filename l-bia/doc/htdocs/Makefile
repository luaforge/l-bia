##  Makefile to build L-Bia htdocs

TXT2TAGS=txt2tags
HTDOCS=copying.html history.html index.html

all:
	@$(MAKE) -si htdocs

htdocs: $(HTDOCS)

copying.html: copying.t2t ../../COPYING lbtopbar.html
history.html: history.t2t ../../HISTORY lbtopbar.html
index.html: index.t2t ../../README lbtopbar.html

clean:
	@echo $(RM) $(HTDOCS)
	@$(RM) $(HTDOCS)

erase: clean

.PHONY: all htdocs clean erase

.SUFFIXES: .t2t .html

.t2t.html:
	$(TXT2TAGS) -t html $<

##  Makefile to build L-Bia docs and htdocs

TXT2TAGS=txt2tags -t lout
LOUT=lout -PDF

all:
	@$(MAKE) -si docs

docs: lbdoc.pdf

lbdoc.lout: lbdoc.t2t
lbdoc.pdf: lbdoc.lout
	@$(LOUT) -o $@ $^ 2> /dev/null &&\
	$(LOUT) -o $@ $^ 2> /dev/null &&\
	echo lout wrote $@

htdocs:
	@$(MAKE) -C htdocs

clean:
	@echo $(RM) lbdoc.lout lbdoc.lout.ld lout.li
	@$(RM) lbdoc.lout lbdoc.lout.ld lout.li

erase: clean clean-target
clean-target:
	@echo $(RM) lbdoc.pdf
	@$(RM) lbdoc.pdf

.PHONY: all htdocs clean clean-target erase

.SUFFIXES: .t2t .lout

.t2t.lout:
	@$(TXT2TAGS) $<

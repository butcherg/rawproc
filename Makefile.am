AUTOMAKE_OPTIONS = -Wno-portability

dtime="$(shell date +%Y%m%d-%H%M%S)"

SUBDIRS = src
dist_doc_DATA = README.md

.PHONY: makedoc
makedoc:
	zip -r -j src/rawprocdoc.zip $(srcdir)/doc/*

.PHONY: makezip
makezip:
	zip -j rawproc-@VERSION@-$(dtime).zip src/rawproc$(EXEEXT) src/rawprocdoc.zip $(srcdir)/src/rawproc.conf


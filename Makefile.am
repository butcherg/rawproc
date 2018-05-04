SUBDIRS = src
dist_doc_DATA = README.md

.PHONY: makedoc
makedoc:
	zip -r -j src/rawprocdoc.zip $(srcdir)/doc/*

.PHONY: makezip
makezip:
	zip -j rawproc.zip src/rawproc$(EXEEXT) src/rawprocdoc.zip $(srcdir)/src/rawproc.conf
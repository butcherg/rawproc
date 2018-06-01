AUTOMAKE_OPTIONS = -Wno-portability

dtime="$(shell date +%Y%m%d-%H%M%S)"

SUBDIRS = src
dist_doc_DATA = README.md

.PHONY: makedoc
makedoc:
	zip -r -j src/rawprocdoc.zip $(srcdir)/doc/*

.PHONY: makezip
makezip:
	zip -j rawproc-@INNO_SUFFIX@-@VERSION@-$(dtime).zip src/rawproc$(EXEEXT) src/img$(EXEEXT) src/rawprocdoc.zip $(srcdir)/src/rawproc.conf

.PHONY: wininstaller
wininstaller:
	$(srcdir)/inno.sh rawproc-@INNO_SUFFIX@-$(VERSION)-install ./rawproc.iss

.PHONY: appimage
appimage:
	$(srcdir)/make-appimage.pl src/rawproc src/rawprocdoc.zip
	
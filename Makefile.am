AUTOMAKE_OPTIONS = -Wno-portability

dtime=$(shell date +%Y%m%d-%H%M%S)

SUBDIRS = src
dist_doc_DATA = README.md

.PHONY: doc
doc:
	$(srcdir)/doc/confparams.pl $(srcdir)/src > $(srcdir)/doc/confparams.html
	$(srcdir)/doc/rawparams.pl $(srcdir)/src/rawimage.cpp > $(srcdir)/doc/rawparams.html
	zip -r -j src/rawprocdoc.zip $(srcdir)/doc/*

.PHONY: conf
conf:
	$(srcdir)/conffile.pl $(srcdir)/src > src/rawproc.conf

.PHONY: zip
zip:
	zip -j rawproc@VERSION@-@INNO_SUFFIX@-$(dtime).zip src/rawproc$(EXEEXT) src/img$(EXEEXT) src/rawprocdoc.zip src/rawproc.conf

.PHONY: wininstaller
wininstaller:
	$(srcdir)/inno.sh rawproc-@INNO_SUFFIX@-$(VERSION)-install ./rawproc.iss

.PHONY: appimage
appimage:
	$(srcdir)/make-appimage.pl $(VERSION) src/rawproc src/rawprocdoc.zip src/img src/exif
	

AUTOMAKE_OPTIONS = -Wno-portability

dtime=$(shell date +%Y%m%d-%H%M%S)
DATE=$(shell date +%m\\/%d\\/%Y)

SUBDIRS = src
dist_doc_DATA = README.md

.PHONY: doc
doc:
	zip -r -j src/rawprocdoc.zip $(srcdir)/doc/*.html $(srcdir)/doc/*.css $(srcdir)/doc/*.png $(srcdir)/doc/*.hh*
	
.PHONY: docpages
docpages:
	sed -e 's/#VERSION#/$(VERSION)/' -e 's/#DATE#/$(DATE)/'  $(srcdir)/doc/main.html.in > $(srcdir)/doc/main.html
	$(srcdir)/doc/confparams.pl $(srcdir)/src > $(srcdir)/doc/confparams.html
	$(srcdir)/doc/rawparams.pl $(srcdir)/src/rawimage.cpp > $(srcdir)/doc/rawparams.html
	$(srcdir)/doc/img.pl $(srcdir)/src/img.cpp > $(srcdir)/doc/img.html

.PHONY: conf
conf:
	$(srcdir)/conffile.pl $(srcdir)/src > src/rawproc.conf

.PHONY: zip
zip:
	zip -j rawproc-@VERSION@-@INNO_SUFFIX@.zip src/rawproc$(EXEEXT) src/img$(EXEEXT) src/rawprocdoc.zip src/rawproc.conf src/dcraw.c src/camconst.json

.PHONY: wininstaller
wininstaller:
	$(srcdir)/inno.sh rawproc-@INNO_SUFFIX@-$(VERSION)-install ./rawproc.iss

.PHONY: appimage
appimage:
	$(srcdir)/make-appimage.pl $(VERSION) src/rawproc src/rawprocdoc.zip src/img src/exif src/dcraw.c src/camconst.json


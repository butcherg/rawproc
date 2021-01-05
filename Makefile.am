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
	$(srcdir)/doc/img.pl $(srcdir)/src/gimage_parse.cpp > $(srcdir)/doc/img.html
	$(srcdir)/doc/index.pl $(srcdir)/doc > $(srcdir)/doc/index.html

.PHONY: conf
conf:
	rm -f src/rawproc.conf src/dcraw.c src/camconst.json
	$(srcdir)/conffile.pl $(srcdir)/src > src/rawproc.conf
	wget -P src https://raw.githubusercontent.com/Beep6581/RawTherapee/dev/rtengine/camconst.json 
	wget -P src https://raw.githubusercontent.com/Beep6581/RawTherapee/dev/rtengine/dcraw.c
	wget -P src http://lensfun.sourceforge.net/db/@LENSFUN_DATABASE_VERSION@.tar.bz2
	mkdir -p src/@LENSFUN_DATABASE_VERSION@
	tar -xjvf src/@LENSFUN_DATABASE_VERSION@.tar.bz2 -C src/@LENSFUN_DATABASE_VERSION@
	echo "0" > src/@LENSFUN_DATABASE_VERSION@/timestamp.txt
	

.PHONY: zip
zip:
	zip -j rawproc-@VERSION@-@INNO_SUFFIX@.zip src/rawproc$(EXEEXT) src/img$(EXEEXT) src/wxcmd$(EXEEXT) src/rawprocdoc.zip src/rawproc.conf src/dcraw.c src/camconst.json

.PHONY: wininstaller
wininstaller:
	$(srcdir)/inno.sh rawproc-@INNO_SUFFIX@-$(VERSION)-install ./rawproc.iss

.PHONY: appimage
appimage:
	$(srcdir)/make-appimage.pl $(VERSION) BIN:src/rawproc BIN:src/rawprocdoc.zip BIN:src/img BIN:src/exif BIN:src/wxcmd "BIN:$(srcdir)/src/icon.xpm" "BIN:$(srcdir)/rawproc.appdata.xml" DATA:src/dcraw.c DATA:src/camconst.json DATA:src/rawproc.conf DATA:src/@LENSFUN_DATABASE_VERSION@


## Need 1.7 for subdir support.  (Well, 1.4 doesn't work, I don't know if anything between does.)
AUTOMAKE_OPTIONS = 1.7 foreign
SUBDIRS = src
ACLOCAL_AMFLAGS = -I autoconf/m4

## Black magic (read: sed) for getting the product ID as defined in code, not by the autotools.
## Quotes are making vim's highlighting of this even worse, so just escape everything,
## including the escapes. Make it lowercase to match ArchHooks::MountInitialFileSystems().
productID := $(shell sed -nr /define\\s+PRODUCT_ID_BARE/\{s/.*define\\s+PRODUCT_ID_BARE\\s+\(.+\)/\\1/\;y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/\;p\;q\} $(srcdir)/src/ProductInfo.h)

installFiles = src/stepmania
if HAVE_GTK
installFiles += src/GtkModule.so
endif

.PHONY: SMData
SMData:
	$(srcdir)/Utils/mksmdata.pl $(srcdir) .

dist-hook: SMData
	mkdir -p $(distdir)/Packages
	cp SMData.smzip $(distdir)/Packages
	mkdir -p $(distdir)/Docs
	cp $(srcdir)/Docs/Licenses.txt $(distdir)/Docs
	cp $(srcdir)/Utils/build.sh $(distdir)

install-data-hook: $(top_srcdir)/Packages/SMData.smzip
	mkdir -p "$(DESTDIR)$(bindir)/$(productID)/Packages"
	$(INSTALL) $(installFiles) "$(DESTDIR)$(bindir)/$(productID)"
	$(INSTALL_DATA) $(top_srcdir)/Packages/SMData.smzip "$(DESTDIR)$(bindir)/$(productID)/Packages"

uninstall-hook:
	rm -f "$(DESTDIR)$(bindir)/$(productID)/stepmania"
	rm -f "$(DESTDIR)$(bindir)/$(productID)/GtkModule.so"
	rm -f "$(DESTDIR)$(bindir)/$(productID)/Packages/SMData.smzip"

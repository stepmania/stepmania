## Need 1.7 for subdir support.  (Well, 1.4 doesn't work, I don't know if anything between does.)
AUTOMAKE_OPTIONS = 1.7 foreign
SUBDIRS = src
ACLOCAL_AMFLAGS = -I autoconf/m4

## Black magic (read: sed) for getting the product ID as defined in code, not by the autotools.
## Quotes are making vim's highlighting of this even worse, so just escape everything,
## including the escapes. Make it lowercase to match ArchHooks::MountInitialFileSystems().
productID := $(shell sed -nr /define\\s+PRODUCT_ID_BARE/\{s/.*define\\s+PRODUCT_ID_BARE\\s+\(.+\)/\\1/\;y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/\;p\;q\} $(srcdir)/src/ProductInfo.h)

.PHONY: SMData
SMData:
	$(srcdir)/Utils/mksmdata.pl $(srcdir) .

dist-hook: SMData
	cp SMData.smzip $(distdir)
	mkdir -p $(distdir)/Docs
	cp $(srcdir)/Docs/BGAnimation\ conditionals.txt $(srcdir)/Docs/BMA-fmt.txt \
		$(srcdir)/Docs/ConditionalBGA\ Info.txt $(srcdir)/Docs/Copying.MAD \
		$(srcdir)/Docs/Licenses.txt $(distdir)/Docs
	cp $(srcdir)/Utils/build.sh $(distdir)

install-data-hook: $(top_srcdir)/SMData.smzip
	mkdir -p "$(DESTDIR)$(datadir)/$(productID)"
	$(INSTALL_DATA) $(top_srcdir)/SMData.smzip "$(DESTDIR)$(datadir)/$(productID)"

uninstall-hook:
	rm -f "$(DESTDIR)$(datadir)/$(productID)/SMData.smzip"

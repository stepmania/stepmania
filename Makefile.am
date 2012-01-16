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

installData = $(srcdir)/Announcers $(srcdir)/BGAnimations $(srcdir)/Themes $(srcdir)/Characters $(srcdir)/Scripts $(srcdir)/Courses $(srcdir)/BackgroundEffects $(srcdir)/Data $(srcdir)/BackgroundTransitions $(srcdir)/Docs $(srcdir)/NoteSkins

.PHONY: SMData
SMData:
	$(srcdir)/Utils/CreatePackage.pl $(srcdir) .

dist-hook: SMData
	mkdir -p "$(distdir)/Packages"
	mv GameData.smzip "$(distdir)/Packages"
	cp -r "$(srcdir)/Docs" "$(distdir)"
	cp -r -t "$(distdir)" $(installFiles)

install-exec-hook:
	mkdir -p "$(prefix)/$(productID)"
	$(INSTALL) $(installFiles) "$(prefix)/$(productID)"

install-data-local:
	cp -r -t "$(prefix)/$(productID)" $(installData)
	mkdir -p "$(prefix)/$(productID)/Songs"

uninstall-hook:
	rm -f "$(prefix)/$(productID)/stepmania"
	rm -f "$(prefix)/$(productID)/GtkModule.so"
# todo: properly remove data

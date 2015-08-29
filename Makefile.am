## Need 1.7 for subdir support.  (Well, 1.4 doesn't work, I don't know if anything between does.)
AUTOMAKE_OPTIONS = 1.7 foreign
SUBDIRS = bundle src
ACLOCAL_AMFLAGS = -I autoconf/m4

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
	mkdir -p "$(DESTDIR)$(prefix)/stepmania-$(VERSION)"
	$(INSTALL) $(installFiles) "$(DESTDIR)$(prefix)/stepmania-$(VERSION)"

install-data-local:
	mkdir -p "$(DESTDIR)$(prefix)/stepmania-$(VERSION)/Songs"
	cp -r -t "$(DESTDIR)$(prefix)/stepmania-$(VERSION)" $(installData)

uninstall-hook:
	rm -f "$(DESTDIR)$(prefix)/stepmania-$(VERSION)/stepmania"
	rm -f "$(DESTDIR)$(prefix)/stepmania-$(VERSION)/GtkModule.so"
# todo: properly remove data

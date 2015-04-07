# still nothing
EXTRA_DIST = nopoll.doxygen \
	     nopoll-doc-footer.html \
	     nopoll-doc-header.html \
	     main-page-logo.png \
	     aspl-logo-header.png \
	     nopoll.css \
	     body_back.gif \
	     hacha-100x171.png

bin_SCRIPTS = initial_build_doc

help:
	@echo 
	@echo " Documentation not build!:"
	@echo " ~~~~~~~~~~~~~~~~~~~~~~~~~"
	@echo " --> Doxygen documentation wasn't built because an stamp was detected: doc.stamp"
	@echo " --> To regenerate or force doc building type make build_doc or remove doc.stamp";
	@echo

initial_build_doc:
	@echo "checking documenation stamp..";
	@test -e doc.stamp & make help
	@test -e doc.stamp || make build_doc
	@test -e doc.stamp || touch doc.stamp

build_doc:
	rm -rf html/
	@echo "Generating module documentation..";
	doxygen nopoll.doxygen
	cp $(top_srcdir)/doc/main-page-logo.png html
	cp $(top_srcdir)/doc/aspl-logo-header.png html
	cp $(top_srcdir)/doc/nopoll.css html
	cp $(top_srcdir)/doc/body_back.gif html
	cp $(top_srcdir)/doc/hacha-100x171.png html

-include ../config.mak
-include ../defs.mak

NAME=lsvmtools

DEBDIR=$(NAME)-$(VERSION)
TARBALL=/tmp/$(DEBDIR)-x86_64.tar.gz
BUILDFILES=debian

all:
	$(MAKE) -s clean
	$(MAKE) -s configure
	$(MAKE) -s sources
	$(MAKE) -s get-build-deps
	$(MAKE) -s build

clean:
	rm -rf $(DEBDIR)
	rm -rf $(TARBALL)
	rm -rf lsvmtools_*

configure:
	@( cd ..; ./configure > /dev/null )

sources:
	@$(MAKE) -s -C .. bindist > /dev/null 
	@mkdir $(DEBDIR)
	@tar xf $(TARBALL) -C $(DEBDIR)
	@rm -rf $(DEBDIR)/$(NAME)-$(VERSION)-x86_64/install

get-build-deps:
	@apt-get -qq -y install build-essential dh-make devscripts 

build:
	@cp -r $(BUILDFILES) $(DEBDIR)
	@(cd $(DEBDIR) && yes | debuild -us -uc)

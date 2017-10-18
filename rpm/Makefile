-include ../config.mak
-include ../defs.mak

NAME=lsvmtools

TARBALL=$(NAME)-$(VERSION).tar.gz

all:
	$(MAKE) -s clean
	$(MAKE) -s configure
	$(MAKE) -s sources
	$(MAKE) -s build

clean:
	rm -rf SOURCES/*
	rm -rf ./RPMS/x86_64

configure:
	@ ( cd ..; ./configure > /dev/null )

sources:
	@ $(MAKE) -s -C .. dist > /dev/null 
	@ cp ../$(TARBALL) SOURCES
	@ echo "Created SPECS/$(TARBALL)"

build:
	rpmbuild -v -bb --clean SPECS/lsvmtools.spec
	@ cp RPMS/x86_64/lsvmtools-1.0.0-1.x86_64.rpm .
	@ echo "Created lsvmtools-1.0.0-1.x86_64.rpm"

include ../../defs.mak

PKGNAME=zlib-1.2.11
URL=http://zlib.net/$(PKGNAME).tar.gz

all: dirs download
	$(MAKE) -f linux.mak
	$(MAKE) -f efi.mak

clean:
	$(MAKE) -f linux.mak clean
	$(MAKE) -f efi.mak clean

depend: dirs download
	$(MAKE) -f linux.mak depend
	$(MAKE) -f efi.mak depend

distclean: clean
	rm -rf zlib
	rm -f depend.mak

install:

dirs:
	mkdir -p $(TOP)/build/obj/zliblinux/zlib
	mkdir -p $(TOP)/build/obj/zlibefi/zlib


##==============================================================================
##
## Download zlib distro:
##
##==============================================================================

download: ./zlib/Makefile

./zlib/Makefile:
	wget $(URL) -O - | tar zxvf -
	mv $(PKGNAME) zlib

##==============================================================================
##
## Test:
##
##==============================================================================

testzlib:
	gcc -o $(BINDIR)/testzlib testzlib.c -lzliblinux -lposixlinux -L $(LIBDIR)
	$(BINDIR)/./testzlib


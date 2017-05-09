include ../../defs.mak

export PKGVERSION=5.2.3
PKGNAME=xz-$(PKGVERSION)
URL=https://sourceforge.net/projects/lzmautils/files/$(PKGNAME).tar.gz/download

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
	rm -rf xz
	rm -f depend.mak

install:

dirs:
	@ mkdir -p $(TOP)/build/obj/lzmalinux/xz
	@ mkdir -p $(TOP)/build/obj/lzmalinux/xz/src/liblzma/rangecoder
	@ mkdir -p $(TOP)/build/obj/lzmalinux/xz/src/liblzma/simple
	@ mkdir -p $(TOP)/build/obj/lzmalinux/xz/src/liblzma/delta
	@ mkdir -p $(TOP)/build/obj/lzmalinux/xz/src/liblzma/lzma
	@ mkdir -p $(TOP)/build/obj/lzmalinux/xz/src/liblzma/check
	@ mkdir -p $(TOP)/build/obj/lzmalinux/xz/src/liblzma/lz
	@ mkdir -p $(TOP)/build/obj/lzmalinux/xz/src/liblzma/common
	@ mkdir -p $(TOP)/build/obj/lzmaefi/xz
	@ mkdir -p $(TOP)/build/obj/lzmaefi/xz/src/liblzma/rangecoder
	@ mkdir -p $(TOP)/build/obj/lzmaefi/xz/src/liblzma/simple
	@ mkdir -p $(TOP)/build/obj/lzmaefi/xz/src/liblzma/delta
	@ mkdir -p $(TOP)/build/obj/lzmaefi/xz/src/liblzma/lzma
	@ mkdir -p $(TOP)/build/obj/lzmaefi/xz/src/liblzma/check
	@ mkdir -p $(TOP)/build/obj/lzmaefi/xz/src/liblzma/lz
	@ mkdir -p $(TOP)/build/obj/lzmaefi/xz/src/liblzma/common

##==============================================================================
##
## Download xz distro:
##
##==============================================================================

download: ./xz/configure.ac

./xz/configure.ac:
	wget $(URL) -O - | tar zxvf -
	mv $(PKGNAME) xz

##==============================================================================
##
## Tests:
##
##==============================================================================

testxz:
	gcc -o $(BINDIR)/testxz testxz.c -llzmalinux -lposixlinux -L $(LIBDIR)
	$(BINDIR)/./testxz


include ./defs.mak

.PHONY: lsvmutils lsvmtool posix 3rdparty lsvmload grub configure bindist install striplic

##==============================================================================
##
## configure:
##
##==============================================================================

ifndef CONFIGURED
$(error "Type ./configure to configure the build system")
endif

##==============================================================================
##
## all:
##
##==============================================================================

define NL


endef

DIRS = striplic 3rdparty posix lsvmutils lsvmtool lsvmload policy

all:
	$(foreach i, $(DIRS), $(MAKE) -C $(i) $(NL) )

##==============================================================================
##
## clean:
##
##==============================================================================

clean: 
	$(foreach i, $(DIRS), $(MAKE) -C $(i) clean $(NL) )
	rm -rf build lsvm.tar.gz
	rm -rf build scripts/ubuntu/blkdev_utils

##==============================================================================
##
## prep:
##
##==============================================================================

prep:
	@ ./lsvmprep

##==============================================================================
##
## shimx64:
##
##==============================================================================

shimentry:
	efibootmgr -c -g -d /dev/sda -p 1 -w -L shim -l "\EFI\ubuntu\shimx64.efi"

##==============================================================================
##
## rescue:
##
##==============================================================================

rescue:
	rm -rf /boot/efi/EFI/rescue
	cp -r /boot/efi/EFI/boot /boot/efi/EFI/rescue
	cp /boot/efi/EFI/boot/bootx64.efi /boot/efi/EFI/rescue/rescuex64.efi
	efibootmgr -c -g -d /dev/sda -p 1 -w -L rescue -l "\EFI\rescue\rescuex64.efi"

##==============================================================================
##
## distclean:
##
##==============================================================================

distclean:
	- $(MAKE) clean
	$(foreach i, $(DIRS), $(MAKE) -C $(i) distclean $(NL) )
	rm -f config.mak
	rm -rf build

##==============================================================================
##
## bindist:
##
##==============================================================================

BINPKGNAME=lsvmtools-$(VERSION)-x86_64
BINDIRNAME=/tmp/$(BINPKGNAME)

bindist: all
	@ rm -rf $(BINDIRNAME)
	@ mkdir -p $(BINDIRNAME)
	@ cp -r build $(BINDIRNAME)/build
	@ rm -rf $(BINDIRNAME)/build/tmp
	@ cp -f dbxupdate.bin $(BINDIRNAME)/
	@ rm -rf $(BINDIRNAME)/build/obj
	@ rm -rf $(BINDIRNAME)/build/lib
	@ rm -f $(BINDIRNAME)/build/bin/lsvmcpio
	@ rm -f $(BINDIRNAME)/build/bin/vfat
	@ rm -f $(BINDIRNAME)/build/bin/ext2
	@ cp -r scripts $(BINDIRNAME)/scripts
	@ rm -rf $(BINDIRNAME)/scripts/dev
	@ cp -r policy $(BINDIRNAME)/policy
	@ rm -rf $(BINDIRNAME)/policy/attic
	@ rm -rf $(BINDIRNAME)/policy/Makefile
ifdef RELEASE
	install -D lsvmload/lsvmload.signed.efi $(BINDIRNAME)/lsvmload/lsvmload.efi
else
	install -D lsvmload/lsvmload.efi $(BINDIRNAME)/lsvmload/lsvmload.efi
endif
	@ cp lsvmprep $(BINDIRNAME)/lsvmprep
	@ cp sanity $(BINDIRNAME)/sanity
	@ cp VERSION $(BINDIRNAME)/VERSION
	@ cp LICENSE $(BINDIRNAME)/LICENSE
	@ cp scripts/install $(BINDIRNAME)/install
	@ ( cd /tmp; tar zcf $(BINPKGNAME).tar.gz $(BINPKGNAME) )
	@ rm -rf $(BINDIRNAME)
	@ echo "Created /tmp/$(BINPKGNAME).tar.gz"

##==============================================================================
##
## dist:
##
##==============================================================================

SRCPKGNAME=lsvmtools-$(VERSION)
SRCDIRNAME=/tmp/$(SRCPKGNAME)

DISTEXCLUDE = \
    $(SRCDIRNAME)/.git \
    $(SRCDIRNAME)/.gitignore \
    $(SRCDIRNAME)/TDC \
    $(SRCDIRNAME)/vhdxparse \
    $(SRCDIRNAME)/lsvmutils/attic \
    $(SRCDIRNAME)/lsvmload/attic \
    $(SRCDIRNAME)/lsvmtool/attic \
    $(SRCDIRNAME)/crypto/attic \
    $(SRCDIRNAME)/policy/attic \
    $(SRCDIRNAME)/vhdxparse/attic \
    $(SRCDIRNAME)/scripts/ubuntu/attic \
    $(SRCDIRNAME)/scripts/attic \
    $(SRCDIRNAME)/attic \
    $(SRCDIRNAME)/attic/Phase2Agent/attic \
    $(SRCDIRNAME)/attic/crypto/attic


dist:
	@ rm -rf $(TOP)/$(SRCPKGNAME).tar.gz
	@ rm -rf $(SRCDIRNAME)
	@ cp -r $(TOP) $(SRCDIRNAME)
	@ rm -rf $(DISTEXCLUDE)
	@ ( cd $(SRCDIRNAME); $(MAKE) -s distclean )
	@ ( cd /tmp; tar zcf $(TOP)/$(SRCPKGNAME).tar.gz $(SRCPKGNAME) )
	@ echo "Created $(TOP)/$(SRCPKGNAME).tar.gz"

##==============================================================================
##
## install:
##
##==============================================================================

install: lsvmload
	$(MAKE) bindist
	rm -rf $(BINDIRNAME)
	( cd /tmp; tar zxf $(BINPKGNAME).tar.gz )
	( cd $(BINDIRNAME); destdir=$(DESTDIR) ./install )

ifdef RELEASE
lsvmload: ./lsvmload/lsvmload.signed.efi
else
lsvmload:
endif

./lsvmload/lsvmload.signed.efi:
	@ echo ""
	@ echo "*** Error: ./lsvmload/lsvmload.signed.efi not found"
	@ echo ""
	@ exit 1

##==============================================================================
##
## test-install:
##
##==============================================================================

test-install:
	$(MAKE) install
	( cd /opt/$(SRCPKGNAME); ./lsvmprep )


##==============================================================================
##
## grubby:
##
##==============================================================================

grubby:
	rm -f /etc/grub2-efi.cfg
	ln -s /boot/grub2/grub.cfg /etc/grub2-efi.cfg
	grubby --update-kernel=ALL --config-file=/etc/grub2-efi.cfg

tests:
	$(MAKE) -s -C lsvmtool tests

check: tests

##==============================================================================
##
## striplic:
##
##==============================================================================

ALLSRCFILES=$(shell find . -name '*.[ch]')

striplic:
	@ $(BINDIR)/striplic $(ALLSRCFILES)

prependlic:
	./scripts/prependlic

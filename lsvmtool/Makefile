.PHONY: bootkey
.PHONY: rootkey
.PHONY: oldrootkey
.PHONY: sample
.PHONY: ext2fs
.PHONY: luksfs 

include ../defs.mak

DIRECTORY=lsvmtool

LSVMCPIO = $(BINDIR)/lsvmcpio
EXT2 = $(BINDIR)/ext2
VFAT = $(BINDIR)/vfat
LSVMTOOL = $(BINDIR)/lsvmtool

##==============================================================================
##
## lsvmtool program:
##
##==============================================================================

USE_XZ=1

DEFINES += "-DLSVMTOOL_VERSION=\"$(VERSION)\""

INCLUDES += -I..
INCLUDES += -I/usr/include/efi
INCLUDES += -I/usr/include/efi/x86_64
INCLUDES += -I$(TOP)/3rdparty
INCLUDES += -I$(TOP)/3rdparty/zlib
INCLUDES += -I$(TOP)/3rdparty/zlib/zlib
INCLUDES += -I$(TOP)/3rdparty/openssl/linux/$(OPENSSLPACKAGE)/include

CFLAGS = -c -Wall -g -O3
CFLAGS += -Werror
CFLAGS += $(LINUX_CFLAGS)

SOURCES += main.c 
SOURCES += ext2main.c
SOURCES += lsvmtoolmain.c
SOURCES += cpiomain.c
SOURCES += cpio.c
SOURCES += vfatmain.c
SOURCES += logo.c
SOURCES += specialize.c
SOURCES += dbxupdate.c

OBJECTS = $(SOURCES:.c=.o)

LIBDEPENDS += $(LIBDIR)/liblsvmutils.a
LIBDEPENDS += $(LIBDIR)/libzliblinux.a
LIBDEPENDS += $(LIBDIR)/liblzmalinux.a

LIBRARIES += -L$(TOP)/build/lib -llsvmutils -lcryptolinux
LIBRARIES += -ldl
LIBRARIES += -lcrypt
LIBRARIES += -lzliblinux
LIBRARIES += -llzmalinux
LIBRARIES += -lposixlinux

#LINKFLAGS = -static
LINKFLAGS = -Wl,-gc-sections

AUTOTESTS=$(TMPDIR)/autotests

CLEAN += $(EXT2)
CLEAN += $(VFAT)
CLEAN += $(LSVMCPIO)
CLEAN += initrd.cpio 
CLEAN += seal.in 
CLEAN += seal.out 
CLEAN += unseal.out 
CLEAN += encrypt.in 
CLEAN += encrypt.out 
CLEAN += decrypt.out 
CLEAN += boot.cfg
CLEAN += lsvmhashes
CLEAN += initrd.cpio 
CLEAN += initrd.cpio.gz 
CLEAN += $(AUTOTESTS)

PROGRAM=lsvmtool

include ../rules.mak

distclean: clean
	rm -f depend.mak

done: $(EXT2) $(VFAT) $(LSVMCPIO)

$(EXT2): $(LSVMTOOL)
	ln -s $(LSVMTOOL) $(EXT2)

$(VFAT): $(LSVMTOOL)
	ln -s $(LSVMTOOL) $(VFAT)

$(LSVMCPIO): $(LSVMTOOL)
	ln -s $(LSVMTOOL) $(LSVMCPIO)

##==============================================================================
##
## logo.c:
##
##==============================================================================

logo:
	@ figlet "LSVMTool" > LOGO
	@ $(BINDIR)/lsvmtool cencode --name g_logo --zeroterminate LOGO > logo.c
	@ echo "Created logo.c"

##==============================================================================
##
## tests:
##
##==============================================================================

lsvmtool-tests: top
	$(LSVMTOOL) tests
	@ echo "=== $@ passed"

##==============================================================================
##
## test-seal:
##
##==============================================================================

test-seal:
	echo -n "ABCDEFGHIJKLMNOPQRSTUVWXYZ" > $(TMPDIR)/seal.in
	$(LSVMTOOL) seal $(TMPDIR)/seal.in $(TMPDIR)/seal.out
	$(LSVMTOOL) unseal $(TMPDIR)/seal.out $(TMPDIR)/unseal.out
	@ echo "=== $@ passed"

##==============================================================================
##
## sealrootkey:
##
##==============================================================================

LUKSDEV=/dev/sda3
SHIMPATH = /boot/efi/EFI/boot/bootx64.efi
ROOTKEYPATH = /boot/efi/EFI/boot/rootkey
FBKEYPATH = /boot/efi/EFI/boot/fbkey
LSVMHASHESPATH = /boot/efi/EFI/boot/lsvmhashes

##==============================================================================
##
## cpio-tests:
##
##==============================================================================

CPIODIR=$(TMPDIR)/cpiodir

SAMPLECPIO=$(TMPDIR)/sample.cpio

cpio-sample:
	@ mkdir -p $(TMPDIR)
	@ ( cd sample; find . | cpio --create --format='newc' > $(SAMPLECPIO) 2> /dev/null )

cpio-test-add: cpio-sample
	@ $(LSVMCPIO) add $(SAMPLECPIO) dir1/Makefile Makefile 
	@ rm -rf $(CPIODIR)
	@ mkdir -p $(CPIODIR)
	@ ( cd $(CPIODIR); cpio -i < $(SAMPLECPIO) 2> /dev/null )
	@ cmp Makefile $(CPIODIR)/dir1/Makefile
	@ echo "=== $@ passed"

cpio-test-remove: cpio-sample
	@ $(LSVMCPIO) remove $(SAMPLECPIO) dir1/readme.txt
	@ echo "=== $@ passed"

cpio-tests: top cpio-test-add cpio-test-remove 
	@ echo "=== $@ passed"

##==============================================================================
##
## ext2-tests:
##
##==============================================================================

ifndef RANDFILE
RANDFILE=randfile
endif

randfile:
	@ head -c 1m /dev/urandom > $(TMPDIR)/$(RANDFILE).in

export EXT2FS=$(TMPDIR)/ext2fs

ext2fs-cleanup:
	-@ umount /mnt/ext2fs 2> /dev/null
	-@ losetup -d /dev/loop0 2> /dev/null
	-@ rm -f $(EXT2FS)

ext2fs: 
	$(MAKE) -s ext2fs-cleanup
	head -c 100M /dev/zero > $(EXT2FS)
	mkdir -p /mnt/ext2fs
	losetup /dev/loop0 $(EXT2FS)
	mke2fs $(MKE2FSFLAGS) -j /dev/loop0 2> /dev/null > /dev/null
	echo mke2fs $(MKE2FSFLAGS) -j /dev/loop0 2> /dev/null > /dev/null
	mount /dev/loop0 /mnt/ext2fs

ext2-dump:
	@ $(EXT2) dump

ext2-check:
	@ $(EXT2) check
	@ echo "=== $@ passed"

ext2-lsr:
	@ $(EXT2) lsr / | sort > $(TMPDIR)/file1
	@ echo "/lost+found" > $(TMPDIR)/file2
	@ cmp $(TMPDIR)/file1 $(TMPDIR)/file2
	@ echo "=== $@ passed"

ext2-mkdir: ext2fs
	@ $(EXT2) mkdir /newdir
	@ echo "=== $@ passed"

ext2-putgetrm:
	@ $(MAKE) randfile
	@ $(EXT2) put $(TMPDIR)/$(RANDFILE).in /$(RANDFILE)
	@ $(EXT2) get /$(RANDFILE) $(TMPDIR)/$(RANDFILE).out
	@ $(EXT2) rm /$(RANDFILE)
	@ cmp $(TMPDIR)/$(RANDFILE).in $(TMPDIR)/$(RANDFILE).out
	@ echo "=== $@ RANDFILE=$(RANDFILE) passed"

ext2-cleanup:
	rm -f $(EXT2FS)

ext2-tests: top ext2fs 
	@ $(MAKE) ext2-check 
	@ $(MAKE) ext2-lsr 
	@ $(MAKE) ext2-mkdir 
	@ $(MAKE) ext2-putgetrm 
	@ $(MAKE) ext2-putgetrm RANDFILE=randfile01
	@ $(MAKE) ext2-putgetrm RANDFILE=randfile02
	@ $(MAKE) ext2-putgetrm RANDFILE=randfile03
	@ $(MAKE) ext2-putgetrm RANDFILE=randfile04
	@ $(MAKE) ext2-putgetrm RANDFILE=randfile05
	@ $(MAKE) ext2-putgetrm RANDFILE=randfile06
	@ $(MAKE) ext2-putgetrm RANDFILE=randfile07
	@ $(MAKE) ext2-putgetrm RANDFILE=randfile08
	@ $(MAKE) ext2-putgetrm RANDFILE=randfile09
	@ $(MAKE) ext2-putgetrm RANDFILE=randfile10
	@ $(MAKE) ext2-cleanup

##==============================================================================
##
## xz-tests:
##
##==============================================================================

xz-tests: randfile
	@ $(LSVMTOOL) xz $(TMPDIR)/$(RANDFILE).in $(TMPDIR)/$(RANDFILE).xz
	@ $(LSVMTOOL) unxz $(TMPDIR)/$(RANDFILE).xz $(TMPDIR)/$(RANDFILE).out
	@ cmp $(TMPDIR)/$(RANDFILE).in $(TMPDIR)/$(RANDFILE).out
	@ echo "=== $@ passed"

testxz:
	@ $(LSVMTOOL) testxz lsvmtoolmain.c
	@ echo "=== $@ passed"

##==============================================================================
##
## gzip-tests:
##
##==============================================================================

gzip-tests: randfile
	@ $(LSVMTOOL) gzip $(TMPDIR)/$(RANDFILE).in $(TMPDIR)/$(RANDFILE).gz
	@ $(LSVMTOOL) gunzip $(TMPDIR)/$(RANDFILE).gz $(TMPDIR)/$(RANDFILE).out
	@ cmp $(TMPDIR)/$(RANDFILE).in $(TMPDIR)/$(RANDFILE).out
	@ echo "=== $@ passed"

testzlib:
	@ $(LSVMTOOL) testzlib lsvmtoolmain.c
	@ echo "=== $@ passed"

##==============================================================================
##
## strtok-tests:
##
##==============================================================================

strtok-tests:
	@ $(LSVMTOOL) strtok /a/b/c > $(TMPDIR)/strtok.1
	@ rm -f $(TMPDIR)/strtok.2
	@ echo "a" >> $(TMPDIR)/strtok.2
	@ echo "b" >> $(TMPDIR)/strtok.2
	@ echo "c" >> $(TMPDIR)/strtok.2
	@ cmp $(TMPDIR)/strtok.1 $(TMPDIR)/strtok.2
	@ echo "=== $@ passed"

##==============================================================================
##
## test-crypt:
##
##==============================================================================

PRIME=15485867
KEY=CEA53153C8283BD2FF4D3BF8D6221D08F082ED38A3B166E383B0DD5E97A40712
KEYFILE=$(TMPDIR)/keyfile
PLAINTEXTFILE=$(TMPDIR)/plaintext
ENCFILE=$(TMPDIR)/encryptfile
DECFILE=$(TMPDIR)/decryptfile

plaintext:
	head -c $(PRIME) /dev/urandom > $(PLAINTEXTFILE)

keyfile:
	@ $(LSVMTOOL) hextobin $(KEY) $(KEYFILE)

test-crypt: plaintext keyfile
	@ $(LSVMTOOL) encrypt --keyfile $(KEYFILE) $(PLAINTEXTFILE) $(ENCFILE)
	@ $(LSVMTOOL) decrypt --keyfile $(KEYFILE) $(ENCFILE) $(DECFILE)
	@ cmp $(PLAINTEXTFILE) $(DECFILE)
	@ echo "=== $@ passed"

##==============================================================================
##
## sha-tests:
##
##==============================================================================

ALPHABET=$(TMPDIR)/alphabet
SHA1_ALPHABET=80256F39A9D308650AC90D9BE9A72A9562454574
SHA256_ALPHABET=D6EC6898DE87DDAC6E5B3611708A7AA1C2D298293349CC1A6C299A1DB7149D38

sha1-tests:
	@ echo -n "ABCDEFGHIJKLMNOPQRSTUVWXYZ" > $(ALPHABET)
	@ echo $(SHA1_ALPHABET) > $(ALPHABET).2
	@ $(LSVMTOOL) sha1 $(ALPHABET) > $(ALPHABET).1
	@ cmp $(ALPHABET).1 $(ALPHABET).2
	@ echo "=== $@ passed"

sha256-tests:
	@ echo -n "ABCDEFGHIJKLMNOPQRSTUVWXYZ" > $(ALPHABET)
	@ echo $(SHA256_ALPHABET) > $(ALPHABET).2
	@ $(LSVMTOOL) sha256 $(ALPHABET) > $(ALPHABET).1
	@ cmp $(ALPHABET).1 $(ALPHABET).2
	@ echo "=== $@ passed"

sha-tests: sha1-tests sha256-tests

##==============================================================================
##
## test-pcrs:
##
##==============================================================================

test-pcrs:
	@ mkdir -p $(TMPDIR)
	@ $(LSVMTOOL) pcrs | egrep "^PCR\\[[0-9][0-9]\\]=[A-F0-9]{40}$$" | wc -l > ${TMPDIR}/test-pcrs.1
	@ echo "16" > ${TMPDIR}/test-pcrs.2
	@ cmp ${TMPDIR}/test-pcrs.1 ${TMPDIR}/test-pcrs.2
	@ echo "=== $@ passed"

test-pcrs256:
	@ mkdir -p $(TMPDIR)
	@ $(LSVMTOOL) pcrs256 | egrep "^PCR\\[[0-9][0-9]\\]=[A-F0-9]{64}$$" | wc -l > ${TMPDIR}/test-pcrs256.1
	@ echo "16" > ${TMPDIR}/test-pcrs256.2
	@ cmp ${TMPDIR}/test-pcrs256.1 ${TMPDIR}/test-pcrs256.2
	@ echo "=== $@ passed"

##==============================================================================
##
## tests:
##
##==============================================================================

tests:
	@ $(MAKE) -s lsvmtool-tests
	@ $(MAKE) -s cpio-tests
	@ $(MAKE) -s ext2-tests MKE2FSFLAGS="-b 1024"
	@ $(MAKE) -s ext2-tests MKE2FSFLAGS="-b 4096"
	@ $(MAKE) -s ext2-tests MKE2FSFLAGS="-b 2048"
	@ $(MAKE) -s test-seal
	@ $(MAKE) -s xz-tests
	@ $(MAKE) -s gzip-tests
	@ $(MAKE) -s testxz
	@ $(MAKE) -s testzlib
	@ $(MAKE) -s strtok-tests
	@ $(MAKE) -s test-crypt
	@ $(MAKE) -s sha-tests
	@ $(MAKE) -s test-pcrs
	@ $(MAKE) -s test-pcrs256

$(AUTOTESTS): $(LSVMTOOL)
	@ $(MAKE) -s tests
	@ touch $(AUTOTESTS)

autotests: $(AUTOTESTS)

##==============================================================================
##
## install:
##
##==============================================================================

install:

-include depend.mak

size:
	size $(TARGET)

##==============================================================================
##
## seal:
##
##==============================================================================

#BOOTLOADER=$(TOP)/lsvmload/lsvmload.efi
BOOTLOADER=/boot/efi/EFI/ubuntu/shimx64.efi

seal:
	@ echo -n "ABCDEFGHIJKLMNOPQRSTUVWXYZ" > ALPHABET.in
	$(LSVMTOOL) seallsvmloadpolicy $(BOOTLOADER) ALPHABET.in ALPHABET.out


EFI_SECTIONS=1
include ../defs.mak

.PHONY: vfatfs

TARGETBASE=lsvmload
TARGET=$(TARGETBASE).efi

DEFINES += $(EFI_DEFINES) 
DEFINES += -DBUILD_EFI
DEFINES += "-DLSVMLOAD_VERSION=\"$(VERSION)\""

INCLUDES += $(EFI_INCLUDES)
INCLUDES += -I.
INCLUDES += -I..
INCLUDES += -I$(TOP)/posix/include
INCLUDES += -I$(TOP)/3rdparty
INCLUDES += -I$(TOP)/3rdparty/openssl/efi/$(OPENSSLPACKAGE)/include

CFLAGS += -Wall -Os $(EFI_CFLAGS) -Werror

SOURCES += bootfs.c
SOURCES += cert.c
SOURCES += globals.c
SOURCES += image.c
SOURCES += loadconf.c
SOURCES += log.c
SOURCES += logging.c
SOURCES += logo.c
SOURCES += main.c
SOURCES += measure.c
SOURCES += paths.c
SOURCES += stubs.c
SOURCES += luksbio.c
SOURCES += bootbio.c
SOURCES += keys.c
SOURCES += shim.c
SOURCES += efivfat.c
SOURCES += devpath.c
SOURCES += diskbio.c
SOURCES += efivfatfs.c
SOURCES += espwrap.c
SOURCES += console.c
SOURCES += initrd.c
SOURCES += specialize.c
SOURCES += rootfs.c
SOURCES += progress.c
SOURCES += version.c
SOURCES += dbxupdate.c

OBJECTS = $(SOURCES:.c=.o)

LIBRARIES += -L$(LIBDIR) -llsvmutilsefi -lcryptoefi -lposixefi -lefi -lgnuefi -llzmaefi -lzlibefi

LIBDEPENDS += $(LIBDIR)/libposixefi.a
LIBDEPENDS += $(LIBDIR)/liblsvmutilsefi.a
LIBDEPENDS += $(LIBDIR)/libzlibefi.a
LIBDEPENDS += $(LIBDIR)/liblzmaefi.a
LIBDEPENDS += $(LIBDIR)/libcryptoefi.a
LIBDEPENDS += $(LIBDIR)/liblsvmutils.a
LIBDEPENDS += $(TOP)/3rdparty/openssl/efi/openssl-1.0.2g/libcrypto.a
LIBDEPENDS += $(TOP)/3rdparty/gnuefi/gnu-efi-3.0.2/x86_64/gnuefi/libgnuefi.a
LIBDEPENDS += $(TOP)/3rdparty/gnuefi/gnu-efi-3.0.2/x86_64/lib/libefi.a

top: depend 
	$(MAKE) $(TARGET)

LDFLAGS += $(EFI_LDFLAGS)

ifdef EFI_SECTIONS
LDFLAGS += --gc-sections
#LDFLAGS += --gc-sections --print-gc-sections
endif

OBJCOPYFLAGS += $(EFI_OBJCOPYFLAGS)

$(TARGET): $(OBJECTS) $(LIBDEPENDS)
	$(MAKE) --no-print-directory timestamp
	gcc -c $(CFLAGS) $(DEFINES) $(INCLUDES) -o timestamp.o timestamp.c
	ld -o $(TARGETBASE).so $(LDFLAGS) $(OBJECTS) timestamp.o $(LIBRARIES)
	@ ./linkcheck
	objcopy $(OBJCOPYFLAGS) $(TARGETBASE).so $(TARGET)

%.o: %.c
	gcc -c $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $<

CLEAN = timestamp.c vfatfs

clean:
	rm -f $(OBJECTS) timestamp.o $(TARGETBASE).so $(TARGET) $(CLEAN)

distclean: clean
	rm -f depend.mak

depend:
	@ gcc -M $(CFLAGS) $(DEFINES) $(INCLUDES) $(SOURCES) > depend.mak

install:

##==============================================================================
##
## logo.c:
##
##==============================================================================

LOGO="LSVM Load"

logo:
	@ figlet $(LOGO) > LOGO 
	@ echo "/* $(LOGO) */" > logo.c
	@ $(BINDIR)/lsvmtool cencode --name g_logo --zeroterminate LOGO >> logo.c
	@ echo "Created logo.c"

##==============================================================================
##
## cert.c:
##
##==============================================================================

CERT=$(TOP)/certs/microsoft-uefica-public.der

cert:
	@ $(BINDIR)/lsvmtool cencode --name g_cert $(CERT) > cert.c
	@ echo "Created cert.c"

##==============================================================================
##
## vfatfs.c:
##
##==============================================================================

VFATFS=vfatfs

vfatfs:
	@ echo "=== Creating $(VFATFS)"
	@ dd if=/dev/zero of=$(VFATFS) bs=1024 count=64 2> /dev/null
	@ mkfs.vfat -s8 $(VFATFS) > /dev/null > /dev/null
	@ echo "=== Creating efivfatfs.c"
	@ $(BINDIR)/lsvmtool cencode --name efivfat $(VFATFS) > efivfatfs.c

MNT=/mnt/vfatfs

umount-vfatfs:
	- umount $(MNT) 2> /dev/null > /dev/null
	- losetup -d /dev/loop0 2> /dev/null > /dev/null
	rm -rf $(MNT)

mount-vfatfs:
	mkdir -p $(MNT)
	losetup /dev/loop0 vfatfs
	mount /dev/loop0 $(MNT)

##==============================================================================
##
## timestamp: generate timestamp.c
##
##==============================================================================

TIMESTAMP=$(shell date +"%a %m/%d/%Y %H:%M:%S")

timestamp:
	@ rm -f timestamp.c
	@ echo -n "const char timestamp[] = " >> timestamp.c
	@ echo '"$(TIMESTAMP)";' >> timestamp.c
	@ echo -n "const char __timestamp[] = " >> timestamp.c
	@ echo '"\\r\\n@TIMESTAMP@=$(TIMESTAMP)\\r\\n";' >> timestamp.c
	@ echo "Created timestamp.c"

-include depend.mak

size:
	size $(TARGETBASE).so

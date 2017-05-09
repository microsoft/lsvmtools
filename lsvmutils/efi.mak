ROOT=..
include $(ROOT)/defs.mak

DIRECTORY = lsvmutilsefi

ARCHIVE = liblsvmutilsefi.a

CFLAGS = -Wall -Werror -Os $(EFI_CFLAGS)

DEFINES = $(EFI_DEFINES) -DBUILD_EFI

INCLUDES += -I. 
INCLUDES += -I$(TOP)
INCLUDES += -I../posix/include
INCLUDES += $(EFI_INCLUDES)
INCLUDES += -I$(TOP)/3rdparty
INCLUDES += -I$(TOP)/3rdparty/openssl/efi/$(OPENSSLPACKAGE)/include

SOURCES = alloc.c buf.c conf.c error.c ext2.c getopt.c peimage.c print.c sha.c strarr.c strings.c tpmbuf.c utils.c tpm2.c tcg2.c dump.c luks.c efifile.c blkdev.c efiblkdev.c efibio.c luksblkdev.c gpt.c guid.c vfat.c memblkdev.c luksopenssl.c cpio.c initrd.c cacheblkdev.c grubcfg.c pass.c heap.c tpm2crypt.c keys.c measure.c policy.c vars.c lsvmloadpolicy.c uefidb.c

OBJECTS = $(SOURCES:.c=.o)

include ../rules.mak
-include depend.mak

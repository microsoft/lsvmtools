ROOT=..
include $(ROOT)/defs.mak

DIRECTORY = lsvmutils

ARCHIVE = liblsvmutils.a

CFLAGS = -Wall -Werror -Os -Wno-trigraphs $(LINUX_CFLAGS)

DEFINES =

INCLUDES += -I$(TOP) 
INCLUDES += -I. 
INCLUDES += $(EFI_INCLUDES)
INCLUDES += -I$(TOP)/3rdparty
INCLUDES += -I$(TOP)/3rdparty/openssl/linux/$(OPENSSLPACKAGE)/include

SOURCES = alloc.c buf.c conf.c error.c ext2.c file.c getopt.c peimage.c print.c sha.c strarr.c strings.c tcg2.c tpm2.c tpmbuf.c utils.c blkdev.c linuxblkdev.c luks.c dump.c luksblkdev.c gpt.c guid.c vfat.c memblkdev.c luksopenssl.c uefidb.c cpio.c initrd.c cacheblkdev.c grubcfg.c exec.c pass.c heap.c tpm2crypt.c keys.c uefidbx.c policy.c measure.c vars.c lsvmloadpolicy.c

OBJECTS = $(SOURCES:.c=.o)

include ../rules.mak
-include depend.mak

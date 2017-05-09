ROOT=../..
include $(ROOT)/defs.mak

DIRECTORY = zlibefi

ARCHIVE = libzlibefi.a

CFLAGS = -Wall -Werror -Os $(EFI_CFLAGS)

DEFINES = -D_LARGEFILE64_SOURCE=1 -DHAVE_HIDDEN -DNO_snprintf -DNO_vsnprintf $(EFI_DEFINES) -DBUILD_EFI

INCLUDES += -I. 
INCLUDES += -I$(TOP)
INCLUDES += -I$(TOP)/posix/include
INCLUDES += -I$(TOP)/3rdparty/zlib/zlib
INCLUDES += $(EFI_INCLUDES)

SOURCES += zlib/adler32.c 
SOURCES += zlib/compress.c
SOURCES += zlib/crc32.c
SOURCES += zlib/deflate.c
SOURCES += zlib/gzclose.c
SOURCES += zlib/gzlib.c
SOURCES += zlib/gzread.c
SOURCES += zlib/gzwrite.c
SOURCES += zlib/infback.c
SOURCES += zlib/inffast.c
SOURCES += zlib/inflate.c
SOURCES += zlib/inftrees.c
SOURCES += zlib/trees.c
SOURCES += zlib/uncompr.c
SOURCES += zlib/zutil.c
SOURCES += zlibextras.c

OBJECTS=$(SOURCES:.c=.o)

include $(ROOT)/rules.mak
-include depend.mak

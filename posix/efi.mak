include ../defs.mak

DIRECTORY = posix

ARCHIVE = libposixefi.a

CFLAGS = -Wall -Werror -Os $(EFI_CFLAGS)

DEFINES += $(EFI_DEFINES) -DBUILD_EFI -DBUILDEFI

INCLUDES += $(EFI_INCLUDES) -Iinclude

SOURCES = assert.c ctype.c dlfcn.c errno.c limits.c stdio.c stdlib.c string.c time.c unistd.c

OBJECTS = $(SOURCES:.c=.o)

include ../rules.mak
-include depend.mak

install:

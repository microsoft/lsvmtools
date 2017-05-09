include ../defs.mak

DIRECTORY = posixlinux

ARCHIVE = libposixlinux.a

CFLAGS = -Wall -Werror -Os $(LINUX_CFLAGS)

DEFINES +=

INCLUDES += -Iinclude

SOURCES = assert.c ctype.c dlfcn.c errno.c limits.c stdio.c stdlib.c string.c time.c unistd.c

OBJECTS = $(SOURCES:.c=.o)

include ../rules.mak
-include depend.mak

install:

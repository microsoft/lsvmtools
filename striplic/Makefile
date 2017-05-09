include ../defs.mak

DIRECTORY=striplic

DEFINES +=

INCLUDES +=

CFLAGS = -c -Wall -g -O3
CFLAGS += -Werror
CFLAGS += $(LINUX_CFLAGS)

SOURCES += main.c 

OBJECTS = $(SOURCES:.c=.o)

PROGRAM=striplic

include ../rules.mak

distclean:

include ../defs.mak

TARGET=$(BINDIR)/lsvmtool-password-agent
SOURCES=lsvmtool-password-agent.c
INCLUDES=-I$(TOP)
LIBRARIES=-L$(LIBDIR) -llsvmutils -pthread

SHARED=-Wall -o $(TARGET) $(INCLUDES) $(SOURCES) $(LIBRARIES)

ifndef TIMEOUT
TIMEOUT=15000
endif

all: debug

release:
	gcc -D TIMEOUT=$(TIMEOUT) $(SHARED)

debug:
	gcc -D DEBUG -D TIMEOUT=$(TIMEOUT) $(SHARED)

debug-no-cap:
	gcc -D DEBUG -D TIMEOUT=$(TIMEOUT) $(SHARED)

CLEAN = \
    *.o \
    $(TARGET) \
    tmp \
    KEY0 \
    KEY1 \
    KEY2

clean:
	rm -rf $(CLEAN)

distclean: clean

tests:
	./agent-tests

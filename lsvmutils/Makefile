include ../defs.mak

all:
	$(MAKE) -f linux.mak
	$(MAKE) -f efi.mak

depend:
	$(MAKE) -f linux.mak depend
	$(MAKE) -f efi.mak depend

clean:
	$(MAKE) -f linux.mak clean
	$(MAKE) -f efi.mak clean

distclean: clean
	rm -f depend.mak

install:

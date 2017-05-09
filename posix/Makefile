include ../defs.mak

all:
	$(MAKE) -f efi.mak
	$(MAKE) -f linux.mak

clean:
	$(MAKE) -f efi.mak clean
	$(MAKE) -f linux.mak clean

distclean: clean
	rm -f depend.mak

depend:
	$(MAKE) -f efi.mak depend
	$(MAKE) -f linux.mak depend

install:

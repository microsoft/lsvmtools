include ../../defs.mak

OPENSSLDISTRO=$(OPENSSLPACKAGE).tar.gz

all: $(OPENSSLDISTRO)
	$(MAKE) -C efi
	$(MAKE) -C linux

clean:
	$(MAKE) -C efi clean
	$(MAKE) -C linux clean

distclean: clean
	$(MAKE) -C efi distclean
	$(MAKE) -C linux distclean
	rm -f $(OPENSSLDISTRO)

install: all
	$(MAKE) -C efi install
	$(MAKE) -C linux install

$(OPENSSLDISTRO):
	wget https://www.openssl.org/source/old/1.0.2/openssl-1.0.2g.tar.gz


define NL


endef

DIRS = gnuefi openssl xz zlib

all:
	@ $(foreach i, $(DIRS), $(MAKE) -C $(i) $(NL) )

clean:
	@ $(foreach i, $(DIRS), $(MAKE) -C $(i) clean $(NL) )

distclean:
	@ $(foreach i, $(DIRS), $(MAKE) -C $(i) distclean $(NL) )


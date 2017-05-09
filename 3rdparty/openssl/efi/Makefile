include ../../../defs.mak

##==============================================================================
##
## TARGET
##
##==============================================================================

TARGET=$(LIBDIR)/libcryptoefi.a

##==============================================================================
##
## OPENSSLSRCDIR
##
##==============================================================================

OPENSSLSRCDIR=$(OPENSSLPACKAGE)

##==============================================================================
##
## EFICC
##
##==============================================================================

EFICC_CFLAGS += -Os $(EFI_CFLAGS)

EFICC_DEFINES += -DEFIBUILD
EFICC_DEFINES += -DBUILD_EFI
EFICC_DEFINES += -DOPENSSL_IMPLEMENTS_strncasecmp 
EFICC_DEFINES += -DOPENSSL_NO_SOCK 
EFICC_DEFINES += -DNO_SYSLOG
EFICC_DEFINES += -DOPENSSL_NO_DEPRECATED 

EFICC_INCLUDES += $(EFI_INCLUDES)
EFICC_INCLUDES += -I$(TOP)/posix/include

EFICC = gcc $(EFICC_CFLAGS) $(EFICC_DEFINES) $(EFICC_INCLUDES)

##==============================================================================
##
## all:
##
##==============================================================================

$(TARGET):
	$(MAKE) unpack
	$(MAKE) patch
	$(MAKE) configure
	$(MAKE) build

##==============================================================================
##
## build
##
##==============================================================================

build:
	$(MAKE) -C $(OPENSSLSRCDIR) DIRS=crypto CC="$(EFICC)"
	mkdir -p $(LIBDIR)
	cp $(OPENSSLSRCDIR)/libcrypto.a $(TARGET)

##==============================================================================
##
## unpack
##
##==============================================================================

unpack:
	rm -rf $(OPENSSLSRCDIR)
	tar zxvf ../$(OPENSSLSRCDIR).tar.gz

##==============================================================================
##
## patch: Patch OpenSSL by truncating sources with unused definitions:
##
##==============================================================================

define NL


endef

PATCH = \
    $(OPENSSLSRCDIR)/crypto/rand/randfile.c \
    $(OPENSSLSRCDIR)/crypto/rand/rand_egd.c \
    $(OPENSSLSRCDIR)/crypto/ui/ui_openssl.c \
    $(OPENSSLSRCDIR)/crypto/ts/ts_rsp_sign.c \
    $(OPENSSLSRCDIR)/crypto/rand/rand_unix.c \
    $(OPENSSLSRCDIR)/crypto/bio/bss_fd.c \
    $(OPENSSLSRCDIR)/crypto/bio/bss_sock.c \
    $(OPENSSLSRCDIR)/crypto/bio/bss_conn.c \
    $(OPENSSLSRCDIR)/crypto/bio/b_sock.c \
    $(OPENSSLSRCDIR)/crypto/bio/bss_acpt.c \
    $(OPENSSLSRCDIR)/crypto/bio/bss_dgram.c \
    $(OPENSSLSRCDIR)/crypto/x509/by_dir.c \
    $(OPENSSLSRCDIR)/crypto/o_dir.c \
    $(OPENSSLSRCDIR)/crypto/des/enc_read.c \
    $(OPENSSLSRCDIR)/crypto/des/enc_writ.c

patch:
	$(foreach i, $(PATCH), ./wrap $(i) $(NL) )

##==============================================================================
##
## configure:
##
##==============================================================================

configure:
	( cd $(OPENSSLSRCDIR); ./config )

##==============================================================================
##
## clean:
##
##==============================================================================

clean:
	- @ $(MAKE) -C $(OPENSSLSRCDIR) clean 2> /dev/null > /dev/null
	- @ rm -f $(TARGET)

distclean:
	$(MAKE) -s clean
	rm -rf $(OPENSSLSRCDIR)
	rm -f depend.mak

install:

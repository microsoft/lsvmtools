include ../defs.mak

LSVMTOOL=$(BINDIR)/lsvmtool

all:

ROOTDEV=$(shell $(TOP)/scripts/rootdev)
BOOTDEV=$(shell $(TOP)/scripts/bootdev)
POLICIES=$(wildcard *.policy)

##==============================================================================
##
## Select policy files:
##
##==============================================================================

ifeq ($(LSVM_SECUREBOOT),TRUE)
  BOOTKEY_POLICY=bootkeysb.policy
  ROOTKEY_POLICY=rootkeysb.policy
else ifeq ($(LSVM_SECUREBOOT),FALSE)
  BOOTKEY_POLICY=bootkey.policy
  ROOTKEY_POLICY=rootkey.policy
else ifeq ($(LSVM_SECUREBOOT),)
  BOOTKEY_POLICY=bootkey.policy
  ROOTKEY_POLICY=rootkey.policy
else
  $(error "Bad value for LSVM_SECUREBOOT") 
endif

##==============================================================================
##
## LSVM_VENDOREFIDIR (used by 'lsvmtool sealpolicy' subcommand)
##
##==============================================================================

ifndef LSVM_VENDOREFIDIR
  export LSVM_VENDOREFIDIR=$(VENDOREFIDIR)
endif

##==============================================================================
##
## LSVM_LSVMLOAD (used by 'lsvmtool sealpolicy' subcommand)
##
##==============================================================================

ifndef LSVM_LSVMLOAD
  export LSVM_LSVMLOAD=$(TOP)/lsvmload/lsvmload.efi
endif

##==============================================================================
##
## LSVM_SECURE_BOOT (used by 'lsvmtool sealpolicy' subcommand)
##
##==============================================================================

ifndef LSVM_SECURE_BOOT
  export LSVM_SECURE_BOOT=False
endif

##==============================================================================
##
## LSVM_SHIM (used by 'lsvmtool sealpolicy' subcommand)
##
##==============================================================================

ifndef LSVM_SHIM
  export LSVM_SHIM=$(SHIMX64EFI)
endif

##==============================================================================
##
## LSVM_GRUB (used by 'lsvmtool sealpolicy' subcommand)
##
##==============================================================================

ifndef LSVM_GRUB
  export LSVM_GRUB=$(GRUBX64EFI)
endif

##==============================================================================
##
## bootkey:
##
##==============================================================================

install-bootkey: gen
	@ echo "=== Creating $(INSTALLEFIDIR)/bootkey"
	@ echo "=== Using $(BOOTKEY_POLICY)"
	@ echo -n $(TEST_BOOT_PASSPHRASE) > /tmp/pp
	@ $(LSVMTOOL) sealpolicy $(BOOTKEY_POLICY) /tmp/pp $(INSTALLEFIDIR)/bootkey
	@ rm -f /tmp/pp

##==============================================================================
##
## rootkey:
##
##==============================================================================

install-rootkey: gen
	@ echo "=== Creating $(INSTALLEFIDIR)/rootkey"
	@ echo "=== Using $(ROOTKEY_POLICY)"
	@ echo -n $(TEST_ROOT_PASSPHRASE) > /tmp/pp
	@ $(LSVMTOOL) sealpolicy $(ROOTKEY_POLICY) /tmp/pp $(INSTALLEFIDIR)/rootkey
	@ rm -f /tmp/pp

##==============================================================================
##
## install:
##
##==============================================================================

install: install-bootkey install-rootkey

##==============================================================================
##
## measurements:
##
##==============================================================================

measure-bootkey: gen
	$(LSVMTOOL) measurepolicy --log $(BOOTKEY_POLICY)

measure-rootkey: gen
	$(LSVMTOOL) measurepolicy --log $(ROOTKEY_POLICY)

measure-final: gen
	$(LSVMTOOL) measurepolicy --log final.policy

gen:
	@ $(LSVMTOOL) efivar 8BE4DF6193CA11D2AA0D00E098032B8C SecureBoot
	@ $(LSVMTOOL) hextobin 00 8BE4DF6193CA11D2AA0D00E098032B8C-SecureBoot.False
	@ $(LSVMTOOL) hextobin 01 8BE4DF6193CA11D2AA0D00E098032B8C-SecureBoot.True
	@ $(LSVMTOOL) hextobin 00000000 SEPARATOR
	@ $(LSVMTOOL) efivar 8BE4DF6193CA11D2AA0D00E098032B8C PK
	@ $(LSVMTOOL) efivar 8BE4DF6193CA11D2AA0D00E098032B8C KEK
	@ $(LSVMTOOL) efivar D719B2CB3D3A4596A3BCDAD00E67656F db
	@ $(LSVMTOOL) efivar D719B2CB3D3A4596A3BCDAD00E67656F dbx
	@ $(LSVMTOOL) parsedb --quiet D719B2CB3D3A4596A3BCDAD00E67656F-db $(LSVM_LSVMLOAD) > D719B2CB3D3A4596A3BCDAD00E67656F-db.Entry
	@ $(LSVMTOOL) hextobin 10000000 ALLOW_PREBOOT_SEALING
	@ $(LSVMTOOL) hextobin 0200F000 LINUX_SCENARIO_ID
	@ $(LSVMTOOL) hextobin 01000000 LINUX_SCENARIO_VERSION

tests: seal unseal
	cmp seal.in unseal.out
	rm -rf seal.in seal.out unseal.out

convert:
	openssl x509 -inform DER -outform PEM -in microsoft-uefica-public.der -out erp.der

pass:
	@ echo "TEST_ROOT_PASSPHRASE=$(TEST_ROOT_PASSPHRASE)"
	@ echo "TEST_BOOT_PASSPHRASE=$(TEST_BOOT_PASSPHRASE)"

CLEAN = \
    8BE4DF6193CA11D2AA0D00E098032B8C-KEK \
    8BE4DF6193CA11D2AA0D00E098032B8C-PK \
    8BE4DF6193CA11D2AA0D00E098032B8C-SecureBoot \
    8BE4DF6193CA11D2AA0D00E098032B8C-SecureBoot.False \
    8BE4DF6193CA11D2AA0D00E098032B8C-SecureBoot.True \
    D719B2CB3D3A4596A3BCDAD00E67656F-db \
    D719B2CB3D3A4596A3BCDAD00E67656F-dbx \
    D719B2CB3D3A4596A3BCDAD00E67656F-db.Entry \
    ALLOW_PREBOOT_SEALING \
    LINUX_SCENARIO_ID \
    LINUX_SCENARIO_VERSION \
    SEPARATOR

clean:
	rm -rf $(CLEAN)

distclean: clean

vars:
	@ echo "LSVMROOT=$(LSVMROOT)"

expand:
	$(LSVMTOOL) policy $(ROOTKEY_POLICY)
	$(LSVMTOOL) policy $(ROOTKEY_POLICY)


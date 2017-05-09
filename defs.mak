TOP=$(abspath $(dir $(word 2, $(MAKEFILE_LIST))))

-include $(TOP)/config.mak

EFI_DEFINES += -DEFI_FUNCTION_WRAPPER
EFI_DEFINES += -DGNU_EFI_USE_MS_ABI

ifdef ENABLE_WERROR
 EFI_CFLAGS += -Werror
endif

EFI_CFLAGS += -fpic 
EFI_CFLAGS += -fno-builtin
EFI_CFLAGS += -std=gnu89 
EFI_CFLAGS += -fno-stack-protector 
EFI_CFLAGS += -fno-strict-aliasing 
EFI_CFLAGS += -fshort-wchar 
EFI_CFLAGS += -nostdinc 
EFI_CFLAGS += -mno-red-zone 
EFI_CFLAGS += -fvisibility=hidden
EFI_CFLAGS += -fno-asynchronous-unwind-tables 
EFI_CFLAGS += -maccumulate-outgoing-args
EFI_CFLAGS += -ffunction-sections 
#EFI_CFLAGS += -fdata-sections

USE_LOCAL_GNUEFI=1

ifdef USE_LOCAL_GNUEFI
EFI_SRCDIR = $(TOP)/3rdparty/gnuefi/gnu-efi-3.0.2
EFI_ARCHDIR = $(EFI_SRCDIR)/x86_64
endif

ifdef USE_LOCAL_GNUEFI
  EFI_INCLUDES += -I$(EFI_SRCDIR)/inc
  EFI_INCLUDES += -I$(EFI_SRCDIR)/inc/protocol
  EFI_INCLUDES += -I$(EFI_SRCDIR)/inc/x86_64
else
  EFI_INCLUDES += -I/usr/include/efi
  EFI_INCLUDES += -I/usr/include/efi/x86_64
endif

EFI_LDFLAGS += -fvisibility=hidden
EFI_LDFLAGS += -nostdlib
EFI_LDFLAGS += -znocombreloc
EFI_LDFLAGS += -shared
EFI_LDFLAGS += -Bsymbolic

EFI_LDFLAGS += -T $(TOP)/3rdparty/gnuefi/elf_x86_64_efi.lds
EFI_LDFLAGS += -L$(EFI_ARCHDIR)/gnuefi
EFI_LDFLAGS += -L$(EFI_ARCHDIR)/lib
EFI_LDFLAGS += $(EFI_ARCHDIR)/gnuefi/crt0-efi-x86_64.o

EFI_LDFLAGS += $(shell gcc -print-libgcc-file-name)

EFI_OBJCOPYFLAGS += -j .text 
EFI_OBJCOPYFLAGS += -j .sdata 
EFI_OBJCOPYFLAGS += -j .data
EFI_OBJCOPYFLAGS += -j .dynamic 
EFI_OBJCOPYFLAGS += -j .dynsym  
EFI_OBJCOPYFLAGS += -j .rel*
EFI_OBJCOPYFLAGS += -j .rela* 
EFI_OBJCOPYFLAGS += -j .reloc 
EFI_OBJCOPYFLAGS += -j .eh_frame
EFI_OBJCOPYFLAGS += -j .vendor_cert
EFI_OBJCOPYFLAGS += --target efi-app-x86_64  

OBJDIR=$(TOP)/build/obj
LIBDIR=$(TOP)/build/lib
BINDIR=$(TOP)/build/bin
TMPDIR=$(TOP)/build/tmp

# EFI paths
VENDOREFIDIR = /boot/efi/EFI/$(DISTRONAME)
INSTALLEFIDIR = /boot/efi/EFI/boot

# Shim / GRUB paths
ifeq ($(DISTRONAME), sles)
  GRUBX64NAME=grub.efi
else
  GRUBX64NAME=grubx64.efi
endif
GRUBX64EFI=$(VENDOREFIDIR)/$(GRUBX64NAME)

ifeq ($(DISTRONAME), ubuntu)
  SHIMX64NAME=shimx64.efi
else
  SHIMX64NAME=shim.efi
endif
SHIMX64EFI=$(VENDOREFIDIR)/$(SHIMX64NAME)

-include $(HOME)/lsvm.mak

ifndef TEST_BOOT_PASSPHRASE
  TEST_BOOT_PASSPHRASE=passphrase
endif

ifndef TEST_ROOT_PASSPHRASE
  TEST_ROOT_PASSPHRASE=passphrase
endif

OPENSSLPACKAGE=openssl-1.0.2g

LINUX_CFLAGS += -ffunction-sections 
LINUX_CFLAGS += -fdata-sections

#OPENSSLPACKAGE=openssl-1.1.0

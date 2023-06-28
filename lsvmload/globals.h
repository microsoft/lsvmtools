/*
**==============================================================================
**
** LSVMTools 
** 
** MIT License
** 
** Copyright (c) Microsoft Corporation. All rights reserved.
** 
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
** 
** The above copyright notice and this permission notice shall be included in 
** all copies or substantial portions of the Software.
** 
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE
**
**==============================================================================
*/
#ifndef _globals_h
#define _globals_h

#include "config.h"
#include <lsvmutils/eficommon.h>
#include <lsvmutils/tcg2.h>
#include <lsvmutils/ext2.h>
#include <lsvmutils/tpm2.h>
#include <lsvmutils/guid.h>
#include <lsvmutils/efibio.h>
#include <lsvmutils/initrd.h>
#include <lsvmutils/strarr.h>

/* All global variables for this binary */

typedef struct _Globals
{
    EFI_HANDLE imageHandle;
    EFI_SYSTEM_TABLE* systemTable;
    EFI_TCG2_PROTOCOL* tcg2Protocol;

    /* TRUE if measured boot failed */
    BOOLEAN measuredBootFailed;

    /* Path of 'lsvmload' */
    CHAR16* lsvmloadPath;

    /* Path of 'lsvmconf' */
    CHAR16* lsvmconfPath;

    /* Path of 'lsvmlog' */
    CHAR16* lsvmlogPath;

    /* From lsvmconf "lsvmconf:KernelPath=" */
    CHAR16* kernelPath;

    /* UUID of the boot device */
    char bootDevice[GUID_STRING_SIZE];

    /* UUID of the root device */
    char rootDevice[GUID_STRING_SIZE];

    /* BIO for LUKS boot partition */
    EFI_BIO* bootbio;

    /* LUKS block device for boot partition */
    Blkdev* bootdev;

    /* Cache device that bootdev uses */
    Blkdev* cachedev;

    /* Sealed keys */
    CHAR16* sealedKeysPath;
    TPM2X_BLOB sealedKeys;
    UINT8* unsealedKeys;
    UINTN unsealedKeySize;

    /* bootkey */
    BOOLEAN bootkeyFound;
    UINT8* bootkeyData;
    UINTN bootkeySize;

    /* rootkey */
    BOOLEAN rootkeyFound;
    UINT8* rootkeyData;
    UINTN rootkeySize;
    BOOLEAN rootkeyValid;

    /* specialization file */
    CHAR16* specializePath;

    /* Boot file system */
    EXT2* bootfs;

    /* Enable root drive I/O hooks */
    BOOLEAN enableIOHooks;

    /* Name of vendor EFI directory (e.g., "UBUNTU") */
    CHAR16* efiVendorDir;

    /* Preloaded grub.cfg */
    void* grubcfgData;
    UINTN grubcfgSize;

    /* Preloaded GRUB */
    void* grubData;
    UINTN grubSize;
}
Globals;

extern Globals globals;

#endif /* _globals_h */

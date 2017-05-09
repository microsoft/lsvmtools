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
#include "luksbio.h"
#include "config.h"
#include <lsvmutils/eficommon.h>
#include <lsvmutils/alloc.h>
#include <lsvmutils/luks.h>
#include <lsvmutils/efibio.h>
#include <lsvmutils/tpm2.h>
#include <lsvmutils/efifile.h>
#include "log.h"
#include "globals.h"

typedef struct _Masterkey
{
    const UINT8* data;
    UINTN size;
}
Masterkey;

static BOOLEAN _MatchBIO(
    EFI_BIO* bio,
    void* data)
{
    const char* uuid = (const char*)data;
    BOOLEAN result = FALSE;
    union
    {
        LUKSHeader header;
        UINT8 blocks[LUKS_SECTOR_SIZE*2];
    }
    u;

    /* Check parameters */
    if (!ValidBIO(bio))
    {
        LOGE(L"_MatchBIO(): invalid BIO");
        goto done;
    }

    /* Check that block size is not LUKS_SECTOR_SIZE */
    if (BlockSizeBIO(bio) != LUKS_SECTOR_SIZE)
        goto done;

    /* Read the LUKS header (blkno == 0) */
    if (ReadBIO(bio, 0, &u.blocks, sizeof(u.blocks)) != EFI_SUCCESS)
        goto done;

    /* If LUKS header UUID does not match BootDeviceLUKS option from lsvmconf */
    if (Strcmp(u.header.uuid, uuid) != 0)
        goto done;

    result = TRUE;

done:

    return result;
}

EFI_BIO* OpenLUKSBIO(
    EFI_HANDLE imageHandle,
    const char uuid[GUID_STRING_SIZE])
{
    EFI_BIO* bio = NULL;

    /* Check parameters */
    if (!imageHandle || !uuid)
        goto done;

    if (!(bio = OpenBIO(imageHandle, _MatchBIO, (void*)uuid)))
    {
        LOGE(L"failed to find LUKS BIO for uuid: %a", uuid);
        goto done;
    }

done:

    return bio;
}

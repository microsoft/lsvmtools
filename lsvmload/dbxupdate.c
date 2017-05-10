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
#include "dbxupdate.h"
#include <lsvmutils/alloc.h>
#include <lsvmutils/efivarauth.h>
#include <lsvmutils/lsvmloadpolicy.h>
#include <lsvmutils/measure.h>
#include <lsvmutils/efifile.h>
#include "bootfs.h"
#include "log.h"
#include "logging.h"
#include "globals.h"
#include "progress.h"

#define PKCS7_GUID { 0x4aafd29d, 0x68df, 0x49ee, {0x8a, 0xa9, 0x34, 0x7d, 0x37, 0x56, 0x65, 0xa7} }

static int _TestSecureBootVariable(BOOLEAN* result)
{
    int rc = -1;
    unsigned char* data = NULL;
    UINTN size;

    if (!result)
        goto done;

    *result = FALSE;

    if (LoadEFIVar(
        "8BE4DF6193CA11D2AA0D00E098032B8C",
        "SecureBoot",
        &data,
        &size) != 0)
    {
        goto done;
    }

    if (size != 1)
        goto done;

    if (data[0])
        *result = TRUE;

    rc = 0;

done:

    if (data)
        Free(data);

    return rc;
}

EFI_STATUS ApplyDBXUpdate(
    EFI_HANDLE imageHandle,
    EFI_TCG2_PROTOCOL *tcg2Protocol,
    EXT2* bootfs,
    BOOLEAN* reboot)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    UINT32 attrs = 0;
    void* data = NULL;
    UINTN size;
    void* lsvmloadData = NULL;
    UINTN lsvmloadSize;
    CHAR16 PATH[] = L"/lsvmload/dbxupdate.bin";
    static EFI_GUID guid =
        {0xd719b2cb,0x3d3a,0x4596,{0xa3,0xbc,0xda,0xd0,0xe,0x67,0x65,0x6f}};

    if (reboot)
        *reboot = TRUE;

    if (!imageHandle || !tcg2Protocol || !bootfs || !reboot)
        goto done;

    /* Load the dbxupdate file if any */
    if (LoadFileFromBootFS(
        imageHandle,
        tcg2Protocol,
        bootfs,
        PATH,
        &data,
        &size) != EFI_SUCCESS)
    {
        LOGI(L"No DBX updates found");
        status = EFI_SUCCESS;
        *reboot = FALSE;
        goto done;
    }

    PutProgress(L"Applying DBX update");

    /* Set the variable */
    {
        attrs |= EFI_VARIABLE_NON_VOLATILE;
        attrs |= EFI_VARIABLE_BOOTSERVICE_ACCESS;
        attrs |= EFI_VARIABLE_RUNTIME_ACCESS;
        attrs |= EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS;
        attrs |= EFI_VARIABLE_APPEND_WRITE;

        status = RT->SetVariable(L"dbx", &guid, attrs, size, data);

        if (status != EFI_SUCCESS)
        {
            LOGE(L"Failed to apply DBX update: %ld", (long)status);
            goto done;
        }
        else
        {
            LOGI(L"Applied DBX update: %s", Wcs(PATH));
        }
    }

    /* Reseal the keys */
    {
        BOOLEAN secureBoot = FALSE;
        BYTE sealedkeysData[TPM2X_BLOB_SIZE];
        UINTN sealedkeysSize;
        Error err;

        /* Read the SecureBoot variable */
        if (_TestSecureBootVariable(&secureBoot) != 0)
        {
            LOGE(L"Failed to test SecureBoot variable");
            goto done;
        }

        LOGI(L"SecureBoot{%d}", (int)secureBoot);

        /* Load LSVMLOAD into memory */
        if (EFILoadFile(
            imageHandle, 
            globals.lsvmloadPath, 
            &lsvmloadData, 
            &lsvmloadSize) != EFI_SUCCESS)
        {
            LOGE(L"cannot load %s", Wcs(globals.lsvmloadPath));
            goto done;
        }

        LOGI(L"Loaded %s", Wcs(globals.lsvmloadPath));

        /* Reseal the keys */
        if (SealLSVMLoadPolicy(
            tcg2Protocol,
            lsvmloadData,
            lsvmloadSize,
            secureBoot,
            globals.unsealedKeys,
            globals.unsealedKeySize,
            sealedkeysData,
            &sealedkeysSize,
            &err) != 0)
        {
            LOGE(L"reseal: %s", Wcs(err.buf));
            goto done;
        }

        /* Rewrite the 'sealedkeys' file */
        if (EFIPutFile(
            imageHandle,
            globals.sealedKeysPath,
            sealedkeysData,
            sealedkeysSize,
            &err) != EFI_SUCCESS)
        {
            LOGE(L"failed to create %s: %s", Wcs(globals.sealedKeysPath),
                Wcs(err.buf));
            goto done;
        }

        LOGI(L"Created %s", Wcs(globals.sealedKeysPath));
    }

    /* Remove 'dbxupdate.bin' from the disk */
    {
        /* Disable caching (to force writes to disk) */
        globals.cachedev->SetFlags(globals.cachedev, 0);

        /* Remove dbxupdate.bin file */
        if (RemoveFileFromBootFS(
            imageHandle,
            tcg2Protocol,
            bootfs,
            PATH) != EFI_SUCCESS)
        {
            globals.cachedev->SetFlags(globals.cachedev, BLKDEV_ENABLE_CACHING);
            LOGE(L"Failed to remove %s", PATH);
            goto done;
        }

        /* Reenable caching (to avoid writes to disk) */
        globals.cachedev->SetFlags(globals.cachedev, BLKDEV_ENABLE_CACHING);
    }

done:

    if (data)
        Free(data);

    if (lsvmloadData)
        Free(lsvmloadData);

    return status;
}

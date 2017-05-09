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
#include "specialize.h"
#include <lsvmutils/efifile.h>
#include <lsvmutils/alloc.h>
#include <lsvmutils/luksblkdev.h>
#include "globals.h"
#include "log.h"
#include "progress.h"

/* Functions to handle different encryption modes. */
typedef int (*SpecFunc)(SECURE_SPECIALIZATION*, const UINT8*, UINTN, UINT8**, UINTN*);
static int _AES_CBC_Decrypt(SECURE_SPECIALIZATION*, const UINT8*, UINTN, UINT8**, UINTN*);
static int _AES_CBC_SHA256_Decrypt(SECURE_SPECIALIZATION*, const UINT8*, UINTN, UINT8**, UINTN*);
static const SpecFunc SpecFuncs[SECURE_SPECIALIZATION_MODE_END-1] =
{
    _AES_CBC_Decrypt,
    _AES_CBC_SHA256_Decrypt
};

static BOOLEAN _InBounds(
        UINT32 off,
        UINT32 len,
        UINTN min,
        UINTN max)
{
    return (off >= min) && (off + len <= max); 
}

static BOOLEAN _IsValidHeader(
    void *data,
    UINTN size)
{
    SECURE_SPECIALIZATION* spec;
    size_t specSize;
    BOOLEAN result = TRUE;

    if (size < SECURE_SPECIALIZATION_SIZE)
    {
        return FALSE;
    }

    spec = (SECURE_SPECIALIZATION*) data;
    specSize = sizeof(*spec);

    if (!IS_SPECIALIZATION_MODE_SUPPORTED(spec->Mode)) 
    {
        return FALSE;
    }

    result &= _InBounds(spec->IVOffset, spec->IVLength, specSize, size);
    result &= _InBounds(spec->CipherOffset, spec->CipherLength, specSize, size);
    if (spec->Mode == SECURE_SPECIALIZATION_MODE_AES_CBC_SHA256)
    {
        result &= _InBounds(spec->HMACOffset, spec->HMACLength, specSize, size);
    }
    return result;
}

static int _AES_CBC_Decrypt(
    SECURE_SPECIALIZATION *spec,
    const UINT8* key,
    UINTN keyBits,
    UINT8** out,
    UINTN* outSize)
{
    UINT8* iv = ((UINT8*) spec + spec->IVOffset);
    UINT8* cipher = ((UINT8*) spec + spec->CipherOffset);

    return TPM2X_AES_CBC_Decrypt(
            cipher,
            spec->CipherLength,
            key,
            keyBits,
            iv,
            spec->IVLength,
            out,
            outSize);
}

// TODO: Change later, just use the same for now.
static int _AES_CBC_SHA256_Decrypt(
    SECURE_SPECIALIZATION *spec,
    const UINT8* key,
    UINTN keyBits,
    UINT8** out,
    UINTN* outSize)
{
    UINT8* iv = ((UINT8*) spec + spec->IVOffset);
    UINT8* cipher = ((UINT8*) spec + spec->CipherOffset);

    return TPM2X_AES_CBC_Decrypt(
            cipher,
            spec->CipherLength,
            key,
            keyBits,
            iv,
            spec->IVLength,
            out,
            outSize); 
}

int LoadDecryptCopySpecializeFile(
    EFI_HANDLE imageHandle,
    Blkdev* bootdev,
    EXT2* bootfs,
    const CHAR16* path)
{
    const char* func = __FUNCTION__;
    int rc = -1;
    void* data = NULL;
    UINTN size;
    const UINT8* mkData = NULL;
    UINTN mkSize;
    UINT8* specializeData = NULL;
    UINTN specializeSize = 0;

    /* Check for null parameters */
    if (!imageHandle || !bootdev || !path)
    {
        LOGE(L"%a(): null parameter", Str(func));
        goto done;
    }

    /* If specialize file exists, then load and decrypt it */
    if (FileExists(imageHandle, path) == EFI_SUCCESS)
    {
        SECURE_SPECIALIZATION* spec;

        /* Load the file into memory */
        if (EFILoadFile(imageHandle, path, &data, &size) != EFI_SUCCESS)
        {
            LOGE(L"%a: failed to load %s", Str(func), Wcs(path));
            goto done;
        }

        /* Check for valid header in spec file. */
        if (!_IsValidHeader(data, size))
        {
            LOGE(L"%a: failed to validate", Str(func));
            goto done;
        }

        /* Get the masterkey from the boot device */
        if (LUKSBlkdevGetMasterKey(bootdev, &mkData, &mkSize) != 0)
        {
            LOGE(L"%a: failed to retrieve masterkey", Str(func));
            goto done;
        }

        /* Decrypt the specialization file */
        spec = (SECURE_SPECIALIZATION*) data;
        if (SpecFuncs[spec->Mode-1](
            spec,
            mkData, 
            mkSize * 8, /* key size in bits */
            &specializeData, 
            &specializeSize) != 0)
        {
            LOGE(L"%a: failed to decrypt specialize file", Str(func));
            goto done;
        }

        LOGI(L"Loaded %s", Wcs(path));

        PutProgress(L"Creating /lsvmload/specialize");

        /* Copy file to boot partition */
        if (EXT2Put(
            bootfs, 
            specializeData, 
            specializeSize, 
            "/lsvmload/specialize",
            EXT2_FILE_MODE_RW0_000_000) != EXT2_ERR_NONE)
        {
            LOGE(L"%a: failed to create boot:/lsvmload/specialize", Str(func));
            goto done;
        }
        else
        {
            LOGI(L"Created boot:/lsvmload/specialize");
        }
    }

    rc = 0;

done:
    if (data)
        Free(data);

    if (specializeData)
        Free(specializeData);

    return rc;
}

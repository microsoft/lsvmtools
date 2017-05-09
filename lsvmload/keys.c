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
#include "keys.h"
#include "luksbio.h"
#include "config.h"
#include <lsvmutils/eficommon.h>
#include <lsvmutils/alloc.h>
#include <lsvmutils/luks.h>
#include <lsvmutils/efibio.h>
#include <lsvmutils/tpm2.h>
#include <lsvmutils/efifile.h>
#include <lsvmutils/sha.h>
#include <lsvmutils/keys.h>
#include "log.h"
#include "logging.h"
#include "console.h"
#include "globals.h"
#include "progress.h"
#include "paths.h"

EFI_STATUS LoadSealedData(
    EFI_HANDLE imageHandle,
    const CHAR16* path,
    TPM2X_BLOB* blob)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    void* data = NULL;
    UINTN size = 0;

    if (!imageHandle || !path || !blob)
        goto done;

    if (EFILoadFile(imageHandle, path, &data, &size) != EFI_SUCCESS)
        goto done;

    if (TPM2X_DeserializeBlob(data, size, blob) != 0)
        goto done;

    status = EFI_SUCCESS;

done:

    if (data)
        Free(data);

    return status;
}


static EFI_STATUS _Get_SRK(
    EFI_TCG2_PROTOCOL* protocol,
    TPM_HANDLE* srkHandle,
    BOOLEAN* isHandleStatic)
{
    TPM2B_PUBLIC junk;
    TPM_RC rc;

    if (!protocol || !srkHandle || !isHandleStatic)
    {
        return EFI_INVALID_PARAMETER;
    }

    /* First, try the well known SRK key. If it doesn't exist, then create one. */
    rc = TPM2_ReadPublic(
            protocol,
            TPM2X_SRK_HANDLE,
            NULL,
            &junk);


    if (rc == TPM_RC_SUCCESS)
    {
        /* Found the SRK in TPM. Just return this handle. */
        *srkHandle = TPM2X_SRK_HANDLE;
        *isHandleStatic = TRUE;
        return EFI_SUCCESS;
    }


    if ((rc & ~TPM_RC_N_MASK) != TPM_RC_HANDLE)
    {
        /* Error wasn't due to a missing handle. */
        LOGE(L"UnsealKey(): SRK ReadPublic failed");
        return EFI_UNSUPPORTED;
    }


    /* Create the SRK key, since well known handle is unknown. */
    *isHandleStatic = FALSE;
    rc = TPM2X_CreateSRKKey(protocol, srkHandle);
    if (rc != TPM_RC_SUCCESS)
    {
        LOGE(L"UnsealKey(): SRK key creation failed");
        return EFI_UNSUPPORTED;
    }
    return EFI_SUCCESS;
}

EFI_STATUS UnsealKey(
    EFI_HANDLE imageHandle,
    EFI_TCG2_PROTOCOL* protocol,
    const TPM2X_BLOB* sealedData,
    UINT8** keyData,
    UINTN* keySize)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    TPM_HANDLE srkHandle;
    BOOLEAN srkHandleNull = TRUE;
    TPM_RC rc;
    Error err;
    UINT32 pcrMask = 0;
    typedef struct _Masterkey
    {
        BYTE data[TPM2X_MAX_DATA_SIZE];
        UINTN size;
    }
    Masterkey;
    Masterkey key;

    /* Check parametrers */
    if (!imageHandle || !protocol || !sealedData || !keyData || !keySize)
    {
        LOGE(L"UnsealKey(): null parameters");
        status = EFI_INVALID_PARAMETER;
        goto done;
    }

    *keyData = NULL;
    *keySize = 0;

    /* Clear the key */
    Memset(&key, 0, sizeof(Masterkey));

    /* Get the SRK key */
    status = _Get_SRK(protocol, &srkHandle, &srkHandleNull);
    if (status != EFI_SUCCESS)
    {
        LOGE(L"UnsealKey(): SRK get failed.");
        goto done;
    }

    /* Set the PCR mask: PCR[7,11] */
    pcrMask |= (1 << 7);
    pcrMask |= (1 << 11);

    /* Unseal the data */
    {
        UINT16 tmpSize = 0;

        if ((rc = TPM2X_Unseal(
            protocol, 
            pcrMask,
            srkHandle, 
            sealedData,
            key.data,
            &tmpSize,
            &err)) != TPM_RC_SUCCESS)
        {
            LOGE(L"UnsealKey(): failed to unseal key");
            goto done;
        }

        key.size = tmpSize;
    }

    /* Initialize the buffer */
    if (key.size > sizeof(key.data))
    {
        LOGE(L"UnsealKey(): unsealed data too big");
        goto done;
    }

    /* Clone the key */
    {
        if (!(*keyData = (UINT8*)Malloc(key.size)))
        {
            LOGE(L"UnsealKey(): allocation failed");
            goto done;
        }

        Memcpy(*keyData, key.data, key.size);
        *keySize = key.size;
    }

    status = EFI_SUCCESS;

done:

    if (srkHandleNull == FALSE)
        TPM2_FlushContext(protocol, srkHandle);

    Memset(&key, 0, sizeof(Masterkey));

    return status;
}

EFI_STATUS UnsealKeys(
    EFI_HANDLE imageHandle,
    EFI_TCG2_PROTOCOL* tcg2Protocol)
{
    EFI_STATUS status = EFI_UNSUPPORTED;

    /* Reject bad parameters */
    if (!imageHandle || !tcg2Protocol)
    {
        LOGW(L"%a(): invalid parameters", __FUNCTION__);
        goto done;
    }

    /* Load sealed blob into memory */
    if (LoadSealedData(
        imageHandle, 
        globals.sealedKeysPath, 
        &globals.sealedKeys) != EFI_SUCCESS)
    {
        LOGW(L"file not found: %s", globals.sealedKeysPath);
        goto done;
    }

    {
        CHAR16 path[PATH_MAX];
        DosToUnixPath(path, globals.sealedKeysPath);
        PutProgress(L"Unsealing %s", Wcs(path));
    }

    /* Unseal the keys file. */
    if (UnsealKey(
        imageHandle, 
        tcg2Protocol,
        &globals.sealedKeys,
        &globals.unsealedKeys,
        &globals.unsealedKeySize) != EFI_SUCCESS)
    {
        LOGW(L"failed to unseal %s", globals.sealedKeysPath);
        goto done;
    }

    if (SplitKeys(
        globals.unsealedKeys,
        globals.unsealedKeySize,
        &globals.bootkeyData,
        &globals.rootkeyData,
        &globals.bootkeySize,
        &globals.rootkeySize) != 0)
    {
        LOGW(L"Failed to split %s", globals.sealedKeysPath);
        goto done;
    }

    globals.bootkeyFound = TRUE;
    globals.rootkeyFound = TRUE;
    status = EFI_SUCCESS;

done:
    return status;
}

EFI_STATUS CapPCR(
    EFI_HANDLE imageHandle,
    EFI_TCG2_PROTOCOL* tcg2Protocol,
    UINT32 pcr)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    SHA256Hash pcrBefore;
    SHA256Hash pcrAfter;

    /* Reject bad parameters */
    if (!imageHandle || !tcg2Protocol)
    {
        LOGE(L"%a(): bad parameter", __FUNCTION__);
        goto done;
    }

    Memset(&pcrBefore, 0, sizeof(pcrBefore));
    Memset(&pcrAfter, 0, sizeof(pcrAfter));

    /* Read value of PCR[N] before capping */
    if (TPM2X_ReadPCRSHA256(tcg2Protocol, pcr, &pcrBefore) != TPM_RC_SUCCESS)
    {
        LOGE(L"Failed to read PCR[%d] before capping", pcr);
        goto done;
    }

    /* CAP PCR[N] */
    if (TPM2X_Cap(tcg2Protocol, pcr) != TPM_RC_SUCCESS)
    {
        LOGE(L"Failed to cap PCR[%d]", pcr);
        goto done;
    }

    /* Read value of PCR[N] after capping */
    if (TPM2X_ReadPCRSHA256(tcg2Protocol, pcr, &pcrAfter) != TPM_RC_SUCCESS)
    {
        LOGE(L"Failed to read PCR[%d] after capping", pcr);
        goto done;
    }

    /* If capping did not actually change PCR[N], then fail */
    if (Memcmp(&pcrBefore, &pcrAfter, sizeof(SHA256Hash)) == 0)
    {
        LOGE(L"Capping failed to change PCR[%d]", pcr);
        goto done;
    }

    status = EFI_SUCCESS;

done:
    return status;
}

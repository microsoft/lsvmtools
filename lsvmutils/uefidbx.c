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
#include "efivarauth.h"
#include "uefidbx.h"
#include <stdlib.h>
#include <stdio.h>
#include "guid.h"
#include "dump.h"
#include "file.h"
#include "measure.h"

//#define WIN_CERT_TYPE_PKCS_SIGNED_DATA 0x0002
//#define WIN_CERT_TYPE_EFI_PKCS115 0x0ef0
//#define WIN_CERT_TYPE_EFI_GUID 0x0ef1

#define EFI_CERT_SHA256_GUID  { 0xc1c41626, 0x504c, 0x4092, { 0xac, 0xa9, 0x41, 0xf9, 0x36, 0x93, 0x43, 0x28 } }

#define EFI_SIG_SHA256_GUID { 0x77FA9ABD , 0x0359, 0x4D32, { 0xBD, 0x60, 0x28, 0xF4, 0xE7, 0x8F, 0x78, 0x4B } }

#define PKCS7_GUID { 0x4aafd29d, 0x68df, 0x49ee, {0x8a, 0xa9, 0x34, 0x7d, 0x37, 0x56, 0x65, 0xa7} }

typedef struct _EFI_SIGNATURE_DATA
{
    EFI_GUID SignatureOwner;
    UINT8 SignatureData[0];
} 
EFI_SIGNATURE_DATA;

typedef struct _EFI_SIGNATURE_LIST
{
    EFI_GUID SignatureType;
    UINT32 SignatureListSize; /* size in bytes of entire list */
    UINT32 SignatureHeaderSize; /* size in bytes of the header */
    UINT32 SignatureSize; /* size in bytes of one signature */
}
EFI_SIGNATURE_LIST;

const void* SkipOptionalDBXHeader(
    const void* data,
    UINTN* size)
{
    if (data && size)
    {
        EFI_VARIABLE_AUTHENTICATION_2* p = (EFI_VARIABLE_AUTHENTICATION_2*)data;
        UINTN headerSize = p->AuthInfo.Hdr.dwLength + sizeof(EFI_TIME);
        static EFI_GUID guid = PKCS7_GUID;

        if (headerSize <= *size)
        {
            if (memcmp(&p->AuthInfo.CertType, &guid, sizeof(guid)) == 0)
            {
                (*size) -= headerSize;
                return data + headerSize;
            }
        }
    }

    return data;
}

int LoadDBXHashes(
    const void* data,
    UINTN size,
    SHA256Hash** hashesData,
    UINTN* hashesSize)
{
    int rc = -1;
    const UINT8* p;
    const UINT8* pend;
    static EFI_GUID certguid = EFI_CERT_SHA256_GUID;

    if (hashesSize)
        *hashesSize = 0;

    if (hashesData)
        *hashesData = NULL;

    if (!data || !size || !hashesData || !hashesSize)
        goto done;

    /* Skip optional EFI_VARIABLE_AUTHENTICATION_2 object */
    data = SkipOptionalDBXHeader(data, &size);

    /* Set pointer to data start and end */
    p = (UINT8*)data;
    pend = (UINT8*)data + size;

    /* Format: EFI_SIGNATURE_LIST HEADER SIGNATURE... */
    while (p < pend)
    {
        const EFI_SIGNATURE_LIST* list = (EFI_SIGNATURE_LIST*)p;

        /* If not enough space remaining for one more structure */
        if (p + sizeof(EFI_SIGNATURE_LIST) > pend)
            goto done;

        /* Sanity check */
        if (p + list->SignatureListSize > pend)
            goto done;

        /* If not a list of SHA-256 hashes, then skip it */
        if (memcmp(&certguid, &list->SignatureType, sizeof(certguid)) != 0)
        {
            /* Skip this signature list */
            p += list->SignatureListSize;
            continue;
        }

        /* Position p on first signature in the list */
        p += sizeof(EFI_SIGNATURE_LIST) + list->SignatureHeaderSize;

        if (p > pend)
            goto done;

        /* Iterate through the signatures */
        {
            UINTN i;
            UINTN m = list->SignatureListSize - 
                (sizeof(EFI_SIGNATURE_LIST) + list->SignatureHeaderSize);

            UINTN n = m / list->SignatureSize;

            for (i = 0; i < n; i++)
            {
                EFI_SIGNATURE_DATA* data = (EFI_SIGNATURE_DATA*)p;
                static EFI_GUID guid = EFI_SIG_SHA256_GUID;

                /* If a SHA-256 hash, then append to list */
                if (memcmp(&guid, &data->SignatureOwner, sizeof(guid)) == 0)
                {
                    UINTN r = list->SignatureSize - sizeof(EFI_GUID);

                    if (r != sizeof(SHA256Hash))
                        goto done;

                    if (AppendElem(
                        (void**)hashesData, 
                        hashesSize,
                        sizeof(SHA256Hash),
                        data->SignatureData) != 0)
                    {
                        goto done;
                    }
                }

                p += list->SignatureSize;
            }
        }
    }

    rc = 0;

done:

    if (rc != 0)
    {
        if (hashesData && *hashesData)
        {
            free(*hashesData);
            *hashesData = NULL;
            *hashesSize = 0;
        }
    }

    return rc;
}

static int _CompareHashes(const void * p1, const void * p2)
{
    SHA256Hash* h1 = (SHA256Hash*)p1;
    SHA256Hash* h2 = (SHA256Hash*)p2;
    return memcmp(h1, h2, sizeof(SHA256Hash));
}

int NeedDBXUpdate(
    const char* dbxupdatePath,
    BOOLEAN* need)
{
    int rc = -1;
    unsigned char* fileData = NULL;
    size_t fileSize;
    SHA256Hash* hashes1Data = NULL;
    UINTN hashes1Size;
    SHA256Hash* hashes2Data = NULL;
    UINTN hashes2Size;
    unsigned char* varData = NULL;
    UINTN varSize = 0;

    if (need)
        *need = FALSE;

    /* Reject null parameters */
    if (!dbxupdatePath || !need)
        goto done;

    /* Load the 'dbxupdate.bin' file */
    if (LoadFile(dbxupdatePath, 0, &fileData, &fileSize) != 0)
        goto done;

    /* Load the 'dbx' UEFI variable into memory */
    if (LoadEFIVar(
        "D719B2CB3D3A4596A3BCDAD00E67656F",
        "dbx",
        &varData,
        &varSize) != 0)
    {
        goto done;
    }

    /* Load the hashes from 'dbxupdate.bin' into memory */
    if (LoadDBXHashes(fileData, fileSize, &hashes1Data, &hashes1Size) != 0)
        goto done;

    /* Load the hashes from the 'dbx' variable into memory */
    if (LoadDBXHashes(fileData, fileSize, &hashes2Data, &hashes2Size) != 0)
        goto done;

    /* If number of hashes are different */
    if (hashes1Size != hashes2Size)
    {
        rc = 0;
        *need = TRUE;
        goto done;
    }

    /* Sort the hashes */
    qsort(hashes1Data, hashes1Size, sizeof(SHA256Hash), _CompareHashes);
    qsort(hashes2Data, hashes2Size, sizeof(SHA256Hash), _CompareHashes);

    /* Compare the hash arrays */
    if (memcmp(hashes1Data, hashes2Data, hashes1Size * sizeof(SHA256Hash)) != 0)
    {
        rc = 0;
        *need = TRUE;
        goto done;
    }

    rc = 0;

done:

    if (fileData)
        free(fileData);

    if (varData)
        free(varData);

    if (hashes1Data)
        free(hashes1Data);

    if (hashes2Data)
        free(hashes2Data);

    return rc;
}

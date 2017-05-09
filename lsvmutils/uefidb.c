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
#include <stdio.h>
#include "guid.h"
#include "uefidb.h"
#include "dump.h"
#include "buf.h"
#include "peimage.h"
#include "alloc.h"

static void _DumpSigList(const EFI_SIGNATURE_LIST* list)
{
    PRINTF0("START EFI SIGNATURE LIST\n");
    PRINTF0("Signature Type: ");
    DumpGUID(&list->SignatureType);
    PRINTF0("\n");
    PRINTF("Signature List Size: %u\n", list->SignatureListSize);
    PRINTF("Signature Header Size: %u\n", list->SignatureHeaderSize);
    PRINTF("Signature Size: %u\n\n", list->SignatureSize);
}

static void _DumpSigHeader(const UINT8* header, UINT32 size)
{
    PRINTF0("Signature Header:\n");
    HexDump(header, size);
    PRINTF0("\n");
}

static void _DumpSigData(const EFI_SIGNATURE_DATA* data, UINT32 size)
{
    PRINTF0("Signature Owner: ");
    DumpGUID(&data->SignatureOwner);
    PRINTF0("\nSignature Data:\n");
    HexDump(data->SignatureData, size - sizeof(EFI_GUID));
    PRINTF0("\n");
}

static int _SearchSignatures(
    const UINT8* in,
    const EFI_SIGNATURE_LIST* list,
    const void* binary,
    UINTN binarySize,
    UINT8** result,
    UINTN* resultSize,
    BOOLEAN quiet)
{          
    UINT32 sigDataSize;
    UINT32 i = 0;
    UINT32 numEntries;

    sigDataSize = list->SignatureListSize
                - sizeof(*list)
                - list->SignatureHeaderSize;

    /* If this happens, then there is an error in the efi database. */
    if (sigDataSize % list->SignatureSize != 0)
        return -1;

    numEntries = sigDataSize / list->SignatureSize;

    /* Read signature data entries. */
    for (i = 0; i < numEntries; i++)
    {
        int rc;
        EFI_SIGNATURE_DATA data;
        
        data.SignatureOwner = *((EFI_GUID*) in);
        data.SignatureData = (UINT8*) in + sizeof(data.SignatureOwner);
        if (!quiet)
            _DumpSigData(&data, list->SignatureSize);
        
        rc = CheckCert(
                binary,
                binarySize,
                data.SignatureData,
                list->SignatureSize - sizeof(EFI_GUID));
 
        /* Found a matching cert. Return 1 for found. */
        if (rc == 0)
        {
            *resultSize = list->SignatureSize;
            *result = (UINT8*)Malloc(*resultSize);
            if (*result == NULL)
            {
                *resultSize = 0;
                return -1;
            }
            memcpy(*result, in, *resultSize);
            return 1;
        }

        /* Otherwise continue searching. */
        in += list->SignatureSize; 
    }
    
    /* Not found. */
    return 0; 
}

int CheckImageDB(
    const void* database,
    UINTN databaseSize,
    const void* binary,
    UINTN binarySize,
    UINT8** result,
    UINTN* resultSize,
    BOOLEAN quiet)
{ 
    const UINT8* max;
    const UINT8* var;

    if (!database)
        return -1;
    
    var = (const UINT8*) database;
    max = (const UINT8*) database + databaseSize;

    while (var < max)
    {
        EFI_SIGNATURE_LIST* list;
        int rc;

        if (var + sizeof(EFI_SIGNATURE_LIST) > max)
            return -1;
 
        /* Read the signature list. */
        list = (EFI_SIGNATURE_LIST*) var;

        /* Do sanity checks for the list header. */
        if (var + list->SignatureListSize > max)
            return -1;
 
        if (list->SignatureHeaderSize +
            list->SignatureSize +
            sizeof(EFI_SIGNATURE_LIST) > list->SignatureListSize)
        {
            return -1;
        }

        if (!quiet)
            _DumpSigList(list);

        /* Read + dump the signature header. */
        if (!quiet)
            _DumpSigHeader(var + sizeof(*list), list->SignatureHeaderSize); 
        
        /* Finally, search through the signatures for a match. */
        rc = _SearchSignatures(var + sizeof(*list) + list->SignatureHeaderSize,
                              list,
                              binary,
                              binarySize,
                              result,
                              resultSize,
                              quiet);
        /* Return on either error or found match. */
        if (rc == 1)
            return 0;
        if (rc == -1)
            return -1;
   
        /* Go to next signature list. */
        var += list->SignatureListSize;
    }

    /* Not found. */
    *result = NULL;
    *resultSize = 0;
    return 0;
}

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
#include "eficommon.h"
#include "alloc.h"
#include "keys.h"
#include "strings.h"

const UINT8* _DeserializeKey(
    const UINT8* in,
    UINT8** bootkey,
    UINT8** rootkey,
    UINTN* bootkeySize,
    UINTN* rootkeySize)
{
    KEYS_SEALED_KEY_HEADER* keyHdr = (KEYS_SEALED_KEY_HEADER*) in;
    UINT8** key;
    UINTN* size;

    if (keyHdr->KeyType == KEYS_BOOTKEY_TYPE)
    {
        key = bootkey;
        size = bootkeySize;
    }
    else if (keyHdr->KeyType == KEYS_ROOTKEY_TYPE)
    {
        key = rootkey;
        size = rootkeySize;
    }
    else
    {
        return NULL;
    }

    *key = (UINT8*) Malloc(keyHdr->KeySize);
    if (*key == NULL)
    {
        return NULL;
    }

    Memcpy(*key, in + KEYS_SEALED_KEY_HEADER_SIZE, keyHdr->KeySize);
    *size = keyHdr->KeySize;
    return in + KEYS_SEALED_KEY_HEADER_SIZE + keyHdr->KeySize;
}

int SplitKeys(
    const UINT8* inData,
    UINTN inSize,
    UINT8** bootkey,
    UINT8** rootkey,
    UINTN* bootkeySize,
    UINTN* rootkeySize)
{
    KEYS_SEALED_HEADER* hdr;
    UINT8* bootkeyTmp = NULL;
    UINT8* rootkeyTmp = NULL;
    UINTN bootkeySizeTmp;
    UINTN rootkeySizeTmp;

    if (!inData || !bootkey || !rootkey || !bootkeySize || !rootkeySize)
    {
        goto Cleanup;
    }

    if (inSize < KEYS_SEALED_HEADER_SIZE)
    {
        goto Cleanup;
    }

    hdr = (KEYS_SEALED_HEADER*) inData;
    if (hdr->KeyCount != 2 ||
        inSize < KEYS_SEALED_HEADER_SIZE + 2 * KEYS_SEALED_KEY_HEADER_SIZE)
    {
        goto Cleanup;
    } 
    inData += KEYS_SEALED_HEADER_SIZE;

    inData = _DeserializeKey(inData, &bootkeyTmp, &rootkeyTmp, &bootkeySizeTmp, &rootkeySizeTmp);
    if (inData == NULL)
    {
        goto Cleanup;
    }

    inData = _DeserializeKey(inData, &bootkeyTmp, &rootkeyTmp, &bootkeySizeTmp, &rootkeySizeTmp);
    if (inData == NULL)
    {
        goto Cleanup;
    }


    // Keys not found.
    if (bootkeyTmp == NULL || rootkeyTmp == NULL)
    {  
        goto Cleanup;
    }

    *bootkey = bootkeyTmp;
    *rootkey = rootkeyTmp;
    *bootkeySize = bootkeySizeTmp;
    *rootkeySize = rootkeySizeTmp;
    return 0;

Cleanup:
    if (bootkey != NULL)
    {
        Free(bootkeyTmp);
    }
    if (rootkey != NULL)
    {
        Free(rootkeyTmp);
    }
    return -1;

}

UINT8* _SerializeKey(
    UINT8* in,
    UINT16 keyType,
    const UINT8* key,
    UINT16 keySize)
{
    KEYS_SEALED_KEY_HEADER* keyHdr;

    keyHdr = (KEYS_SEALED_KEY_HEADER*) in;
    keyHdr->KeyType = keyType;
    keyHdr->KeySize = keySize;
    Memcpy(in + KEYS_SEALED_KEY_HEADER_SIZE, key, keySize);
    in += KEYS_SEALED_KEY_HEADER_SIZE + keySize;

    return in;
}
    

int CombineKeys(
    const UINT8* bootkey,
    const UINT8* rootkey,
    UINT16 bootkeySize,
    UINT16 rootkeySize,
    UINT8** outData,
    UINTN* outDataSize)
{
    UINT8* outTmp;
    KEYS_SEALED_HEADER* hdr;

    if (!bootkey || !rootkey || !outData || !outDataSize)
    {
        return -1;
    }

    *outDataSize = KEYS_SEALED_HEADER_SIZE +
                   2 * KEYS_SEALED_KEY_HEADER_SIZE +
                   bootkeySize +
                   rootkeySize;

    *outData = (UINT8*) Malloc(*outDataSize);
    if (*outData == NULL)
    {
        return -1;
    }
    outTmp = *outData;

    hdr = (KEYS_SEALED_HEADER*) outTmp;
    hdr->KeyCount = 2;
    outTmp += KEYS_SEALED_HEADER_SIZE;

    outTmp = _SerializeKey(outTmp, KEYS_BOOTKEY_TYPE, bootkey, bootkeySize);
    outTmp = _SerializeKey(outTmp, KEYS_ROOTKEY_TYPE, rootkey, rootkeySize);
    return 0;
}

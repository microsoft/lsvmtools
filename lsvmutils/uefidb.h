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
#ifndef _uefidb_h_
#define _uefidb_h_

#include "eficommon.h"
#include "error.h"

/* Official UEFI structs. */
typedef struct _EFI_SIGNATURE_DATA
{
    EFI_GUID SignatureOwner;
    UINT8* SignatureData;
} EFI_SIGNATURE_DATA;

typedef struct _EFI_SIGNATURE_LIST
{
    EFI_GUID SignatureType;
    UINT32 SignatureListSize;
    UINT32 SignatureHeaderSize;
    UINT32 SignatureSize;
} EFI_SIGNATURE_LIST;

int CheckImageDB(
    const void* database,
    UINTN databaseSize,
    const void *binary,
    UINTN binarySize,
    UINT8** result,
    UINTN* resultSize,
    BOOLEAN quiet);

int DumpDBXUpdate(
    const void* data,
    UINTN size);

#endif /* _uefidb_h_ */

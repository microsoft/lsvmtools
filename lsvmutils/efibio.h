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
#ifndef _efibio_h
#define _efibio_h

#include "config.h"
#include <lsvmutils/eficommon.h>

#define BIO_MAGIC 0x99c3a120

typedef struct _EFI_BIO
{
    UINT32 magic;
    EFI_HANDLE imageHandle;
    EFI_HANDLE handle;
    EFI_BLOCK_IO* blockIO;
}
EFI_BIO;

typedef BOOLEAN (*MatchBIO)(
    EFI_BIO* bio,
    void* data);

EFI_BIO* OpenBIO(
    EFI_HANDLE imageHandle,
    MatchBIO match,
    void* matchData);

BOOLEAN ValidBIO(
    const EFI_BIO* bio);

EFI_STATUS CloseBIO(
    EFI_BIO* bio);

static __inline UINTN BlockSizeBIO(
    EFI_BIO* bio)
{
    if (ValidBIO(bio))
        return (UINTN)bio->blockIO->Media->BlockSize;

    return 0;
}

static __inline UINTN LastBlockBIO(
    const EFI_BIO* bio)
{
    if (ValidBIO(bio))
        return (UINTN)bio->blockIO->Media->LastBlock;

    return 0;
}

EFI_STATUS ReadBIO(
    EFI_BIO* bio, 
    UINTN blkno,
    void* data,
    UINTN size);

EFI_STATUS WriteBIO(
    EFI_BIO* bio, 
    UINTN blkno,
    const void* data,
    UINTN size);

EFI_STATUS LocateBlockIOHandles(
    EFI_HANDLE** handles,
    UINTN* numHandles);

EFI_STATUS OpenBlockIOProtocol(
    EFI_HANDLE imageHandle,
    EFI_HANDLE handle,
    EFI_BLOCK_IO** blockIO);

#endif /* _efibio_h */

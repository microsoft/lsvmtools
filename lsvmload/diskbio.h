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
#ifndef _rootbio_h
#define _rootbio_h

#include "config.h"
#include <lsvmutils/eficommon.h>
#include <lsvmutils/guid.h>
#include <lsvmutils/gpt.h>

#define BLOCK_SIZE 512

typedef struct _Block
{
    UINT8 data[BLOCK_SIZE];
}
Block;

int GetGPTEntry(
    UINTN partitionNumber,
    GPTEntry** entry);

typedef enum _RegionId
{
    REGION_ID_GPT,
    REGION_ID_ESP,
    REGION_ID_BOOT
}
RegionId;

/* Either 'blocks' or 'bio' must be null */
int AddRegion(
    RegionId id,
    UINT64 firstLBA, 
    UINT64 lastLBA, 
    UINT64 numBlocks,
    BOOLEAN readOnly,
    Block* blocks,
    EFI_BLOCK_IO* bio);

EFI_STATUS InstallRootBIO(
    EFI_HANDLE imageHandle);

int AddPartition(
    const CHAR16* name,
    EFI_GUID* guid,
    UINT64 firstLBA, 
    UINT64 lastLBA);

#if 0
EFI_STATUS InstallEFIBIO(
    EFI_HANDLE imageHandle);
#endif

#endif /* _rootbio_h */

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
#include <lsvmutils/alloc.h>
#include <lsvmutils/blkdev.h>
#include <lsvmutils/luksblkdev.h>
#include <lsvmutils/efiblkdev.h>
#include <lsvmutils/efibio.h>
#include <lsvmutils/guid.h>
#include <lsvmutils/gpt.h>
#include <lsvmutils/strings.h>
#include "devpath.h"
#include "luksbio.h"
#include "globals.h"
#include "trace.h"
#include "logging.h"
#include "console.h"
#include "diskbio.h"
#include "logging.h"
#include "initrd.h"

#define BLOCK_SIZE 512

#define MAX_REGIONS 8

typedef union _U
{
    GPT gpt;
    Block blocks[64];
}
U;

/* Cached GPT sectors */
static U _u;

int GetGPTEntry(
    UINTN partitionNumber,
    GPTEntry** entry)
{
    int rc = -1;
    UINTN i;

    if (!entry)
        goto done;

    *entry = NULL;

    for (i = 0; i < GPT_MAX_ENTRIES; i++)
    {
        GPTEntry* e = &_u.gpt.entries[i];

        /* If end of entries */
        if (e->typeGUID1 == 0)
            break;

        /* If this is the partition we are looking for */
        if (partitionNumber == i + 1)
        {
            *entry = e;
            break;
        }
    }

    /* If found */
    if (*entry)
        rc = 0;

done:
    return rc;
}

static EFI_BLOCK_IO* _bio;

int AddPartition(
    const CHAR16* name,
    EFI_GUID* guid,
    UINT64 firstLBA, 
    UINT64 lastLBA)
{
    UINTN i;

    for (i = 1; i < GPT_MAX_ENTRIES; i++)
    {
        GPTEntry* entry = &_u.gpt.entries[i];

        /* If the entry in this slot is available */
        if (entry->typeGUID1 == 0)
        {
            /* Copy the previous entry to pick up type GUID */
            Memcpy(entry, &_u.gpt.entries[i-1], sizeof(GPTEntry));

            /* Set the guid */
            SplitGUID(guid, &entry->uniqueGUID1, &entry->uniqueGUID2);

            /* Set the first and last block numbers */
            entry->startingLBA = firstLBA;
            entry->endingLBA = lastLBA;

            /* Set the name */
            Wcslcpy(entry->typeName, name, sizeof(entry->typeName));

            /* Increase size of disk if necessary */
            if (lastLBA > _u.gpt.header.lastUsableLBA)
            {
                _u.gpt.header.lastUsableLBA = lastLBA;
                _bio->Media->LastBlock = lastLBA;
            }

            break;
        }
    }

    return 0;
}

/* Cached regions */
typedef struct _Region
{
    RegionId id;
    UINT64 firstLBA;
    UINT64 lastLBA;
    UINT64 numBlocks;
    BOOLEAN readOnly;
    Block* blocks;
    EFI_BLOCK_IO* bio; /* if non-null, use it to get blocks */
}
Region;

static Region _regions[MAX_REGIONS];
static UINTN _nregions;

int AddRegion(
    RegionId id,
    UINT64 firstLBA, 
    UINT64 lastLBA, 
    UINT64 numBlocks, 
    BOOLEAN readOnly,
    Block* blocks,
    EFI_BLOCK_IO* bio)
{
    if (_nregions == MAX_REGIONS)
        return -1;

    if (blocks && bio)
        return -1;

    if (!blocks && !bio)
        return -1;

    _regions[_nregions].id = id;
    _regions[_nregions].firstLBA = firstLBA;
    _regions[_nregions].lastLBA = lastLBA;
    _regions[_nregions].numBlocks = numBlocks;
    _regions[_nregions].lastLBA = lastLBA;
    _regions[_nregions].readOnly = readOnly;
    _regions[_nregions].blocks = blocks;
    _regions[_nregions].bio = bio;
    _nregions++;

    if (lastLBA > _u.gpt.header.lastUsableLBA)
    {
        _u.gpt.header.lastUsableLBA = lastLBA;
        _bio->Media->LastBlock = lastLBA;
    }

    return 0;
}

static Region* _FindRegion(UINT64 lba)
{
    UINTN i;

    for (i = 0; i < _nregions; i++)
    {
        const Region* reg = &_regions[i];

        if (lba >= reg->firstLBA && lba <= reg->lastLBA)
            return &_regions[i];
    }

    /* Not found! */
    return NULL;
}

/* Pointers to saved EFI_BLOCK_IO functions */
static EFI_BLOCK_RESET _EFI_BLOCK_IO_Reset;
static EFI_BLOCK_READ _EFI_BLOCK_IO_ReadBlocks;
static EFI_BLOCK_WRITE _EFI_BLOCK_IO_WriteBlocks;
static EFI_BLOCK_FLUSH _EFI_BLOCK_IO_FlushBlocks;

static EFI_STATUS EFIAPI _EFI_BLOCK_IO_ResetHook(
    IN struct _EFI_BLOCK_IO *this,
    IN BOOLEAN ExtendedVerification)
{
    return _EFI_BLOCK_IO_Reset(this, ExtendedVerification);
}

static EFI_STATUS EFIAPI _EFI_BLOCK_IO_ReadBlocksHook(
    IN struct _EFI_BLOCK_IO *this,
    IN UINT32 mediaId,
    IN EFI_LBA lba,
    IN UINTN bufferSize,
    OUT VOID *buffer)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    const Region* reg;
    BOOLEAN enableIOHooks = globals.enableIOHooks;
    UINT8* ptr = (UINT8*)buffer;
    UINTN rem = bufferSize;

    if (!globals.enableIOHooks)
        return _EFI_BLOCK_IO_ReadBlocks(this, mediaId, lba, rem, ptr);

    globals.enableIOHooks = FALSE;

#if 0
    {
        static UINTN count = 0;

        if (count < 64)
        {
            UINT32 partno = LBAToParitionNumber(&_u.gpt, lba);
            LOGI(L"READ: /dev/sda%d lba=%ld bufferSize=%ld", 
                partno, lba, bufferSize);
            count++;
        }
    }
#endif

    if ((reg = _FindRegion(lba)))
    {
        UINTN i;

        for (i = lba; i <= reg->lastLBA && rem > 0; i++)
        {
            Block block;
            EFI_LBA localLBA = i - reg->firstLBA;

            if (reg->bio)
            {
                /* Read the next block */
                if (reg->bio->ReadBlocks(
                    reg->bio,
                    reg->bio->Media->MediaId,
                    localLBA,
                    sizeof(Block),
                    &block) != EFI_SUCCESS)
                {
                    LOGE(L"ReadBlocks() failed: 1");
                    goto done;
                }
            }
            else
            {
                if (localLBA < reg->numBlocks)
                    Memcpy(&block, &reg->blocks[localLBA], sizeof(Block));
                else
                    Memset(&block, 0, sizeof(Block));
            }

            /* Copy block to caller's buffer */
            {
                UINTN n = sizeof(Block);

                if (n > rem)
                    n = rem;

                Memcpy(ptr, block.data, n);
                ptr += n;
                rem -= n;
            }
        }

        /* Fallthrough to read anything that remains */
        lba = i;
    }

    /* Delegate to original ReadBlocks() implementation */
    if (rem)
    {
        if (_EFI_BLOCK_IO_ReadBlocks(
            this, 
            mediaId, 
            lba, 
            rem, 
            ptr) != EFI_SUCCESS)
        {
            goto done;
        }
    }

    status = EFI_SUCCESS;

done:
    globals.enableIOHooks = enableIOHooks;
    return status;
}

static EFI_STATUS EFIAPI _EFI_BLOCK_IO_WriteBlocksHook(
    IN struct _EFI_BLOCK_IO *this,
    IN UINT32 mediaId,
    IN EFI_LBA lba,
    IN UINTN bufferSize,
    IN VOID *voidBuffer)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    UINT8* buffer = (UINT8*)voidBuffer;
    Region* reg;
    BOOLEAN enableIOHooks = globals.enableIOHooks;

    if (!globals.enableIOHooks)
        return _EFI_BLOCK_IO_WriteBlocks(this, mediaId, lba, bufferSize, buffer);

    globals.enableIOHooks = FALSE;

#if 0
    {
        UINT32 partno = LBAToParitionNumber(&_u.gpt, lba);
        LOGI(L"WRITE: /dev/sda%d lba=%ld bufferSize=%ld", 
            partno, lba, bufferSize);
    }
#endif

    if ((reg = _FindRegion(lba)))
    {
        UINTN i;

        if (reg->readOnly)
        {
            status = EFI_WRITE_PROTECTED;
            goto done;
        }

        for (i = lba; i <= reg->lastLBA && bufferSize > 0; i++)
        {
            Block block;
            EFI_LBA localLBA = i - reg->firstLBA;

            /* Copy caller's buffer onto block */
            {
                UINTN n = sizeof(Block);

                if (n > bufferSize)
                    n = bufferSize;

                Memset(block.data, 0, sizeof(Block));
                Memcpy(block.data, buffer, n);
                buffer += n;
                bufferSize -= n;
            }

            /* Perform write: onto BIO or buffer */
            if (reg->bio)
            {
                /* Write the next block */
                if (reg->bio->WriteBlocks(
                    reg->bio,
                    reg->bio->Media->MediaId,
                    localLBA,
                    sizeof(Block),
                    &block) != EFI_SUCCESS)
                {
                    LOGE(L"WriteBlocks() failed 1: lba=%d", (int)i);
                    goto done;
                }

#if 0
                LOGI(L"WRITE: lba=%d localLBA=%d", (int)i, (int)localLBA);
                LogASCIIStr(L"WRITE", (UINT8*)&block, sizeof(Block));
                LogHexStr(L"WRITE", (UINT8*)&block, sizeof(Block));
#endif
            }
            else
            {
                if (localLBA < reg->numBlocks)
                    Memcpy(&reg->blocks[localLBA], &block, sizeof(Block));
                else
                {
                    /* Ignore excess writes */
                }
            }
        }

        /* Fallthrough to write anything that remains */
        lba = i;
    }

    /* Delegate to original ReadBlocks() implementation */
    if (bufferSize)
    {
        if (_EFI_BLOCK_IO_WriteBlocks(
            this, 
            mediaId, 
            lba, 
            bufferSize, 
            buffer) != EFI_SUCCESS)
        {
            LOGE(L"WriteBlocks() failed 2: lba=%d", (int)lba);
            goto done;
        }
    }

    status = EFI_SUCCESS;

done:
    globals.enableIOHooks = enableIOHooks;
    return status;
}

static EFI_STATUS EFIAPI _EFI_BLOCK_IO_FlushBlocksHook(
    IN struct _EFI_BLOCK_IO *this)
{
    return _EFI_BLOCK_IO_FlushBlocks(this);
}

/* Find handle of root device (e.g., /dev/sda */
EFI_HANDLE FindHandle(
    EFI_HANDLE imageHandle,
    int pun,
    int lun,
    int part)
{
    EFI_HANDLE* result = NULL;
    EFI_HANDLE* handles = NULL;
    UINTN numHandles = 0;
    UINTN i;

    if (!imageHandle)
        goto done;

    if (LocateBlockIOHandles(&handles, &numHandles) != EFI_SUCCESS)
        goto done;

    for (i = 0; i < numHandles; i++)
    {
        EFI_DEVICE_PATH* dp = DevicePathFromHandle(handles[i]);
        EFI_DEVICE_PATH* p = dp;
        int tmpPun = -1;
        int tmpLun = -1;
        int tmpPart = 0;

        for (; p; p = (EFI_DEVICE_PATH*)DevNodeNext(p))
        {
            if (IsSCSINode(p))
            {
                SCSIDevicePathPacked tmp;
                Memcpy(&tmp, p, sizeof(tmp));
                tmpPun = tmp.pun;
                tmpLun = tmp.lun;
            }
            else if (IsHardDriveNode(p))
            {
                HardDriveDevicePathPacked tmp;
                Memcpy(&tmp, p, sizeof(tmp));
                tmpPart = tmp.partitionNumber;
            }
        }

        if (pun == tmpPun && lun == tmpLun && part == tmpPart)
        {
            result = handles[i];
            break;
        }
    }

done:
    return result;
}

EFI_STATUS InstallRootBIO(
    EFI_HANDLE imageHandle)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    EFI_HANDLE handle = NULL;
    EFI_BLOCK_IO* bio = NULL;

    if (!imageHandle)
        goto done;

    /* Find SCSI(0,0) */
    if (!(handle = FindHandle(imageHandle, 0, 0, 0)))
    {
        LOGE(L"root bio handle not found");
        goto done;
    }

    /* Open the existing bio */
    if (OpenBlockIOProtocol(imageHandle, handle, &bio) != EFI_SUCCESS)
    {
        LOGE(L"failed to open root bio");
        goto done;
    }

    if (!bio)
    {
        LOGE(L"root bio is null");
        goto done;
    }

    /* Save old function pointers */
    _EFI_BLOCK_IO_Reset = bio->Reset;
    _EFI_BLOCK_IO_ReadBlocks = bio->ReadBlocks;
    _EFI_BLOCK_IO_WriteBlocks = bio->WriteBlocks;
    _EFI_BLOCK_IO_FlushBlocks = bio->FlushBlocks;

    /* Install hooks */
    bio->Reset = _EFI_BLOCK_IO_ResetHook;
    bio->ReadBlocks = _EFI_BLOCK_IO_ReadBlocksHook;
    bio->WriteBlocks = _EFI_BLOCK_IO_WriteBlocksHook;
    bio->FlushBlocks = _EFI_BLOCK_IO_FlushBlocksHook;

    /* Read the GPT into memory (LBA 0) */
    if (_EFI_BLOCK_IO_ReadBlocks(
        bio, 
        bio->Media->MediaId, 
        0, /* lba */
        sizeof(_u.gpt),
        &_u.gpt) != EFI_SUCCESS)
    {
        LOGE(L"failed to read GPT");
        goto done;
    }

    /* Add GPT to list of regions */
    {
        const UINT64 firstRegion = 0;
        const UINT64 lastRegion = (sizeof(_u) / BLOCK_SIZE) - 1;

        if (AddRegion(
            REGION_ID_GPT,
            firstRegion, 
            lastRegion, 
            lastRegion - firstRegion + 1,
            TRUE, /* readOnly */
            _u.blocks, NULL) != 0)
        {
            LOGE(L"AddRegion() faeild");
            goto done;
        }
    }

    _bio = bio;

    status = EFI_SUCCESS;

done:
    return status;
}

#if 0
static EFI_BLOCK_READ _EFI_BLOCK_IO_ReadBlocks2;

static EFI_STATUS EFIAPI _EFI_BLOCK_IO_ReadBlocks2Hook(
    IN struct _EFI_BLOCK_IO *this,
    IN UINT32 mediaId,
    IN EFI_LBA lba,
    IN UINTN bufferSize,
    OUT VOID *buffer)
{
    EFI_STATUS status;
    BOOLEAN enableIOHooks = globals.enableIOHooks;

    globals.enableIOHooks = FALSE;

    LOGI(L"_EFI_BLOCK_IO_ReadBlocks2Hook: %d", (int)lba);

    status = _EFI_BLOCK_IO_ReadBlocks2(this, mediaId, lba, bufferSize, buffer);

    globals.enableIOHooks = enableIOHooks;

    return status;
}

EFI_STATUS InstallEFIBIO(
    EFI_HANDLE imageHandle)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    EFI_HANDLE handle = NULL;
    EFI_BLOCK_IO* bio = NULL;

    if (!imageHandle)
        goto done;

    /* Find SCSI(0,0) */
    if (!(handle = FindHandle(imageHandle, 0, 0, 1)))
    {
        LOGE(L"root bio handle not found");
        goto done;
    }

    /* Open the existing bio */
    if (OpenBlockIOProtocol(imageHandle, handle, &bio) != EFI_SUCCESS)
    {
        LOGE(L"failed to open EFI bio");
        goto done;
    }

    if (!bio)
    {
        LOGE(L"EFI bio is null");
        goto done;
    }

    _EFI_BLOCK_IO_ReadBlocks2 = bio->ReadBlocks;
    bio->ReadBlocks = _EFI_BLOCK_IO_ReadBlocks2Hook;

#if 0
    /* Save old function pointers */
    _EFI_BLOCK_IO_Reset = bio->Reset;
    _EFI_BLOCK_IO_ReadBlocks = bio->ReadBlocks;
    _EFI_BLOCK_IO_WriteBlocks = bio->WriteBlocks;
    _EFI_BLOCK_IO_FlushBlocks = bio->FlushBlocks;

    /* Install hooks */
    bio->Reset = _EFI_BLOCK_IO_ResetHook;
    bio->ReadBlocks = _EFI_BLOCK_IO_ReadBlocksHook;
    bio->WriteBlocks = _EFI_BLOCK_IO_WriteBlocksHook;
    bio->FlushBlocks = _EFI_BLOCK_IO_FlushBlocksHook;
#endif

    status = EFI_SUCCESS;

done:
    return status;
}
#endif

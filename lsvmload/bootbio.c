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
#include "bootbio.h"
#include <lsvmutils/alloc.h>
#include <lsvmutils/blkdev.h>
#include <lsvmutils/luksblkdev.h>
#include <lsvmutils/efiblkdev.h>
#include <lsvmutils/efibio.h>
#include "luksbio.h"
#include "globals.h"
#include "trace.h"
#include "devpath.h"
#include "diskbio.h"
#include "logging.h"
#include "bootfs.h"
#include "initrd.h"

typedef struct _BlockIO
{
    EFI_BLOCK_IO base;
    EFI_BLOCK_IO_MEDIA media;
    Blkdev* dev;
}
BlockIO;

static EFI_STATUS EFIAPI _EFI_BLOCK_IO_Reset(
    IN struct _EFI_BLOCK_IO *this,
    IN BOOLEAN ExtendedVerification)
{
    return EFI_SUCCESS;
}

static EFI_STATUS EFIAPI _EFI_BLOCK_IO_ReadBlocks(
    IN struct _EFI_BLOCK_IO *this,
    IN UINT32 mediaId,
    IN EFI_LBA lba,
    IN UINTN bufferSize,
    OUT VOID *buffer)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    BlockIO* impl = (BlockIO*)this;

    if (!impl || !buffer)
        goto done;

    if (BlkdevRead(impl->dev, lba, buffer, bufferSize) != 0)
        goto done;

#if 0
    if (bufferSize >= BLKDEV_BLKSIZE)
    {
        SHA1Hash sha1;
        char path[PATH_MAX];

        ComputeSHA1(buffer, BLKDEV_BLKSIZE, &sha1);

        if (MatchInitrdPathByHash(&sha1, path) == 0)
        {
            LOGI(L"Matched1: lba=%ld path=%a", (unsigned long)lba, Str(path));
        }

        if (MatchInitrdPathByLBA(lba, path) == 0)
        {
            LOGI(L"Matched2: lba=%ld path=%a", (unsigned long)lba, Str(path));
        }
    }
#endif

    status = EFI_SUCCESS;

done:
    return status;
}

static EFI_STATUS EFIAPI _EFI_BLOCK_IO_WriteBlocks(
    IN struct _EFI_BLOCK_IO *this,
    IN UINT32 mediaId,
    IN EFI_LBA lba,
    IN UINTN bufferSize,
    IN VOID *buffer)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    BlockIO* impl = (BlockIO*)this;

    if (!impl || !buffer)
        goto done;

    if (BlkdevWrite(impl->dev, lba, buffer, bufferSize) != 0)
        goto done;

    status = EFI_SUCCESS;

done:
    return status;
}

static EFI_STATUS EFIAPI _EFI_BLOCK_IO_FlushBlocks(
    IN struct _EFI_BLOCK_IO *this)
{
    return EFI_SUCCESS;
}

static BlockIO _block_io =
{
    {
        1, /* Revision */
        NULL, /* Media */
        _EFI_BLOCK_IO_Reset,
        _EFI_BLOCK_IO_ReadBlocks,
        _EFI_BLOCK_IO_WriteBlocks,
        _EFI_BLOCK_IO_FlushBlocks,
    },
};

EFI_STATUS WrapBootBIO(
    EFI_HANDLE imageHandle,
    EFI_TCG2_PROTOCOL* tcg2Protocol,
    const UINT8* masterkeyData,
    UINTN masterkeySize)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    EFI_STATUS rc;
    EFI_GUID guid;
    EFI_DEVICE_PATH* dp = NULL;
    UINT64 firstLBA = 0;
    UINT64 lastLBA = 0;
    EFI_HANDLE handle = NULL;
    Blkdev* bootdev = NULL;

    /* Check parameters */
    if (!imageHandle || !masterkeyData || !masterkeySize)
        goto done;

    /* Get boot device */
    if (!(bootdev = GetBootDevice(
        imageHandle, 
        tcg2Protocol,
        masterkeyData,
        masterkeySize)))
    {
        LOGE(L"WrapBootBIO(): GetBootDevice() failed");
        goto done;
    }

    /* Copy over media from LUKS parition */
    Memcpy(
        &_block_io.media, 
        globals.bootbio->blockIO->Media, 
        sizeof(EFI_BLOCK_IO_MEDIA));
    _block_io.base.Media = &_block_io.media;

    /* Set EFI_BLOCK_IO_MEDIA.LastBlock */
    {
        union 
        {
            LUKSHeader header;
            UINT8 blocks[LUKS_SECTOR_SIZE*2];
        }
        u;
        static UINT8 _magic[LUKS_MAGIC_SIZE] = LUKS_MAGIC_INITIALIZER;

        /* Read the LUKS header (blkno == 0) */
        if (ReadBIO(globals.bootbio, 0, &u, sizeof(u)) != EFI_SUCCESS)
        {
            LOGE(L"WrapBootBIO(): ReadBIO() failed");
            goto done;
        }

        /* Check the magic number */
        if (Memcmp(u.header.magic, _magic, sizeof(_magic)) != 0)
        {
            LOGE(L"WrapBootBIO(): bad LUK magic"); 
            goto done;
        }

        /* Fix the byte order of the hader */
        LUKSFixByteOrder(&u.header);

        /* Adjust the last block field (omit leading LUKS metadata) */
        _block_io.media.LastBlock -= u.header.payload_offset;
    }

    /* Set the function pointers */
    _block_io.base.Reset = _EFI_BLOCK_IO_Reset;
    _block_io.base.ReadBlocks = _EFI_BLOCK_IO_ReadBlocks;
    _block_io.base.WriteBlocks = _EFI_BLOCK_IO_WriteBlocks;
    _block_io.base.FlushBlocks = _EFI_BLOCK_IO_FlushBlocks;
    _block_io.dev = bootdev;

#if 0
    /* Use the guid from the boot file system */
    if (globals.bootfs)
    {
        LogHexStr(L"EXT2-GUID", globals.bootfs->sb.s_uuid,
            sizeof(globals.bootfs->sb.s_uuid));
        MakeGUIDFromBytes(&guid, globals.bootfs->sb.s_uuid);
    }
#endif

    /* Allocate device path for this new parition */
    if (!(dp = DevPathCreatePseudoPartition(
        _block_io.media.LastBlock,
        &guid,
        &firstLBA,
        &lastLBA)))
    {
        LOGE(L"WrapBootBIO(): DevPathCreatePseudoPartition() failed");
        goto done;
    }

    LOGI(L"Mapped pseudo partition: firstLBA=%d lastLBA=%d",
        (int)firstLBA, (int)lastLBA);

    /* Install new EFI_BLOCK_IO (assigns new handle) */
    {
        static EFI_GUID protocol = BLOCK_IO_PROTOCOL;

        if ((rc = uefi_call_wrapper(
            BS->InstallProtocolInterface, 
            4, 
            &handle,
            &protocol,
            EFI_NATIVE_INTERFACE,
            &_block_io)) != EFI_SUCCESS)
        {
            LOGE(L"WrapBootBIO(): InstallProtocolInterface(1) failed");
            goto done;
        }
    }

    /* Install new DEVICE_PATH protocol interface */
    {
        static EFI_GUID protocol = DEVICE_PATH_PROTOCOL;

        if ((rc = uefi_call_wrapper(
            BS->InstallProtocolInterface, 
            4, 
            &handle,
            &protocol,
            EFI_NATIVE_INTERFACE,
            dp)) != EFI_SUCCESS)
        {
            LOGE(L"WrapBootBIO(): InstallProtocolInterface(2) failed");
            goto done;
        }
    }

    /* Add this new parition to the GPT */
    if (AddPartition(L"BOOTFS", &guid, firstLBA, lastLBA) != 0)
    {
        LOGE(L"WrapBootBIO(): AddPartition() failed");
        goto done;
    }

    /* Add a region so diskbio will find this data */
    if (AddRegion(
        REGION_ID_BOOT,
        firstLBA, 
        lastLBA, 
        lastLBA - firstLBA + 1,
        FALSE, /* readOnly */
        NULL, 
        &_block_io.base) != 0)
    {
        LOGE(L"WrapBootBIO(): AddRegion() failed");
        goto done;
    }

    status = EFI_SUCCESS;

done:

    return status;
}

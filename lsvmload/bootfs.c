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
#include "config.h"
#include <lsvmutils/eficommon.h>
#include "bootfs.h"
#include <lsvmutils/alloc.h>
#include "console.h"
#include <lsvmutils/ext2.h>
#include <lsvmutils/tpm2.h>
#include <lsvmutils/sha.h>
#include <lsvmutils/efiblkdev.h>
#include <lsvmutils/efibio.h>
#include <lsvmutils/luksblkdev.h>
#include <lsvmutils/efifile.h>
#include <lsvmutils/cacheblkdev.h>
#include "luksbio.h"
#include "paths.h"
#include "strings.h"
#include "log.h"

Blkdev* GetBootDevice(
    EFI_HANDLE imageHandle,
    EFI_TCG2_PROTOCOL* tcg2Protocol,
    const UINT8* masterkeyData,
    UINTN masterkeySize)
{
    EFI_BIO* bio = NULL;
    Blkdev* rawdev = NULL;
    Blkdev* bootdev = NULL;
    Blkdev* cachedev = NULL;

    /* bootdev -> cachedev -> rawdev -> bio */

    if (globals.bootdev)
        return globals.bootdev;

    /* Open 'LUKS BIO' */
    if (!(bio = OpenLUKSBIO(imageHandle, globals.bootDevice)))
    {
        LOGI(L"No LUKS BIO found");
        goto done;
    }

    /* Wrap 'LUKS BIO' in 'raw device' */
    if (!(rawdev = BlkdevFromBIO(bio)))
    {
        LOGE(L"BlkdevFromBIO() failed");
        CloseBIO(bio);
        goto done;
    }

    /* Wrap 'raw device' in 'cache device' */
    if (!(cachedev = NewCacheBlkdev(rawdev)))
    {
        LOGE(L"NewCacheBlkdev() failed");
        rawdev->Close(rawdev);
        goto done;
    }

#if 0
    /* Enable caching to prevent writes to boot partition */
    /* cachedev->SetFlags(cachedev, BLKDEV_ENABLE_CACHING); */
#endif

    /* Wrap 'cache device' in 'LUKS device' */
    if (!(bootdev = LUKSBlkdevFromRawBytes(
        cachedev, 
        masterkeyData,
        masterkeySize)))
    {
        LOGE(L"LUKSBlkdevNew() failed: LUKSBlkdevFromPassphrase2()");
        cachedev->Close(cachedev);
        goto done;
    }

    globals.bootbio = bio;
    globals.cachedev = cachedev;
    globals.bootdev = bootdev;

done:

    return bootdev;
}

EXT2* OpenBootFS(
    EFI_HANDLE imageHandle,
    EFI_TCG2_PROTOCOL* tcg2Protocol,
    const UINT8* masterkeyData,
    UINTN masterkeySize)
{
    Blkdev* bootdev = NULL;
    EXT2* ext2 = NULL;

    /* Get the boot device (sets globals.bootdev) */
    if (!(bootdev = GetBootDevice(
        imageHandle, 
        tcg2Protocol,
        masterkeyData,
        masterkeySize)))
    {
        LOGE(L"GetBootDevice() failed");
        goto done;
    }

    /* Open the EXT2 file system */
    if (EXT2New(bootdev, &ext2) != EXT2_ERR_NONE)
    {
        LOGE(L"EXT2New() failed");
        goto done;
    }

done:

    return ext2;
}

EFI_STATUS LoadFileFromBootFS(
    EFI_HANDLE imageHandle,
    EFI_TCG2_PROTOCOL* tcg2Protocol,
    EXT2* bootfs,
    const CHAR16* wcspath,
    void** dataOut,
    UINTN* sizeOut)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    char path[PATH_SIZE];
    void *data = NULL;
    UINT32 size = 0;
    EXT2Inode inode;

    /* Check parameters */
    if (!wcspath || !dataOut || !sizeOut)
    {
        LOGE(L"%a(): bad parameters", __FUNCTION__);
        status = EFI_INVALID_PARAMETER;
        goto done;
    }

    /* Convert path to char type */
    if (StrWcslcpy(path, wcspath, PATH_SIZE) >= PATH_SIZE)
    {
        LOGE(L"%a(): path overflow", __FUNCTION__);
        goto done;
    }

    /* Lookup inode for this path */
    if (EXT2PathToInode(bootfs, path, NULL, &inode) != EXT2_ERR_NONE)
    {
        LOGD(L"%a(): file not found: %a", __FUNCTION__, path);
        goto done;
    }

    /* Load the file into memory */
    if (EXT2LoadFileFromInode(
        bootfs, 
        &inode, 
        &data, 
        &size) != EXT2_ERR_NONE)
    {
        LOGE(L"%a(): failed to load file from inode", __FUNCTION__);
        goto done;
    }

    /* Set output parameters */
    *dataOut = data;
    *sizeOut = size;

    status = EFI_SUCCESS;

done:

    if (status != EFI_SUCCESS)
    {
        if (data)
            Free(data);
    }

    return status;
}

EFI_STATUS RemoveFileFromBootFS(
    EFI_HANDLE imageHandle,
    EFI_TCG2_PROTOCOL* tcg2Protocol,
    EXT2* bootfs,
    const CHAR16* wcspath)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    char path[PATH_SIZE];

    /* Check parameters */
    if (!imageHandle || !tcg2Protocol || !bootfs || !wcspath)
    {
        LOGE(L"%a(): bad parameters", __FUNCTION__);
        status = EFI_INVALID_PARAMETER;
        goto done;
    }

    /* Convert path to char type */
    if (StrWcslcpy(path, wcspath, PATH_SIZE) >= PATH_SIZE)
    {
        LOGE(L"%a(): path overflow", __FUNCTION__);
        goto done;
    }

    if (EXT2Rm(bootfs, path) != EXT2_ERR_NONE)
    {
        LOGE(L"%a(): cannot remove file", __FUNCTION__);
        goto done;
    }

    status = EFI_SUCCESS;

done:

    return status;
}

#if 0
EFI_STATUS PutFileToBootFS(
    EFI_HANDLE imageHandle,
    EFI_TCG2_PROTOCOL* tcg2Protocol,
    EXT2* bootfs,
    const CHAR16* wcspath,
    const void* data,
    UINTN size)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    char path[PATH_SIZE];

    /* Check parameters */
    if (!imageHandle || !tcg2Protocol || !bootfs || !wcspath || !data)
    {
        LOGE(L"PutFileToBootFS(): bad parameters");
        status = EFI_INVALID_PARAMETER;
        goto done;
    }

    /* Convert path to char type */
    if (StrWcslcpy(path, wcspath, PATH_SIZE) >= PATH_SIZE)
    {
        LOGE(L"PutFileToBootFS(): path overflow");
        goto done;
    }

    status = EFI_SUCCESS;

done:

    return status;
}
#endif

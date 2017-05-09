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
#include "luksblkdev.h"
#include "luks.h"
#include "alloc.h"
#include "strings.h"
#include "print.h"

#define LUKSBLKDEV_MAGIC 0x5acdeed9

typedef struct _BlkdevImpl BlkdevImpl;

struct _BlkdevImpl
{
    Blkdev base;
    UINT32 magic;
    LUKSHeader header;
    Blkdev* rawdev; /* underlying raw LUKS device */
    UINT8* masterkey; /* size is header->key_bytes */
};

static BOOLEAN _ValidLUKSBlkdev(
    Blkdev* dev)
{
    BlkdevImpl* impl = (BlkdevImpl*)dev;
    return impl != NULL && impl->magic == LUKSBLKDEV_MAGIC;
}

static int _Close(
    Blkdev* dev)
{
    int rc = -1;
    BlkdevImpl* impl = (BlkdevImpl*)dev;

    if (!_ValidLUKSBlkdev(dev))
        goto done;

    if (impl->masterkey)
        Free(impl->masterkey);

    if (impl->rawdev)
        impl->rawdev->Close(impl->rawdev);

    Free(impl);

    rc = 0;

done:
    return rc;
}

static int _Get(
    Blkdev* dev,
    UINTN blkno,
    void* data)
{
    int rc = -1;
    BlkdevImpl* impl = (BlkdevImpl*)dev;

    if (!_ValidLUKSBlkdev(dev) || !data || !impl->rawdev || !impl->masterkey)
        goto done;

    /* Read the given block */
    if (LUKSGetPayloadSector(
        impl->rawdev, 
        &impl->header, 
        impl->masterkey, 
        blkno, 
        data) != 0)
    {
        goto done;
    }

    rc = 0;

done:
    return rc;
}

static int _Put(
    Blkdev* dev,
    UINTN blkno,
    const void* data)
{
    int rc = -1;
    BlkdevImpl* impl = (BlkdevImpl*)dev;

    if (!_ValidLUKSBlkdev(dev) || !data || !impl->rawdev || !impl->masterkey)
    {
        goto done;
    }

    /* Write the given block */
    if (LUKSPutPayloadSector(
        impl->rawdev, 
        &impl->header, 
        impl->masterkey, 
        blkno, 
        data) != 0)
    {
        goto done;
    }

    rc = 0;

done:
    return rc;
}

static int _SetFlags(
    Blkdev* dev,
    UINT32 flags)
{
    /* No flags supported */
    return -1;
}

Blkdev* LUKSBlkdevFromMasterkey(
    Blkdev* rawdev,
    const UINT8* masterkey,
    UINT32 masterkeyBytes)
{
    BlkdevImpl* impl = NULL;
    UINT8* masterkeyClone = NULL;
    LUKSHeader header;

    /* Check parameters */
    if (!rawdev || !masterkey)
        goto done;

    /* Read the LUKS header */
    if (LUKSReadHeader(rawdev, &header) != 0)
        goto done;

    /* If masterkey size is wrong */
    if (masterkeyBytes != header.key_bytes)
        goto done;

    /* Allocate the master key */
    if (!(masterkeyClone = (UINT8*)Calloc(1, header.key_bytes)))
        goto done;

    /* Clone the master key */
    Memcpy(masterkeyClone, masterkey, header.key_bytes);

    /* Allocate the block device */
    if (!(impl = (BlkdevImpl*)Calloc(1, sizeof(BlkdevImpl))))
        goto done;

    /* Initialize the block device */
    impl->base.Close = _Close;
    impl->base.Put = _Put;
    impl->base.Get = _Get;
    impl->rawdev = rawdev;
    impl->magic = LUKSBLKDEV_MAGIC;
    impl->header = header;
    impl->masterkey = masterkeyClone;

done:

    if (!impl)
    {
        if (masterkeyClone)
            Free(masterkeyClone);
    }

    return &impl->base;
}

Blkdev* LUKSBlkdevFromRawBytes(
    Blkdev* rawdev,
    const UINT8* passphrase,
    UINTN passphraseSize)
{
    BlkdevImpl* impl = NULL;
    UINT8* masterkey = NULL;
    LUKSHeader header;

    /* Check parameters */
    if (!rawdev || !passphrase)
        goto done;

    /* Read the LUKS header */
    if (LUKSReadHeader(rawdev, &header) != 0)
        goto done;
    
    /* Allocate the master key */
    if (!(masterkey = (UINT8*)Calloc(1, header.key_bytes)))
        goto done;

    /* Use passphrase to unlock the master key */
    if (LUKSGetMasterKey(rawdev, &header, passphrase, passphraseSize, masterkey) != 0)
        goto done;

    /* Allocate the block device */
    if (!(impl = (BlkdevImpl*)Calloc(1, sizeof(BlkdevImpl))))
        goto done;

    /* Initialize the block device */
    impl->base.Close = _Close;
    impl->base.Get = _Get;
    impl->base.Put = _Put;
    impl->base.SetFlags = _SetFlags;
    impl->magic = LUKSBLKDEV_MAGIC;
    impl->header = header;
    impl->rawdev = rawdev;
    impl->masterkey = masterkey;

done:

    if (!impl)
    {
        if (masterkey)
            Free(masterkey);
    }

    return &impl->base;
}

BOOLEAN IsRawLUKSDevice(
    Blkdev* rawdev)
{
    BOOLEAN result = FALSE;
    union
    {
        LUKSHeader header;
        UINT8 sectors[LUKS_SECTOR_SIZE];
    }
    u;
    static UINT8 _magic[] = LUKS_MAGIC_INITIALIZER;

    if (!rawdev)
        goto done;

    /* Read the first two sectors of the raw device */
    if (BlkdevRead(rawdev, 0, &u, sizeof(u)) != 0)
        goto done;

    /* Check the LUKS magic bytes */
    if (Memcmp(&u.header, _magic, LUKS_MAGIC_SIZE) != 0)
        goto done;

    result = TRUE;

done:
    return result;
}

Blkdev* LUKSBlkdevFromPassphrase(
    Blkdev* rawdev,
    const char* passphrase)
{
    return LUKSBlkdevFromRawBytes(rawdev, (const UINT8*) passphrase, Strlen(passphrase));
}

int LUKSBlkdevGetMasterKey(
    Blkdev* dev,
    const UINT8** masterkeyData,
    UINTN* masterkeyBytes)
{
    int rc = -1;
    BlkdevImpl* impl = (BlkdevImpl*)dev;

    if (!_ValidLUKSBlkdev(dev))
        goto done;

    if (!masterkeyData || !masterkeyBytes || !impl->masterkey)
        goto done;

    *masterkeyData = impl->masterkey;
    *masterkeyBytes = impl->header.key_bytes;

    rc = 0;

done:
    return rc;
}

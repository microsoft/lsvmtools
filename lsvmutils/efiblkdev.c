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
#include "blkdev.h"
#include "eficommon.h"
#include "efibio.h"
#include "alloc.h"
#include "strings.h"

typedef struct _BlkdevImpl BlkdevImpl;

#define CACHE_SIZE 8

typedef struct _Block
{
    UINT8 data[BLKDEV_BLKSIZE];
}
Block;

struct _BlkdevImpl
{
    Blkdev base;
    EFI_BIO* bio;

    /* Cache */
    BOOLEAN cached;
    UINTN blkno;
    Block cache[CACHE_SIZE];
};

static int _Close(
    Blkdev* dev)
{
    int rc = -1;
    BlkdevImpl* impl = (BlkdevImpl*)dev;
    EFI_BIO* bio;

    if (!impl || !impl->bio)
        goto done;

    bio = impl->bio;
    Free(impl);

    if (CloseBIO(bio) != 0)
        return -1;

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

    if (!impl || !data || !impl->bio)
        goto done;

    /* Use the cache for single block reads */
    {
        if (impl->cached && 
            blkno >= impl->blkno && 
            blkno < impl->blkno + CACHE_SIZE)
        {
            UINTN index = blkno - impl->blkno;
            Memcpy(data, impl->cache[index].data, BLKDEV_BLKSIZE);
            rc = 0;
            goto done;
        }

        if (blkno + CACHE_SIZE <= impl->bio->blockIO->Media->LastBlock)
        {
            if (ReadBIO(
                impl->bio, 
                blkno, 
                &impl->cache, 
                CACHE_SIZE * BLKDEV_BLKSIZE) != EFI_SUCCESS)
            {
                goto done;
            }

            impl->cached = TRUE;
            impl->blkno = blkno;
            Memcpy(data, impl->cache[0].data, BLKDEV_BLKSIZE);
            rc = 0;
            goto done;
        }
    }

    if (ReadBIO(impl->bio, blkno, data, BLKDEV_BLKSIZE) != EFI_SUCCESS)
        goto done;

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

    if (!impl || !data || !impl->bio)
        goto done;

    if (WriteBIO(impl->bio, blkno, data, BLKDEV_BLKSIZE) != EFI_SUCCESS)
        goto done;

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

Blkdev* BlkdevFromBIO(
    EFI_BIO* bio)
{
    BlkdevImpl* impl = NULL;

    if (!bio)
        goto done;

    if (!(impl = Calloc(1, sizeof(BlkdevImpl))))
        goto done;

    impl->base.Close = _Close;
    impl->base.Get = _Get;
    impl->base.Put = _Put;
    impl->base.SetFlags = _SetFlags;
    impl->bio = bio;

done:
    return &impl->base;
}

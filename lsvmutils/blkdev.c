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
#include "strings.h"
#include "print.h"
#include "chksum.h"

int BlkdevRead(
    Blkdev* dev,
    UINTN blkno,
    void* data,
    UINTN size)
{
    int rc = -1;
    UINTN nblocks;
    UINT8* ptr;
    UINTN rem;
    UINT8 block[BLKDEV_BLKSIZE];

    if (!dev || !data)
        goto done;

    nblocks = (size + BLKDEV_BLKSIZE - 1) / BLKDEV_BLKSIZE;
    ptr = (UINT8*)data;
    rem = size;

    /* Nothing to do. */
    if (nblocks == 0)
    {
        rc = 0;
        goto done;
    }

    /* If size is aligned to block size, we just do a batch read. */
    if (size % BLKDEV_BLKSIZE == 0)
    {
        if (dev->GetN(dev, blkno, nblocks, ptr) != 0)
            goto done;

        rc = 0;
        goto done;
    }

    /* Otherwise, we can only read n-1 blocks to avoid overflow. */
    if (dev->GetN(dev, blkno, nblocks - 1, ptr) != 0)
        goto done;

    ptr += (nblocks - 1) * BLKDEV_BLKSIZE;
    rem -= (nblocks - 1) * BLKDEV_BLKSIZE;

    /* Read the last block and copy the remaining bytes to ptr. */
    if (dev->GetN(dev, blkno + nblocks - 1, 1, block) != 0)
        goto done;

    Memcpy(ptr, block, rem);

    rc = 0;

done:
    return rc;
}

int BlkdevWrite(
    Blkdev* dev,
    UINTN blkno,
    const void* data,
    UINTN size)
{
    int rc = -1;
    UINTN nblocks;
    const UINT8* ptr;
    UINTN rem;
    UINT8 block[BLKDEV_BLKSIZE];

    if (!dev || !data)
        goto done;

    nblocks = (size + BLKDEV_BLKSIZE - 1) / BLKDEV_BLKSIZE;
    ptr = (const UINT8*)data;
    rem = size;

    /* Nothing to do. */
    if (nblocks == 0)
    {
        rc = 0;
        goto done;
    }

    /* If size is aligned to block size, we just do a batch write. */
    if (size % BLKDEV_BLKSIZE == 0)
    {
        if (dev->PutN(dev, blkno, nblocks, ptr) != 0)
            goto done;

        rc = 0;
        goto done;
    }

    /* Otherwise, we can only write nblocks - 1 blocks without overflow. */
    if (dev->PutN(dev, blkno, nblocks - 1, ptr) != 0)
        goto done;

    ptr += (nblocks - 1) * BLKDEV_BLKSIZE;
    rem -= (nblocks - 1) * BLKDEV_BLKSIZE;

    /* Read the last block and rewrite part of it. */
    if (dev->GetN(dev, blkno + nblocks - 1, 1, block) != 0)
        goto done;

    Memcpy(block, ptr, rem);

    if (dev->PutN(dev, blkno + nblocks - 1, 1, block) != 0)
        goto done;

    rc = 0;

done:
    return rc;
}

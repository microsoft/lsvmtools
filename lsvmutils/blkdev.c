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
    UINTN i;
    UINT8* ptr;
    UINTN rem;

    if (!dev || !data)
        goto done;

    nblocks = (size + BLKDEV_BLKSIZE - 1) / BLKDEV_BLKSIZE;
    ptr = (UINT8*)data;
    rem = size;

    for (i = blkno; rem > 0 && i < blkno + nblocks; i++)
    {
        UINT8 block[BLKDEV_BLKSIZE];
        UINTN n;

        if (dev->Get(dev, i, block) != 0)
            goto done;

        n = rem < BLKDEV_BLKSIZE ? rem : BLKDEV_BLKSIZE;

        Memcpy(ptr, block, n);
        ptr += n;
        rem -= n;
    }

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
    UINTN i;
    const UINT8* ptr;
    UINTN rem;

    if (!dev || !data)
        goto done;

    nblocks = (size + BLKDEV_BLKSIZE - 1) / BLKDEV_BLKSIZE;
    ptr = (UINT8*)data;
    rem = size;

    for (i = blkno; rem > 0 && i < blkno + nblocks; i++)
    {
        UINT8 block[BLKDEV_BLKSIZE];
        UINTN n;

        /* Read the existing block */
        if (dev->Get(dev, i, block) != 0)
            goto done;

        /* Update this block */
        n = rem < BLKDEV_BLKSIZE ? rem : BLKDEV_BLKSIZE;
        Memcpy(block, ptr, n);

        /* Rewrite the block */
        if (dev->Put(dev, i, block) != 0)
            goto done;

        ptr += n;
        rem -= n;
    }

    rc = 0;

done:
    return rc;
}

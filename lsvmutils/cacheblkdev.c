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
#include "cacheblkdev.h"
#include "alloc.h"
#include "strings.h"
#include "print.h"

#define MAX_CHAINS (64*1024)

typedef struct _BlkdevImpl BlkdevImpl;
typedef struct _Block Block;

struct _Block
{
    Block* next;
    UINTN blkno;
    UINT8 data[BLKDEV_BLKSIZE];
};

struct _BlkdevImpl
{
    Blkdev base;
    Blkdev* child;
    Block* chains[MAX_CHAINS];
    UINT32 flags; /* BLKDEV_ENABLE_CACHING supported */
};

static void _ReleaseCache(
    BlkdevImpl* impl)
{
    UINTN i;

    for (i = 0; i < MAX_CHAINS; i++)
    {
        Block* p;
        Block* next;

        for (p = impl->chains[i]; p; p = next)
        {
            next = p->next;
            Free(p);
        }
    }
}

static Block* _GetCache(
    BlkdevImpl* impl,
    UINTN blkno)
{
    Block* p;
    UINTN slot = blkno % MAX_CHAINS;

    for (p = impl->chains[slot]; p; p = p->next)
    {
        if (p->blkno == blkno)
            return p;
    }

    return NULL;
}

static int _PutCache(
    BlkdevImpl* impl,
    UINTN blkno,
    const void* data)
{
    int rc = -1;
    UINTN slot = blkno % MAX_CHAINS;
    Block* block;

    /* Allocate new block */
    if (!(block = Calloc(1, sizeof(Block))))
        goto done;

    /* Initialize the block */
    Memcpy(block->data, data, sizeof(block->data));
    block->blkno = blkno;

    /* Add to cache */
    block->next = impl->chains[slot];
    impl->chains[slot] = block;

    rc = 0;

done:
    return rc;
}

static int _Close(
    Blkdev* dev)
{
    int rc = -1;
    BlkdevImpl* impl = (BlkdevImpl*)dev;
    Blkdev* child;

    /* Check for null parameters */
    if (!impl || !impl->child)
        goto done;

    /* Free all the cached blocks */
    _ReleaseCache(impl);

    /* Free this block device */
    child = impl->child;
    Free(impl);

    /* Free the child block device */
    if (child->Close(child) != 0)
        goto done;

    rc = 0;

done:

    return rc;
}

static int _GetN(
    Blkdev* dev,
    UINTN blkno,
    UINTN nblocks,
    void* data)
{
    int rc = -1;
    BlkdevImpl* impl = (BlkdevImpl*)dev;
    UINTN i;
    UINT8* ptr = (UINT8*) data;

    /* Check for null parameters */
    if (!impl || !data || !impl->child)
        goto done;

    /* Loop through and check if in cache. Otherwise, read from disk. */
    for (i = 0; i < nblocks;)
    {
        UINTN j;
        const Block* block;

        /* Loop through blocks and retrieve from cache. */
        while (i < nblocks && (block = _GetCache(impl, blkno + i)))
        {
            Memcpy(
                ptr + i*sizeof(block->data), block->data, sizeof(block->data));
            i++;
        }

        /* Find the next cached block to see how much we need to read. */
        j = i;
        while (j < nblocks && (!_GetCache(impl, blkno + j)))
            j++;

        /* Skip if we don't need to read anything. */
        if (j == i)
            continue;

        /* Read as much as we can from the disk. */
        if (impl->child->GetN(
                impl->child,
                blkno + i,
                j - i,
                ptr + i*sizeof(block->data)) != 0)
        {
            goto done;
        }

        /* If caching is enabled, then put into cache. */
        if (impl->flags & BLKDEV_ENABLE_CACHING)
        {
            UINTN k;
            for (k = i; k < j; k++)
            {
                if (_PutCache(impl, blkno + k, ptr + k*sizeof(block->data)) != 0)
                    goto done;
            }
        }

        /* Advance to next sequence. */
        i = j;
    }

    rc = 0;

done:
    return rc;
}

static int _PutN(
    Blkdev* dev,
    UINTN blkno,
    UINTN nblocks,
    const void* data)
{
    int rc = -1;
    BlkdevImpl* impl = (BlkdevImpl*)dev;

    /* Check for null parameters */
    if (!impl || !data || !impl->child)
        goto done;

    /* If caching is enabled, change cache, but not disk */
    if (impl->flags & BLKDEV_ENABLE_CACHING)
    {
        UINTN i;
        Block* block;
        const UINT8* ptr = (const UINT8*) data;

        for (i = 0; i < nblocks; i++)
        {
            if ((block = _GetCache(impl, blkno + i)))
            {
                Memcpy(block->data, ptr, sizeof(block->data));
            }
            else if (_PutCache(impl, blkno + i, ptr) != 0)
            {
                goto done;
            }
            ptr += sizeof(block->data);
        }
    }
    else
    {
        /* If control reaches here, then disk will be modified */
        if (impl->child->PutN(impl->child, blkno, nblocks, data) != 0)
        {
            goto done;
        }
    }

    rc = 0;

done:
    return rc;
}

static int _SetFlags(
    Blkdev* dev,
    UINT32 flags)
{
    int rc = -1;
    BlkdevImpl* impl = (BlkdevImpl*)dev;

    /* Check for null parameters */
    if (!impl)
        goto done;

    /* If unrecognized flag */
    if (flags & ~BLKDEV_ENABLE_CACHING)
        goto done;

    impl->flags = flags;

    rc = 0;

done:
    return rc;
}

Blkdev* NewCacheBlkdev(
    Blkdev* dev)
{
    BlkdevImpl* impl = NULL;

    if (!dev)
        goto done;

    if (!(impl = Calloc(1, sizeof(BlkdevImpl))))
        goto done;

    impl->base.Close = _Close;
    impl->base.GetN = _GetN;
    impl->base.PutN = _PutN;
    impl->base.SetFlags = _SetFlags;
    impl->child = dev;

done:
    return &impl->base;
}

void CacheBlkdevStats(
    Blkdev* dev,
    UINTN* numBlocks,
    UINTN* numChains,
    UINTN* maxChains,
    UINTN* longestChain)
{
    BlkdevImpl* impl = (BlkdevImpl*)dev;
    UINTN i;

    *numBlocks = 0;
    *numChains = 0;
    *maxChains = MAX_CHAINS;
    *longestChain = 0;

    for (i = 0; i < MAX_CHAINS; i++)
    {
        Block* p;
        UINTN n = 0;

        if (impl->chains[i])
            (*numChains)++;

        for (p = impl->chains[i]; p; p = p->next)
        {
            (*numBlocks)++;
            n++;
        }

        if (n > *longestChain)
            (*longestChain)++;
    }
}

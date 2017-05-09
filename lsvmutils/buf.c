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
#include "buf.h"
#include "strings.h"
#include "alloc.h"

#if !defined(BUILD_EFI)
# include <string.h>
# include <stdlib.h>
# include <stdio.h>
#endif /* !defined(BUILD_EFI) */

/*
**==============================================================================
**
** Buf:
**
**==============================================================================
*/

#define EXT2_BUF_CHUNK_SIZE 1024

void BufInit(
    Buf* buf)
{
    if (buf)
        Memset(buf, 0, sizeof(Buf));
}

void BufRelease(
    Buf* buf)
{
    if (buf && buf->data)
    {
        Memset(buf->data, 0xDD, buf->size);
        Free(buf->data);
    }

    Memset(buf, 0x00, sizeof(Buf));
}

int BufClear(
    Buf* buf)
{
    if (!buf)
        return -1;

    buf->size = 0;

    return 0;
}

int BufReserve(
    Buf* buf,
    UINTN cap)
{
    if (!buf)
        return -1;

    /* If capacity is bigger than current capacity */
    if (cap > buf->cap)
    {
        void* new_data;
        UINTN new_cap;

        /* Double current capacity (will be zero the first time) */
        new_cap = buf->cap * 2;

        /* If capacity still insufficent, round to multiple of chunk size */
        if (cap > new_cap)
        {
            const UINTN N = EXT2_BUF_CHUNK_SIZE;
            new_cap = (cap + N - 1) / N * N;
        }

        /* Expand allocation */
        if (!(new_data = Realloc(buf->data, buf->cap, new_cap)))
            return -1;

        buf->data = new_data;
        buf->cap = new_cap;
    }

    return 0;
}

int BufAppend(
    Buf* buf,
    const void* data,
    UINTN size)
{
    UINTN new_size;

    /* Check arguments */
    if (!buf || !data)
        return -1;

    /* If zero-sized, then success */
    if (size == 0)
        return 0;

    /* Compute the new size */
    new_size = buf->size + size;

    /* If insufficient capacity to hold new data */
    if (new_size > buf->cap)
    {
        int err;

        if ((err = BufReserve(buf, new_size)) != 0)
            return err;
    }

    /* Copy the data */
    Memcpy((unsigned char*)buf->data + buf->size, data, size);
    buf->size = new_size;

    return 0;
}

/*
**==============================================================================
**
** BufU32:
**
**==============================================================================
*/

void BufU32Release(
    BufU32* buf)
{
    Buf tmp;

    tmp.data = buf->data;
    tmp.size = buf->size * sizeof(UINT32);
    tmp.cap = buf->cap * sizeof(UINT32);
    BufRelease(&tmp);
}

int BufU32Append(
    BufU32* buf,
    const UINT32* data,
    UINTN size)
{
    Buf tmp;

    tmp.data = buf->data;
    tmp.size = buf->size * sizeof(UINT32);
    tmp.cap = buf->cap * sizeof(UINT32);

    if (BufAppend(
        &tmp, 
        data, 
        size * sizeof(UINT32)) != 0)
    {
        return -1;
    }

    buf->data = tmp.data;
    buf->size = tmp.size / sizeof(UINT32);
    buf->cap = tmp.cap / sizeof(UINT32);

    return 0;
}

/*
**==============================================================================
**
** BufPtr:
**
**==============================================================================
*/

void BufPtrRelease(
    BufPtr* buf)
{
    Buf tmp;

    tmp.data = buf->data;
    tmp.size = buf->size * sizeof(void*);
    tmp.cap = buf->cap * sizeof(void*);
    BufRelease(&tmp);
}

int BufPtrAppend(
    BufPtr* buf,
    const void** data,
    UINTN size)
{
    Buf tmp;

    tmp.data = buf->data;
    tmp.size = buf->size * sizeof(void*);
    tmp.cap = buf->cap * sizeof(void*);

    if (BufAppend(
        &tmp, 
        data, 
        size * sizeof(void*)) != 0)
    {
        return -1;
    }

    buf->data = tmp.data;
    buf->size = tmp.size / sizeof(void*);
    buf->cap = tmp.cap / sizeof(void*);

    return 0;
}

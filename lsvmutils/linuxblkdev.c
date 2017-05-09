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
#include "stdlib.h"
#include "alloc.h"
#include "strings.h"
#include "print.h"

#if defined(__linux__) && !defined(BUILD_EFI)
# include <sys/types.h>
# include <sys/fcntl.h>
# include <unistd.h>
#endif

typedef struct _BlkdevImpl BlkdevImpl;

struct _BlkdevImpl
{
    Blkdev base;
    UINTN offset;
    int fd;
};

static int _Close(
    Blkdev* dev)
{
    int rc = -1;
    BlkdevImpl* impl = (BlkdevImpl*)dev;

    if (!dev)
        goto done;

    close(impl->fd);
    Free(impl);

    rc = 0;

done:
    return rc;
}

static ssize_t _Readn(
    int fd,
    void* data,
    size_t size)
{
    unsigned char* p = (unsigned char*)data;
    size_t r = size;

    if (!data)
        return -1;

    while (r)
    {
        ssize_t n = read(fd, p, r);

        if (n > 0)
        {
            p += n;
            r -= n;
        }
        else
            return -1;
    }

    return size;
}

static ssize_t _Writen(
    int fd,
    const void* data,
    size_t size)
{
    const unsigned char* p = (const unsigned char*)data;
    size_t r = size;

    if (!data)
        return -1;

    while (r)
    {
        ssize_t n = write(fd, p, r);

        if (n > 0)
        {
            p += n;
            r -= n;
        }
        else
            return -1;
    }

    return size;
}

static int _Get(
    Blkdev* dev,
    UINTN blkno,
    void* data)
{
    int rc = -1;
    BlkdevImpl* impl = (BlkdevImpl*)dev;
    UINTN offset;

    if (!dev || !data)
        goto done;

    offset = impl->offset + blkno * BLKDEV_BLKSIZE;

    if (lseek(impl->fd, offset, SEEK_SET) != offset)
        goto done;

    if (_Readn(impl->fd, data, BLKDEV_BLKSIZE) != BLKDEV_BLKSIZE)
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
    UINTN offset;

    if (!dev || !data)
        goto done;

    offset = impl->offset + blkno * BLKDEV_BLKSIZE;

    if (lseek(impl->fd, offset, SEEK_SET) != offset)
        goto done;

    if (_Writen(impl->fd, data, BLKDEV_BLKSIZE) != BLKDEV_BLKSIZE)
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

Blkdev* BlkdevOpen(
    const char* path,
    BlkdevAccess access,
    UINTN offset)
{
    BlkdevImpl* impl = NULL;
    int flags = 0;
    int fd;

    if (!path)
        goto done;

    if (access == BLKDEV_ACCESS_RDWR)
        flags = O_RDWR;
    else if (access == BLKDEV_ACCESS_RDONLY)
        flags = O_RDONLY;
    else if (access == BLKDEV_ACCESS_WRONLY)
        flags = O_WRONLY;

    if ((fd = open(path, flags)) < 0)
        goto done;

    if (!(impl = Calloc(1, sizeof(BlkdevImpl))))
        goto done;

    impl->base.Close = _Close;
    impl->base.Get = _Get;
    impl->base.Put = _Put;
    impl->base.SetFlags = _SetFlags;
    impl->offset = offset;
    impl->fd = fd;

done:
    return &impl->base;
}

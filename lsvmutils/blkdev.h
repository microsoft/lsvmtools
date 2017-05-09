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
#ifndef _blkdev_h
#define _blkdev_h

#include "config.h"
#include <lsvmutils/eficommon.h>

#define BLKDEV_BLKSIZE 512

#define BLKDEV_ENABLE_CACHING 1

typedef struct _Blkdev Blkdev;

struct _Blkdev
{
    int (*Close)(
        Blkdev* dev);

    int (*Get)(
        Blkdev* dev,
        UINTN blkno,
        void* data);

    int (*Put)(
        Blkdev* dev,
        UINTN blkno,
        const void* data);

    int (*SetFlags)(
        Blkdev* dev,
        UINT32 flags);
};

typedef enum _BlkdevAccess
{
    BLKDEV_ACCESS_RDWR,
    BLKDEV_ACCESS_RDONLY,
    BLKDEV_ACCESS_WRONLY
}
BlkdevAccess;

Blkdev* BlkdevOpen(
    const char* path,
    BlkdevAccess access,
    UINTN offset); /* add this offset to all seeks */

int BlkdevRead(
    Blkdev* dev,
    UINTN blkno,
    void* data,
    UINTN size);

int BlkdevWrite(
    Blkdev* dev,
    UINTN blkno,
    const void* data,
    UINTN size);

#endif /* _blkdev_h */

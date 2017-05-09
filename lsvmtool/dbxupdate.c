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
#include "dbxupdate.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/fs.h>
#include <unistd.h>
#include <fcntl.h>

#define EFIVARDIR "/sys/firmware/efi/efivars/"
#define EFIVARDBX EFIVARDIR "dbx-d719b2cb-3d3a-4596-a3bc-dad00e67656f"

static int _SetImmutableByPath(const char* path, int immutable)
{
    int rc = -1;
    int fd = -1;
    unsigned int flags; 

    /* Open the file */
    if ((fd = open(path, O_RDONLY)) < 0)
        goto done;
 
    if (ioctl(fd, FS_IOC_GETFLAGS, &flags) < 0)
        goto done;

    if (immutable)
        flags |= FS_IMMUTABLE_FL;
    else
        flags &= ~FS_IMMUTABLE_FL;

    if (ioctl(fd, FS_IOC_SETFLAGS, &flags) < 0)
        goto done;

    rc = 0;

done:
    if (fd != -1)
        close(fd);

    return rc;
}

static int _SetImmutable(int fd, int immutable)
{
    int rc = -1;
    unsigned int flags; 

    if (ioctl(fd, FS_IOC_GETFLAGS, &flags) < 0)
        goto done;

    if (immutable)
        flags |= FS_IMMUTABLE_FL;
    else
        flags &= ~FS_IMMUTABLE_FL;

    if (ioctl(fd, FS_IOC_SETFLAGS, &flags) < 0)
        goto done;

    rc = 0;

done:

    return rc;
}

static int _SetVariable(
    const char* path,
    const void* data,
    UINTN size,
    UINT32 attrs,
    int mode)
{
    int rc = -1;
    int fd = -1;
    unsigned char* buf = NULL;
    UINTN bufSize;
    BOOLEAN setImmutableOk = TRUE;

    /* Reject null parameters */
    if (!path || !data)
        goto done;

    /* If parent directory does not exist, then fail */
    {
        struct stat st;

        if (stat(EFIVARDIR, &st) != 0 || !(st.st_mode & S_IFDIR))
            goto done;
    }

    /* Clear the immutable flag */
    if (access(path, R_OK) == 0)
    {
        /* Ignore error since not all platforms support efivarfs ioctl's */
        if (_SetImmutableByPath(path, 0) < 0)
            setImmutableOk = FALSE;
    }

    /* Delete the file if APPEND_WRITE */
    if (attrs & EFI_VARIABLE_APPEND_WRITE)
        unlink(path);

    /* Open the file */
    if ((fd = open(path, O_WRONLY | O_CREAT, mode)) < 0)
        goto done;

    /* Clear the immutable flag */
    if (setImmutableOk && _SetImmutable(fd, 0) != 0)
        goto done;

    /* Write the data to the variable file */
    {
        bufSize = sizeof(attrs) + size;

        if (!(buf = (unsigned char*)malloc(bufSize)))
            goto done;

        memcpy(buf, &attrs, sizeof(attrs));
        memcpy(&buf[sizeof(attrs)], data, size);

        if (write(fd, buf, bufSize) != bufSize)
            goto done;
    }

    /* Set the immutable flag */
    if (setImmutableOk && _SetImmutable(fd, 1) != 0)
        goto done;

    rc = 0;

done:

    if (fd < 0)
        close(fd);

    if (buf)
        free(buf);

    return rc;
}

int ApplyDBXUpdate(
    const void* data,
    UINTN size)
{
    int rc = -1;

    UINT32 attrs = 0;
    attrs |= EFI_VARIABLE_NON_VOLATILE;
    attrs |= EFI_VARIABLE_BOOTSERVICE_ACCESS;
    attrs |= EFI_VARIABLE_RUNTIME_ACCESS;
    attrs |= EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS;
    attrs |= EFI_VARIABLE_APPEND_WRITE;

    if (_SetVariable(
        EFIVARDBX,
        data,
        size,
        attrs,
        0644) != 0)
    {
        goto done;
    }

    rc = 0;

done:
    return rc;
}

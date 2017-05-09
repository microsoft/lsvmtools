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
#define POSIXEFI_SUPPRESS_DEFINITIONS
#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#if defined(BUILD_EFI)
# include <efi.h>
# include <efilib.h>
#endif

void __posix_panic(const char* func);

posix_pid_t posix_getpid()
{
    __posix_panic("getpid");
    return 0;
}

posix_uid_t posix_getuid()
{
    __posix_panic("getuid");
    return 0;
}

posix_uid_t posix_geteuid()
{
    __posix_panic("geteuid");
    return 0;
}

posix_gid_t posix_getgid()
{
    __posix_panic("getgid");
    return 0;
}

posix_gid_t posix_getegid()
{
    __posix_panic("getegid");
    return 0;
}

/*
**==============================================================================
** 
** File I/O functions:
**
**==============================================================================
*/

typedef struct _File
{
    char* path;
    void* context;
    posix_lseek_callback lseek;
    posix_read_callback read;
    posix_write_callback write;
    posix_close_callback close;
}
File;

#define MAX_FILES 64
static File _files[MAX_FILES];
static posix_size_t _nfiles;

static int _path_to_fd(const char* path)
{
    int fd;

    for (fd = 0; fd < MAX_FILES; fd++)
    {
        if (_files[fd].path && posix_strcmp(_files[fd].path, path) == 0)
            return fd;
    }

    /* Not found */
    return -1;
}

int posix_register_file(
    const char* path,
    void* context,
    posix_lseek_callback lseek,
    posix_read_callback read,
    posix_write_callback write,
    posix_close_callback close)
{
    int rc = -1;
    File file;

    /* Check for null parameters */
    if (!path)
        goto done;

    /* Clear file object */
    posix_memset(&file, 0, sizeof(file));

    /* Fail if this file already exists */
    if (_path_to_fd(path) >= 0)
        goto done;

    /* Check for array overflow */
    if (_nfiles == MAX_FILES)
        goto done;

    /* Initialize file.path */
    if (!(file.path = posix_strdup(path)))
        goto done;

    /* Initialize file.data and file.size */
    file.lseek = lseek;
    file.read = read;
    file.write = write;
    file.close = close;
    file.context = context;

    /* Append to files array */
    _files[_nfiles++] = file;

    rc = 0;

done:

    if (rc != 0)
    {
        if (file.path)
            posix_free(file.path);
    }

    return rc;
}

int posix_open(const char *path, int flags, int mode)
{
    int fd = -1;

    if (!path)
        goto done;

    /* If file is not regitered, then fail */
    if ((fd = _path_to_fd(path)) < 0)
        goto done;

done:
    return fd;
}

posix_off_t posix_lseek(int fd, posix_off_t offset, int whence)
{
    posix_off_t n = -1;

    if (fd < 0 || fd >= MAX_FILES || !_files[fd].path)
        goto done;

    if (_files[fd].lseek)
        n = _files[fd].lseek(_files[fd].context, offset, whence);

done:
    return n;
}

posix_ssize_t posix_read(int fd, void *buf, posix_size_t count)
{
    posix_ssize_t n = -1;

    if (!buf)
        goto done;

    if (fd < 0 || fd >= MAX_FILES || !_files[fd].path)
        goto done;

    if (_files[fd].read)
        n = (*_files[fd].read)(_files[fd].context, buf, count);

done:
    return n;
}

posix_ssize_t posix_write(int fd, const void *buf, posix_size_t count)
{
    posix_size_t n = -1;

    if (fd < 0 || fd >= MAX_FILES || !_files[fd].path)
        goto done;

    if (_files[fd].write)
        n = (*_files[fd].write)(_files[fd].context, buf, count);

done:
    return n;
}

int posix_close(int fd)
{
    int rc = -1;

    if (fd < 0 || fd >= MAX_FILES || !_files[fd].path)
        goto done;

    if (_files[fd].close)
        rc = (*_files[fd].close)(_files[fd].context);

    posix_free(_files[fd].path);
    posix_memset(&_files[fd], 0, sizeof(File));

    rc = 0;

done:
    return rc;
}

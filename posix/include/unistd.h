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
#ifndef _posix_unistd_h
#define _posix_unistd_h

#include <posix.h>
#include <stddef.h>

typedef int posix_pid_t;
typedef int posix_uid_t;
typedef int posix_gid_t;
typedef unsigned long posix_off_t;

posix_pid_t posix_getpid();

posix_uid_t posix_getuid();

posix_uid_t posix_geteuid();

posix_gid_t posix_getgid();

posix_gid_t posix_getegid();

#define O_RDONLY 00000000
#define O_WRONLY 00000001
#define O_CREAT  00000100
#define O_TRUNC  00001000
#define O_APPEND 00002000

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

int posix_open(const char *pathname, int flags, int mode);

posix_off_t posix_lseek(int fd, posix_off_t offset, int whence);

posix_ssize_t posix_read(int fd, void *buf, posix_size_t count);

posix_ssize_t posix_write(int fd, const void *buf, posix_size_t count);

int posix_close(int fd);

typedef posix_off_t (*posix_lseek_callback)(
    void* context,
    posix_off_t offset,
    int whence);

typedef posix_ssize_t (*posix_read_callback)(
    void* context,
    void* buf,
    posix_ssize_t count);

typedef posix_ssize_t (*posix_write_callback)(
    void* context,
    const void* buf,
    posix_ssize_t size);

typedef int (*posix_close_callback)(
    void* context);

int posix_register_file(
    const char* path,
    void* context,
    posix_lseek_callback lseek,
    posix_read_callback read,
    posix_write_callback write,
    posix_close_callback close);

#endif /* _posix_unistd_h */

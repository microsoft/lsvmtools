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
#ifndef _efibuild_unistd_h
#define _efibuild_unistd_h

#include <stddef.h>
#include <efi.h>
#include <efilib.h>

typedef int pid_t;
typedef int uid_t;
typedef int gid_t;
typedef unsigned long off_t;

void __panic(const char* func);

static __inline__ pid_t getpid()
{
    __panic("getpid");
    return 0;
}

static __inline__ uid_t getuid()
{
    __panic("getuid");
    return 0;
}

static __inline__ uid_t geteuid()
{
    __panic("geteuid");
    return 0;
}

static __inline__ gid_t getgid()
{
    __panic("getgid");
    return 0;
}

static __inline__ gid_t getegid()
{
    __panic("getegid");
    return 0;
}

#endif /* _efibuild_unistd_h */

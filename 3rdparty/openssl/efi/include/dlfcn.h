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
#ifndef _efibuild_dlsym_h
#define _efibuild_dlsym_h

#include <string.h>

#define RTLD_NOW 0
#define RTLD_LAZY 1

typedef struct
{
    const char *dli_fname;
    void *dli_fbase;
    const char *dli_sname;
    void *dli_saddr;
} 
Dl_info;

void __panic(const char* func);

static __inline__ void *dlopen(const char *filename, int flags)
{
    __panic("dlopen");
    return NULL;
}

static __inline__ void *dlsym(void *handle, const char *symbol)
{
    __panic("dlsym");
    return NULL;
}

static __inline__ char *dlerror()
{
    __panic("dlerror");
    return "unknown";
}

static __inline__ int dlclose(void *handle)
{
    __panic("dlclose");
    return -1;
}

static __inline__ int dladdr(void *addr, Dl_info *info)
{
    if (info)
        memset(info, 0, sizeof(Dl_info));
    __panic("dladdr");
    return -1;
}

#endif /* _efibuild_dlsym_h */

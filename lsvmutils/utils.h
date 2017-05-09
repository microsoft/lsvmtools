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
#ifndef _utils_h
#define _utils_h

#include "config.h"
#include "eficommon.h"

#if !defined(BUILD_EFI)
# include <limits.h>
# include <stddef.h>
#endif /* defined(BUILD_EFI) */

char *StrcpySafe(
    char *dest, 
    size_t size, 
    const char *src);

char *StrncatSafe(
    char *dest, 
    size_t size, 
    const char *src, 
    size_t count);

#if !defined(BUILD_EFI)
FILE* Fopen(
    const char* path,
    const char* mode);
#endif /* defined(BUILD_EFI) */

char* Dupenv(
    const char* name);

const char* Basename(
    const char* path);

#if !defined(BUILD_EFI)
void GetProgramName(
    const char* path,
    char programName[PATH_MAX]);
#endif /* defined(BUILD_EFI) */

int ExpandEnvVars(
    char* buf,
    UINTN bufSize,
    const char* str);

#endif /* _utils_h */

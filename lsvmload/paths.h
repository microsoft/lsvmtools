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
#ifndef _paths_h
#define _paths_h

#include "config.h"
#include <lsvmutils/eficommon.h>
#include <lsvmutils/error.h>
#include "globals.h"

#if !defined(PATH_SIZE)
# define PATH_SIZE 128
#endif

#define GRUB_PATH L"/lsvmload/grubx64.efi"

#define SHIM_PATH L"/lsvmload/shimx64.efi"

EFI_STATUS ResolvePaths(
    IN EFI_HANDLE imageHandle,
    Error* err);

const CHAR16* UnixToDosPath(
    CHAR16 dosPath[PATH_SIZE], 
    const CHAR16* unixPath);

const CHAR16* DosToUnixPath(
    CHAR16 unixPath[PATH_SIZE], 
    const CHAR16* dosPath);

const CHAR16* Basename(
    const CHAR16* path);

int Dirname(
    CHAR16* path);

#endif /* _paths_h */

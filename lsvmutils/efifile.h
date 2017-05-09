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
#ifndef _efifile_h
#define _efifile_h

#include "config.h"
#include <lsvmutils/eficommon.h>
#include <lsvmutils/error.h>
#include <lsvmutils/error.h>

typedef struct _EFIFile EFIFile;

EFI_STATUS FileExists(
    IN EFI_HANDLE imageHandle,
    IN const CHAR16* path);

EFI_STATUS GetFileSize(
    IN EFI_HANDLE imageHandle,
    IN const CHAR16* path,
    OUT UINTN* size);

EFIFile* OpenFile(
    IN EFI_HANDLE imageHandle,
    IN const CHAR16* path,
    IN UINT64 fileMode, /* EFI_FILE_MODE_[READ|CREATE|WRITE] */
    IN BOOLEAN append); /* If TRUE open for append */

EFI_STATUS ReadFile(
    IN EFIFile* efiFile,
    IN void* data,
    IN UINTN size,
    IN UINTN* sizeRead);

EFI_STATUS ReadFileN(
    EFIFile* efiFile,
    void* data,
    UINTN size);

EFI_STATUS WriteFile(
    IN EFIFile* efiFile,
    IN const void* data,
    IN UINTN size,
    IN UINTN* sizeWritten);

EFI_STATUS WriteFileN(
    IN EFIFile* efiFile,
    IN const void* data,
    IN UINTN size);

EFI_STATUS FlushFile(
    IN EFIFile* efiFile);

EFI_STATUS DeleteFile(
    IN EFI_HANDLE imageHandle,
    IN const CHAR16* path);

EFI_STATUS CloseFile(
    IN EFIFile* efiFile);

EFI_STATUS SeekFile(
    IN EFIFile* efiFile,
    IN UINT64 position);

EFI_STATUS EFILoadFile(
    IN EFI_HANDLE imageHandle,
    IN const CHAR16* path,
    OUT void** data,
    OUT UINTN* size);

EFI_STATUS EFIPutFile(
    IN EFI_HANDLE imageHandle,
    IN const CHAR16* path,
    IN const void* data,
    IN UINTN size,
    Error* err);

#endif /* _efifile_h */

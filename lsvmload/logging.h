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
#ifndef _logging_h
#define _logging_h

#include "config.h"
#include <lsvmutils/eficommon.h>
#include <lsvmutils/tcg2.h>
#include <lsvmutils/sha.h>

void LogPCRs(
    EFI_TCG2_PROTOCOL* tcg2Protocol,
    EFI_HANDLE imageHandle,
    const CHAR16* message);

void LogEventLog(
    EFI_TCG2_PROTOCOL* tcg2Protocol,
    EFI_HANDLE imageHandle);

void LogPaths();

void LogSHA1(
    const CHAR16* name,
    const SHA1Hash* sha1);

void LogSHA256(
    const CHAR16* name,
    const SHA256Hash* sha256);

int LogHexStr(
    const CHAR16* name,
    const void* data, 
    UINTN size);

int LogASCIIStr(
    const CHAR16* name,
    const void* data, 
    UINTN size);

#endif /* _logging_h */

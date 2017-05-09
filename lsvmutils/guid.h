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
#ifndef _guid_h
#define _guid_h

#include <lsvmutils/eficommon.h>
#include <lsvmutils/strings.h>
#include <lsvmutils/print.h>

#if !defined(BUILD_EFI)
# include <stdio.h>
#endif

#define GUID_STRING_LENGTH 36
#define GUID_STRING_SIZE (GUID_STRING_LENGTH + 1)
#define GUID_FORMAT "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x"

void MakeGUID(EFI_GUID* guid, UINT64 x, UINT64 y);

void SplitGUID(const EFI_GUID* guid, UINT64* x, UINT64* y);

void MakeGUIDFromBytes(EFI_GUID* guid, const UINT8 x[16]);

void FormatGUID(
    TCHAR buf[GUID_STRING_SIZE],
    const EFI_GUID* guid);

static __inline void DumpGUID(const EFI_GUID* guid)
{
    TCHAR buf[GUID_STRING_SIZE];
    FormatGUID(buf, guid);
    Print(TCS("%s"), buf);
}

static __inline void DumpGUID2(UINT64 x, UINT64 y)
{
    EFI_GUID guid;
    MakeGUID(&guid, x, y);
    DumpGUID(&guid);
}

static __inline void DumpGUID3(const UINT8 bytes[16])
{
    EFI_GUID guid;
    MakeGUIDFromBytes(&guid, bytes);
    DumpGUID(&guid);
}

BOOLEAN ValidGUIDStr(
    const char* str);

#endif /* _guid_h */

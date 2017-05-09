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
#ifndef _tpmbuf_h
#define _tpmbuf_h

#include "config.h"
#include "eficommon.h"

#define __BUF_CAPACITY 4096

typedef struct _TPMBuf
{
    unsigned char data[__BUF_CAPACITY];
    UINT32 size;
    UINT32 offset;
    UINT32 cap;
    int error; /* non-zero if buffer is in an error state */
}
TPMBuf;

void TPMBufInit(
    TPMBuf* self);

void TPMBufPack(
    TPMBuf* self,
    const void* data,
    UINT32 size);

void TPMBufPackU8(
    TPMBuf* self,
    UINT8 x);

void TPMBufPackU16(
    TPMBuf* self,
    UINT16 x);

void TPMBufPackU32(
    TPMBuf* self,
    UINT32 x);

#if 0
void TPMBufPackU64(
    TPMBuf* self,
    UINT64 x);
#endif

void TPMBufUnpack(
    TPMBuf* self,
    void* data,
    UINT32 size);

void TPMBufUnpackU8(
    TPMBuf* self,
    UINT8* x);

void TPMBufUnpackU16(
    TPMBuf* self,
    UINT16* x);

void TPMBufUnpackU32(
    TPMBuf* self,
    UINT32* x);

#if 0
void TPMBufUnpackU64(
    TPMBuf* self,
    UINT64* x);
#endif

#endif /* _tpmbuf_h */

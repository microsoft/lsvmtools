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
#include "config.h"
#include "byteorder.h"
#include "tpmbuf.h"
#include "strings.h"

void TPMBufInit(
    TPMBuf* self)
{
    Memset(self->data, 0xDD, sizeof(self->data));
    self->size = 0;
    self->offset = 0;
    self->cap = sizeof(self->data);
    self->error = 0;
}

void TPMBufPack(
    TPMBuf* self,
    const void* data,
    UINT32 size)
{
    UINT32 r = self->cap - self->size;

    if (self->error)
        return;

    if (size > r)
    {
        self->error = 1;
        return;
    }

    Memcpy(&self->data[self->size], (void*)data, size);
    self->size += size;
}

void TPMBufPackU8(
    TPMBuf* self,
    UINT8 x)
{
    TPMBufPack(self, (const char*)&x, sizeof(x));
}

void TPMBufPackU16(
    TPMBuf* self,
    UINT16 x)
{
    UINT16 tmp = ByteSwapU16(x);
    TPMBufPack(self, (const char*)&tmp, sizeof(tmp));
}

void TPMBufPackU32(
    TPMBuf* self,
    UINT32 x)
{
    UINT32 tmp = ByteSwapU32(x);
    TPMBufPack(self, (const char*)&tmp, sizeof(tmp));
}

#if 0
void TPMBufPackU64(
    TPMBuf* self,
    UINT64 x)
{
    UINT64 tmp = ByteSwapU64(x);
    TPMBufPack(self, (const char*)&tmp, sizeof(tmp));
}
#endif

void TPMBufUnpack(
    TPMBuf* self,
    void* data,
    UINT32 size)
{
    UINT32 r = self->size - self->offset;

    if (self->error)
        return;

    if (size > r)
    {
        self->error = 1;
        return;
    }

    Memcpy(data, &self->data[self->offset], size);
    self->offset += size;
}

void TPMBufUnpackU8(
    TPMBuf* self,
    UINT8* x)
{
    UINT32 r = self->size - self->offset;

    if (self->error)
        return;

    if (sizeof(*x) > r)
    {
        self->error = 1;
        return;
    }

    Memcpy(x, &self->data[self->offset], sizeof(*x));
    self->offset += sizeof(*x);
}

void TPMBufUnpackU16(
    TPMBuf* self,
    UINT16* x)
{
    UINT16 tmp;
    UINT32 r = self->size - self->offset;

    if (self->error)
        return;

    if (sizeof(tmp) > r)
    {
        self->error = 1;
        return;
    }

    Memcpy(&tmp, &self->data[self->offset], sizeof(tmp));
    self->offset += sizeof(tmp);
    *x = ByteSwapU16(tmp);
}

void TPMBufUnpackU32(
    TPMBuf* self,
    UINT32* x)
{
    UINT32 tmp;
    UINT32 r = self->size - self->offset;

    if (self->error)
        return;

    if (sizeof(tmp) > r)
    {
        self->error = 1;
        return;
    }

    Memcpy(&tmp, &self->data[self->offset], sizeof(tmp));
    self->offset += sizeof(tmp);
    *x = ByteSwapU32(tmp);
}

#if 0
void TPMBufUnpackU64(
    TPMBuf* self,
    UINT64* x)
{
    UINT64 tmp;
    UINT32 r = self->size - self->offset;

    if (self->error)
        return;

    if (sizeof(tmp) > r)
    {
        self->error = 1;
        return;
    }

    Memcpy(&tmp, &self->data[self->offset], sizeof(tmp));
    self->offset += sizeof(tmp);
    *x = ByteSwapU64(tmp);
}
#endif

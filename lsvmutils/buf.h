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
#ifndef _buf_h
#define _buf_h

#include "config.h"
#include <lsvmutils/eficommon.h>

#if !defined(BUILD_EFI)
# include <stddef.h>
#endif /* defined(BUILD_EFI) */

/*
**==============================================================================
**
** _Buf
**
**==============================================================================
*/

#define BUF_INITIALIZER { NULL, 0, 0 }

typedef struct _Buf
{
    void* data;
    UINTN size;
    UINTN cap;
}
Buf;

void BufInit(
    Buf* buf);

void BufRelease(
    Buf* buf);

int BufClear(
    Buf* buf);

int BufReserve(
    Buf* buf,
    UINTN cap);

int BufAppend(
    Buf* buf,
    const void* data,
    UINTN size);

/*
**==============================================================================
**
** _BufU32
**
**==============================================================================
*/

#define BUF_U32_INITIALIZER { NULL, 0, 0 }

typedef struct _BufU32
{
    UINT32* data;
    UINTN size;
    UINTN cap;
}
BufU32;

void BufU32Release(
    BufU32* buf);

int BufU32Append(
    BufU32* buf,
    const UINT32* data,
    UINTN size);

/*
**==============================================================================
**
** _BufPtr
**
**==============================================================================
*/

#define BUF_PTR_INITIALIZER { NULL, 0, 0 }

typedef struct _BufPtr
{
    void** data;
    UINTN size;
    UINTN cap;
}
BufPtr;

void BufPtrRelease(
    BufPtr* buf);

int BufPtrAppend(
    BufPtr* buf,
    const void** data,
    UINTN size);

#endif /* _buf_h */

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
#ifndef _byteorder_h
#define _byteorder_h

#include "config.h"
#include "eficommon.h"
#include "inline.h"

/*
**==============================================================================
**
** IS_BIG_ENDIAN
**
**==============================================================================
*/

INLINE BOOLEAN __IsBigEndian()
{
    typedef union _U
    {
        unsigned short x;
        unsigned char bytes[2];
    }
    U;
    static U u = { 0xABCD };
    return u.bytes[0] == 0xAB ? TRUE : FALSE;
}

#if defined(__i386) || defined(__x86_64)
# define IS_BIG_ENDIAN FALSE
#endif

#if !defined(IS_BIG_ENDIAN)
# define IS_BIG_ENDIAN __IsBigEndian()
#endif

/*
**==============================================================================
**
** ByteSwapU16()
** ByteSwapU32()
** ByteSwapU64()
**
**==============================================================================
*/

INLINE UINT64 ByteSwapU64(UINT64 x)
{
    if (IS_BIG_ENDIAN)
    {
        return x;
    }
    else
    {
        return
            ((UINT64)((x & 0xFF) << 56)) |
            ((UINT64)((x & 0xFF00) << 40)) |
            ((UINT64)((x & 0xFF0000) << 24)) |
            ((UINT64)((x & 0xFF000000) << 8)) |
            ((UINT64)((x & 0xFF00000000) >> 8)) |
            ((UINT64)((x & 0xFF0000000000) >> 24)) |
            ((UINT64)((x & 0xFF000000000000) >> 40)) |
            ((UINT64)((x & 0xFF00000000000000) >> 56));
    }
}

INLINE UINT32 ByteSwapU32(UINT32 x)
{
    if (IS_BIG_ENDIAN)
    {
        return x;
    }
    else
    {
        return
            ((UINT32)((x & 0x000000FF) << 24)) |
            ((UINT32)((x & 0x0000FF00) << 8)) |
            ((UINT32)((x & 0x00FF0000) >> 8)) |
            ((UINT32)((x & 0xFF000000) >> 24));
    }
}

INLINE UINT16 ByteSwapU16(UINT16 x)
{
    if (IS_BIG_ENDIAN)
    {
        return x;
    }
    else
    {
        return
            ((UINT16)((x & 0x00FF) << 8)) |
            ((UINT16)((x & 0xFF00) >> 8));
    }
}

#endif /* _byteorder_h */

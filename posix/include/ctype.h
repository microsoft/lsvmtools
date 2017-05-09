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
#ifndef _posix_ctype_h
#define _posix_ctype_h

#include <posix.h>

POSIXEFI_INLINE int posix_toupper(int c)
{
    if (c >= 'a' && c <= 'z')
        c -= ' ';

    return c;
}

POSIXEFI_INLINE int toupper(int c)
{
    return posix_toupper(c);
}

POSIXEFI_INLINE int posix_tolower(int c)
{
    if (c >= 'A' && c <= 'Z')
        c += ' ';

    return c;
}

POSIXEFI_INLINE int posix_isupper(int c)
{
    if (c >= 'A' && c <= 'Z')
        return 1;
    return 0;
}

POSIXEFI_INLINE int posix_isalpha(int c)
{
    if (c >= 'a' && c <= 'z')
        return 1;
    if (c >= 'A' && c <= 'Z')
        return 1;
    return 0;
}

POSIXEFI_INLINE int posix_isdigit(int c)
{
    if (c >= '0' && c <= '9')
        return 1;
    return 0;
}

POSIXEFI_INLINE int posix_isalnum(int c)
{
    if (posix_isalpha(c) || posix_isdigit(c))
        return 1;
    return 0;
}

POSIXEFI_INLINE int posix_isxdigit(int c)
{
    if (posix_isdigit(c))
        return 1;
    if (c >= 'a' && c <= 'f')
        return 1;
    if (c >= 'A' && c <= 'F')
        return 1;
    return 0;
}

POSIXEFI_INLINE int posix_isspace(int c)
{
    switch (c)
    {
        case ' ':
        case '\f':
        case '\n':
        case '\r':
        case '\t':
        case '\v':
            return 1;
        default:
            return 0;
    }

    return 0;
}

POSIXEFI_INLINE int posix_isgraph(int c)
{
    /* ATTN: not implemented */
    return 0;
}

#endif /* _posix_ctype_h */

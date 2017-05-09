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
#ifndef _strings_h
#define _strings_h

#include "config.h"
#include "eficommon.h"

#if defined(BUILD_EFI)
# include <ctype.h> /* From posix library */
# include <string.h> /* From posix library */
#else
# include <ctype.h>
# include <string.h>
# include <stdlib.h>
# include <stdio.h>
#endif

#include "inline.h"

/*
**==============================================================================
**
** Format specifiers casts:
**
**==============================================================================
*/

INLINE UINT32 U32(UINT32 x)
{
    return x;
}

/*
**==============================================================================
**
** Char functions:
**
**==============================================================================
*/

INLINE int Toupper(int c)
{
#if defined(BUILD_EFI)
    return posix_toupper(c);
#else
    return toupper(c);
#endif
}

INLINE int Tolower(int c)
{
#if defined(BUILD_EFI)
    return posix_tolower(c);
#else
    return tolower(c);
#endif
}

INLINE int Isspace(char c)
{
#if defined(BUILD_EFI)
    return posix_isspace(c);
#else
    return isspace(c);
#endif
}

INLINE int Isalpha(char c)
{
#if defined(BUILD_EFI)
    return posix_isalpha(c);
#else
    return isalpha(c);
#endif
}

INLINE int Isdigit(char c)
{
#if defined(BUILD_EFI)
    return posix_isdigit(c);
#else
    return isdigit(c);
#endif
}

INLINE int Isalnum(char c)
{
#if defined(BUILD_EFI)
    return posix_isalnum(c);
#else
    return isalnum(c);
#endif
}

/*
**==============================================================================
**
** Mem functions:
**
**==============================================================================
*/

INLINE int Memcmp(
    const void* dest,
    const void* src,
    unsigned long size)
{
#if defined(BUILD_EFI)
    return posix_memcmp(dest, src, size);
#else
    return memcmp(dest, src, size);
#endif
}

INLINE void* Memset(
    void* dest,
    unsigned char ch,
    unsigned long size)
{
#if defined(BUILD_EFI)
    return posix_memset(dest, ch, size);
#else
    return memset(dest, ch, size);
#endif
}

INLINE void* Memcpy(
    void* dest,
    const void* src,
    unsigned long size)
{
#if defined(BUILD_EFI)
    return posix_memcpy(dest, src, size);
#else
    return memcpy(dest, src, size);
#endif
}

void* Memdup(const void* data, UINTN size);

void* Memstr(
    const void* haystack,
    UINTN haystackSize,
    const void* needle,
    UINTN needleSize);

/*
**==============================================================================
**
** Str functions:
**
**==============================================================================
*/

INLINE const char* Str(const char* str)
{
    return str;
}

INLINE int Strcmp(
    const char* s1,
    const char* s2)
{
#if defined(BUILD_EFI)
    return posix_strcmp(s1, s2);
#else
    return strcmp(s1, s2);
#endif
}

INLINE int Strncmp(
    const char* s1,
    const char* s2,
    UINTN n)
{
#if defined(BUILD_EFI)
    return posix_strncmp(s1, s2, n);
#else
    return strncmp(s1, s2, n);
#endif
}

INLINE char* Strchr(
    const char* s,
    char c)
{
#if defined(BUILD_EFI)
    return posix_strchr(s, c);
#else
    return strchr(s, c);
#endif
}

INLINE char* Strrchr(
    const char* s,
    char c)
{
#if defined(BUILD_EFI)
    return posix_strrchr(s, c);
#else
    return strrchr(s, c);
#endif
}

INLINE UINTN Strlen(
    const char* s)
{
#if defined(BUILD_EFI)
    return posix_strlen(s);
#else
    return strlen(s);
#endif
}

INLINE const char* Strstr(
    const char* haystack,
    const char* needle)
{
#if defined(BUILD_EFI)
    return posix_strstr(haystack, needle);
#else
    return strstr(haystack, needle);
#endif
}

INLINE char* Strcpy(
    char* dest, 
    const char* src)
{
#if defined(BUILD_EFI)
    return posix_strcpy(dest, src);
#else
    return strcpy(dest, src);
#endif
}

char* Strncat(
    char* dest, 
    UINTN destSize,
    const char* src,
    UINTN len);

char *Strtok(
    char *str, 
    const char *delim, 
    char **saveptr);

char* Strdup(
    const char* str);

char* Strndup(
    const char* str, 
    UINTN len);

char* Strtoupper(
    char* str);

char* Strrtrim(
    char* str);

UINTN Strlcpy(
    char* dest, 
    const char* src, 
    UINTN size);

UINTN Strlcat(
    char* dest, 
    const char* src, 
    UINTN size);

typedef struct _U32tostrBuf
{
    char data[11];
}
U32tostrBuf;

const char* U32tostr(
    U32tostrBuf* buf,
    UINT32 x);

typedef struct _U64tostrBuf
{
    char data[21];
}
U64tostrBuf;

const char* U64tostr(
    U64tostrBuf* buf,
    UINT64 x);

/*
**==============================================================================
**
** Tcs functions:
**
**==============================================================================
*/

#if defined(BUILD_EFI)
# define __TCS(STR) L##STR
# define TCS(STR) __TCS(STR)
typedef CHAR16 TCHAR;
#endif

#if !defined(BUILD_EFI)
# define TCS(STR) STR
typedef char TCHAR;
#endif

INLINE UINTN Tcslen(
    const TCHAR* str)
{
#if defined(BUILD_EFI)
    return StrLen(str);
#else
    return strlen(str);
#endif
}

INLINE void Tcscpy(
    TCHAR *dest,
    const TCHAR* src)
{
#if defined(BUILD_EFI)
    StrCpy(dest, src);
#else
    strcpy(dest, src);
#endif
}

INLINE char* StrTcscpy(
    char *dest,
    const TCHAR* src)
{
#if defined(BUILD_EFI)
    while (*src)
        *dest++ = (char)*src++;
    *dest = '\0';
    return dest;
#else
    return Strcpy(dest, src);
#endif
}

/*
**==============================================================================
**
** Wcs functions:
**
**==============================================================================
*/

INLINE const CHAR16* Wcs(const CHAR16* wcs)
{
    return wcs;
}

UINTN Wcslen(
    const CHAR16* str);

CHAR16 *Wcsrtrim(
    CHAR16* str);

CHAR16* Wcsdup2(
    const CHAR16* s1,
    const CHAR16* s2);

CHAR16* Wcschr(
    const CHAR16* wcs,
    CHAR16 ch);

CHAR16* Wcsrchr(
    const CHAR16* wcs,
    CHAR16 ch);

CHAR16* WcsStrdup(
    const char* str);

const CHAR16* Wcswcs(
    const CHAR16* src,
    const CHAR16* pattern);

UINTN Wcslcpy(
    CHAR16* dest,
    const CHAR16* src,
    UINTN size);

UINTN Wcslcat(
    CHAR16* dest,
    const CHAR16* src,
    UINTN size);

UINTN WcsStrlcpy(
    CHAR16* dest,
    const char* src,
    UINTN size);

UINTN WcsStrlcat(
    CHAR16* dest,
    const char* src,
    UINTN size);

UINTN StrWcslcpy(
    char* dest,
    const CHAR16* src,
    UINTN size);

UINTN StrWcslcat(
    char* dest,
    const CHAR16* src,
    UINTN size);

int AppendElem(
    void** data, 
    UINTN* size, 
    UINTN elemSize,
    const void* elem);

#endif /* _strings_h */

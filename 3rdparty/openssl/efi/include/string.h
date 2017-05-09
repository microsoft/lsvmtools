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
#ifndef _efibuild_string_h
#define _efibuild_string_h

#include <stddef.h>
#include <efi.h>
#include <efilib.h>
#include <stdlib.h>
#include <ctype.h>

void __warn(const char* func);

static __inline__ void* memset(void* s, int c, size_t n)
{
    SetMem(s, n, c);
    return s;
}

static __inline__ void* memcpy(void* s1, const void* s2, size_t n)
{
    CopyMem((void*)s1, (void*)s2, n);
    return s1;
}

static __inline__ void* memmove(void* s1, const void* s2, size_t n)
{
    void* p;
    
    /* ATTN: rewrite this! */
    if ((p = malloc(n)))
    {
        memcpy(p, s2, n);
        memcpy(s1, p, n);
        free(p);
    }
    else
        __warn("memmove");

    return s1;
}

static __inline__ int memcmp(const void* s1, const void* s2, size_t n)
{
    return CompareMem((void*)s1, (void*)s2, n);
}

static __inline__ size_t strlen(const char *s)
{
    return strlena((CONST CHAR8*)s);
}

static __inline__ int strcmp(const char *s1, const char *s2)
{
    return strcmpa((CONST CHAR8*)s1, (CONST CHAR8*)s2);
}

static __inline__  int strncmp(const char *s1, const char *s2, size_t n)
{
    return strncmpa((CONST CHAR8*)s1, (CONST CHAR8*)s2, n);
}

static __inline__ char *strcpy(char *dest, const char *src)
{
    char* p = dest;

    while (*src)
        *p++ = *src++;

    *p = '\0';

    return dest;
}

static __inline__ char* strcat(char* dest, const char* str)
{
    return strcpy(dest + strlen(dest), str);
}

static __inline__ char *strncpy(char *dest, const char *src, size_t n)
{
    char* p = dest;

    while (n-- && *src)
        *p++ = *src++;

    *p = '\0';

    return dest;
}

static __inline__ int strncasecmp(const char* s1, const char* s2, size_t n)
{
    while (n && *s1 && *s2 && toupper(*s1) == toupper(*s2))
    {
        n--;
        s1++;
        s2++;
    }

    if (n == 0)
        return 0;

    if (!*s1)
        return -1;
    else if (!*s2)
        return 1;
    else
        return toupper(*s1) - toupper(*s2);
}

static __inline__ int strcasecmp(const char* s1, const char* s2)
{
    while (*s1 && *s2 && toupper(*s1) == toupper(*s2))
    {
        s1++;
        s2++;
    }

    if (*s1)
        return 1;

    if (*s2)
        return -1;

    return 0;
}

static __inline__ char* __strchr(const char* s, char c)
{
    char* p = (char*)s;

    while (*p)
    {
        if (*p == c)
            return p;

        p++;
    }

    return NULL;
}

#define strchr __strchr

static __inline__ char* __strrchr(const char* s, int c)
{
    char* p = (char*)s + strlen(s);

    while (p != s)
    {
        if (*--p == c)
            return p;
    }

    return NULL;
}

#define strrchr __strrchr

static __inline__ const char* strstr(
    const char* src,
    const char* pattern)
{
    UINTN srcLen = strlen(src);
    UINTN patternLen = strlen(pattern);
    UINTN i;

    if (patternLen > srcLen)
        return NULL;

    for (i = 0; i < srcLen - patternLen + 1; i++)
    {
        if (memcmp(src + i, pattern, patternLen) == 0)
            return src + i;
    }

    return NULL;
}

static __inline__ void *memchr(const void *s, int c, size_t n)
{
    char* p;

    for (p = (char*)s; n--; p++)
    {
        if (*p == c)
            return p;
    }

    return NULL;
}

#endif /* _efibuild_string_h */

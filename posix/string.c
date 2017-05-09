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
#define POSIXEFI_SUPPRESS_DEFINITIONS
#include <posix.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <ctype.h>

#if defined(BUILD_EFI)
# include <efi.h>
# include <efilib.h>
#endif

void* posix_memset(void* s, int c, posix_size_t n)
{
#if defined(BUILD_EFI)
    SetMem(s, n, c);
#else
    extern void* memset(void* s, int c, posix_size_t n);
    return memset(s, c, n);
#endif
    return s;
}

void* posix_memcpy(void* s1, const void* s2, posix_size_t n)
{
#if defined(BUILD_EFI)
    CopyMem((void*)s1, (void*)s2, n);
#else
    extern void* memcpy(void* s1, const void* s2, posix_size_t n);
    return memcpy(s1, s2, n);
#endif
    return s1;
}

int posix_memcmp(const void* s1, const void* s2, posix_size_t n)
{
#if defined(BUILD_EFI)
    return CompareMem((void*)s1, (void*)s2, n);
#else
    int memcmp(const void* s1, const void* s2, posix_size_t n);
    return memcmp(s1, s2, n);
#endif
}

posix_size_t posix_strlen(const char *s)
{
    const char* start = s;

    while (*s)
        s++;

    return s - start;
}

int posix_strcmp(const char *s1, const char *s2)
{
#if defined(BUILD_EFI)
    return strcmpa((CONST CHAR8*)s1, (CONST CHAR8*)s2);
#else
    extern int strcmp(const char *s1, const char *s2);
    return strcmp(s1, s2);
#endif
}

int posix_strncmp(const char *s1, const char *s2, posix_size_t n)
{
#if defined(BUILD_EFI)
    return strncmpa((CONST CHAR8*)s1, (CONST CHAR8*)s2, n);
#else
    extern int strncmp(const char *s1, const char *s2, posix_size_t n);
    return strncmp(s1, s2, n);
#endif
}

char* posix_strcat(char* dest, const char* str)
{
    return posix_strcpy(dest + posix_strlen(dest), str);
}

void __posix_warn(const char* func);

/*
    posix_memmove(): tests:

    {
        char str[] = "aaabbbcccdddeee";
        posix_memmove(str, str + 3, 4);
        assert(strcmp(str, "bbbcbbcccdddeee") == 0);
    }

    {
        char str[] = "aaabbbcccdddeee";
        posix_memmove(str + 3, str, 6);
        assert(strcmp(str, "aaaaaabbbdddeee") == 0);
    }

    {
        char str[] = "aaabbbcccdddeee";
        posix_memmove(str + 3, str + 6, 9);
        assert(strcmp(str, "aaacccdddeeeeee") == 0);
    }
*/
void* posix_memmove(void* dest_, const void* src_, posix_size_t n)
{
    char *dest = (char*)dest_;
    const char *src = (const char*)src_;

    if (dest != src && n > 0)
    {
        if (dest <= src)
        {
            posix_memcpy(dest, src, n);
        }
        else
        {
            for (src += n, dest += n; n--; dest--, src--)
                dest[-1] = src[-1];
        }
    }

    return dest;
}

char *posix_strcpy(char *dest, const char *src)
{
    char* p = dest;

    while (*src)
        *p++ = *src++;

    *p = '\0';

    return dest;
}

char *posix_strncpy(char *dest, const char *src, posix_size_t n)
{
    char* p = dest;

    while (n-- && *src)
        *p++ = *src++;

    *p = '\0';

    return dest;
}

int posix_strncasecmp(const char* s1, const char* s2, posix_size_t n)
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

int posix_strcasecmp(const char* s1, const char* s2)
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

char* posix_strchr(const char* s, char c)
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

char* posix_strrchr(const char* s, int c)
{
    char* p = (char*)s + posix_strlen(s);

    while (p != s)
    {
        if (*--p == c)
            return p;
    }

    return NULL;
}

const char* posix_strstr(
    const char* src,
    const char* pattern)
{
    posix_size_t srcLen = posix_strlen(src);
    posix_size_t patternLen = posix_strlen(pattern);
    posix_size_t i;

    if (patternLen > srcLen)
        return NULL;

    for (i = 0; i < srcLen - patternLen + 1; i++)
    {
        if (posix_memcmp(src + i, pattern, patternLen) == 0)
            return src + i;
    }

    return NULL;
}

void *posix_memchr(const void *s, int c, posix_size_t n)
{
    char* p;

    for (p = (char*)s; n--; p++)
    {
        if (*p == c)
            return p;
    }

    return NULL;
}

char* posix_strdup(const char* s)
{
    posix_size_t len = posix_strlen(s);
    char* p;

    if (!(p = posix_malloc(len + 1)))
        return posix_NULL;

    return posix_memcpy(p, s, len + 1);
}

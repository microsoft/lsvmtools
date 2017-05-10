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
#include "strings.h"
#include "alloc.h"

void* Memdup(const void* data, UINTN size)
{
    void* p;

    if (!data || !size)
        return NULL;

    if (!(p = Malloc(size)))
        return NULL;

    return Memcpy(p, data, size);
}

void* Memstr(
    const void* haystack,
    UINTN haystackSize,
    const void* needle,
    UINTN needleSize)
{
    UINTN i;

    if (needleSize <= haystackSize)
    {
        for (i = 0; i <= haystackSize - needleSize; i++)
        {
            UINT8* p = (UINT8*)haystack + i;

            if (Memcmp(p, needle, needleSize) == 0)
                return p;
        }
    }

    /* Not found */
    return NULL;
}

#define ARRCPY(DEST, SRC, COUNT) \
    do \
    { \
        Memcpy(DEST, SRC, (COUNT) * sizeof((SRC)[0])); \
    } \
    while (0)

char* Strncat(
    char* dest, 
    UINTN destSize,
    const char* src,
    UINTN len)
{
#if defined(__linux__) || defined(BUILD_EFI)
    UINTN n = Strlen(dest);
    UINTN i;

    for (i = 0; (i < len) && (n + i < destSize); i++)
    {
        dest[n + i] = src[i];
    }

    if (n + i < destSize)
        dest[n + i] = '\0';
    else if (destSize > 0)
        dest[destSize - 1] = '\0';

    return dest;
#endif

#if defined(_WIN32)
    strncat_s(dest, destSize, src, len);
    return dest;
#endif
}

char *Strtok(
    char *str, 
    const char *delim, 
    char **saveptr)
{
    char* p = str;
    char* end;

    if (str)
        p = str;
    else if (*saveptr)
        p = *saveptr;
    else
        return NULL;

    /* Find start of next token */
    while (*p && Strchr(delim, *p))
        p++;

    /* Find the end of the next token */
    for (end = p; *end && !Strchr(delim, *end); end++)
        ;

    if (p == end)
        return NULL;

    if (*end)
    {
        *end++ = '\0';
        *saveptr = end;
    }
    else
        *saveptr = NULL;

    return p;
}

#define STRLCPY Strlcpy
#define DEST char
#define SRC char
#include "strlcpy.h"
#undef STRLCPY
#undef DEST
#undef SRC

#define STRLCAT Strlcat
#define DEST char
#define SRC char
#include "strlcat.h"
#undef STRLCAT
#undef DEST
#undef SRC

char* Strdup(const char* str)
{
    UINTN len = Strlen(str);
    char* s;
    UINTN i;

    if (!(s = Malloc(sizeof(char) * (len + 1))))
        return NULL;

    for (i = 0; i < len; i++)
        s[i] = (CHAR16)str[i];

    s[len] = '\0';

    return s;
}

char* Strndup(const char* str, UINTN len)
{
    UINTN n;
    char* p;

    for (n = 0; n < len && str[n]; n++)
        ;

    if (!(p = Malloc(sizeof(char) * (n + 1))))
        return NULL;

    ARRCPY(p, str, n);
    p[n] = '\0';

    return p;
}

char* Strtoupper(
    char* str)
{
    char* p = str;

    while (*p)
    {
        *p = Toupper(*p);
        p++;
    }

    return str;
}

char* Strrtrim(
    char* str)
{
    char* end = str + Strlen(str);

    while (end != str && Isspace(end[-1]))
        *--end = '\0';

    return str;
}

const char* U32tostr(
    U32tostrBuf* buf,
    UINT32 x)
{
    char* end = &buf->data[sizeof(U32tostrBuf)];

    *--end = '\0';

    if (x == 0)
    {
        *--end = '0';
        return end;
    }

    while (x)
    {
        UINT32 m = x % 10;
        *--end = m + '0';
        x = x / 10;
    }

    return end;
}

const char* U64tostr(
    U64tostrBuf* buf,
    UINT64 x)
{
    char* end = &buf->data[sizeof(U64tostrBuf)];

    *--end = '\0';

    if (x == 0)
    {
        *--end = '0';
        return end;
    }

    while (x)
    {
        UINT32 m = x % 10;
        *--end = m + '0';
        x = x / 10;
    }

    return end;
}

#define ARRCPY(DEST, SRC, COUNT) \
    do \
    { \
        Memcpy(DEST, SRC, (COUNT) * sizeof((SRC)[0])); \
    } \
    while (0)

UINTN Wcslen(
    const CHAR16* str)
{
#if defined(BUILD_EFI)
    return StrLen(str);
#else
    UINTN n = 0;
    while (*str++)
        n++;
    return n;
#endif
}

CHAR16 *Wcsrtrim(
    CHAR16* str)
{
    CHAR16* end = str + Wcslen(str);

    while (end != str && Isspace(end[-1]))
        *--end = '\0';

    return str;
}

CHAR16* Wcsdup(const CHAR16* str)
{
    UINTN len = Wcslen(str);
    CHAR16* s;

    if (!(s = Malloc((len + 1) * sizeof(CHAR16))))
    {
        return NULL;
    }

    ARRCPY(s, str, len + 1);

    return s;
}

CHAR16* Wcsdup2(const CHAR16* s1, const CHAR16* s2)
{
    UINTN n1 = Wcslen(s1);
    UINTN n2 = Wcslen(s2);
    CHAR16* s;

    if (!(s = Malloc((n1 + n2 + 1) * sizeof(CHAR16))))
    {
        return NULL;
    }

    ARRCPY(s, s1, n1);
    ARRCPY(s + n1, s2, n2 + 1);

    return s;
}

void Wcscpy(CHAR16* dest, const CHAR16* src)
{
    UINTN len = Wcslen(src);
    UINTN i;

    for (i = 0; i < len; i++)
        dest[i] = src[i];

    dest[len] = '\0';
}

CHAR16* Wcschr(const CHAR16* wcs, CHAR16 ch)
{
    while (*wcs)
    {
        if (*wcs == ch)
            return (CHAR16*)wcs;

        wcs++;
    }

    /* Not found */
    return NULL;
}

CHAR16* Wcsrchr(const CHAR16* wcs, CHAR16 ch)
{
    const CHAR16* p = wcs + Wcslen(wcs);

    while (p != wcs)
    {
        if (*--p == ch)
            return (CHAR16*)p;
    }

    /* Not found */
    return NULL;
}

CHAR16* WcsStrdup(const char* str)
{
    UINTN len = Strlen(str);
    CHAR16* wcs;
    UINTN i;

    if (!(wcs = Malloc(sizeof(CHAR16) * (len + 1))))
        return NULL;

    for (i = 0; i < len; i++)
        wcs[i] = (CHAR16)str[i];

    wcs[len] = '\0';

    return wcs;

}

const CHAR16* Wcswcs(
    const CHAR16* src,
    const CHAR16* pattern)
{
    UINTN srcLen = Wcslen(src);
    UINTN patternLen = Wcslen(pattern);
    UINTN i;

    if (patternLen > srcLen)
        return NULL;

    for (i = 0; i < srcLen - patternLen + 1; i++)
    {
        if (Memcmp(src + i, pattern, patternLen * sizeof(CHAR16)) == 0)
            return src + i;
    }

    return NULL;
}

#define STRLCPY Wcslcpy
#define DEST CHAR16
#define SRC CHAR16
#include "../lsvmutils/strlcpy.h"
#undef STRLCPY
#undef DEST
#undef SRC

#define STRLCAT Wcslcat
#define DEST CHAR16
#define SRC CHAR16
#include "../lsvmutils/strlcat.h"
#undef STRLCAT
#undef DEST
#undef SRC

#define STRLCPY WcsStrlcpy
#define DEST CHAR16
#define SRC char
#include "../lsvmutils/strlcpy.h"
#undef STRLCPY
#undef DEST
#undef SRC

#define STRLCAT WcsStrlcat
#define DEST CHAR16
#define SRC char
#include "../lsvmutils/strlcat.h"
#undef STRLCAT
#undef DEST
#undef SRC

#define STRLCPY StrWcslcpy
#define DEST char
#define SRC CHAR16
#include "../lsvmutils/strlcpy.h"
#undef STRLCPY
#undef DEST
#undef SRC

#define STRLCAT StrWcslcat
#define DEST char
#define SRC CHAR16
#include "../lsvmutils/strlcat.h"
#undef STRLCAT
#undef DEST
#undef SRC

int AppendElem(
    void** data, 
    UINTN* size, 
    UINTN elemSize,
    const void* elem)
{
    void* p;

    if (!data || !size || !elemSize || !elem)
        return -1;

    if (!(p = Realloc(
        *data, /* memory object */
        (*size) * elemSize, /* old size */
        (*size + 1) * elemSize))) /* new size */
    {
        return -1;
    }

    Memcpy((UINT8*)p + ((*size) * elemSize), elem, elemSize);

    *data = p;
    (*size)++;

    return 0;
}

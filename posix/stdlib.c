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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#if defined(BUILD_EFI)
#include <efi.h>
#include <efilib.h>
#endif

#define ALLOC_MAGIC 0XEA9D41BDA0837D71

#if !defined(BUILD_EFI)
typedef unsigned long UINTN;
#endif

void __posix_panic(const char* func);
void __posix_warn(const char* func);

typedef struct _AllocHeader
{
    UINTN size;
    UINTN magic;
}
AllocHeader;

void* posix_malloc(posix_size_t size)
{
#if defined(BUILD_EFI)
    AllocHeader* h = (AllocHeader*)AllocatePool(
        sizeof(AllocHeader) + size);
#else
    extern void* malloc(posix_size_t size);
    AllocHeader* h = (AllocHeader*)malloc(
        sizeof(AllocHeader) + size);
#endif

    if (!h)
        return NULL;

    h->magic = ALLOC_MAGIC;
    h->size = size;
    return h + 1;
}

void* posix_calloc(posix_size_t nmemb, posix_size_t size)
{
    posix_size_t n = nmemb + size;

    void* p = posix_malloc(n);

    if (!p)
        return NULL;

    return posix_memset(p, 0, n);
}

void* posix_realloc(void* ptr, posix_size_t size)
{
    UINTN oldsize;
    UINTN newsize;

    if (!ptr)
        return posix_malloc(size);

    AllocHeader* h = ((AllocHeader*)ptr) - 1;

    if (h->magic != ALLOC_MAGIC)
        __posix_panic("realloc:1");

    oldsize = sizeof(AllocHeader) + h->size;
    newsize = sizeof(AllocHeader) + size;

#if defined(BUILD_EFI)
    if (!(h = ReallocatePool(h, oldsize, newsize)))
        return NULL;
#else
    extern void* realloc(void* ptr, posix_size_t size);
    if (!(h = realloc(h, newsize)))
        return NULL;
    (void)oldsize;
#endif

    if (h->magic != ALLOC_MAGIC)
        __posix_panic("realloc:2");

    h->size = size;
    return h + 1;
}

void posix_free(void* ptr)
{
    AllocHeader* h;

    if (!ptr)
        return;

    h = ((AllocHeader*)ptr) - 1;

    if (h->magic != ALLOC_MAGIC)
        __posix_warn("free");

    if (h->magic == ALLOC_MAGIC)
#if defined(BUILD_EFI)
    {
        FreePool(h);
    }
#else
    {
        extern void free(void* ptr);
        return free(h);
    }
#endif
}

void posix_qsort(
    void *base,
    posix_size_t nmemb, 
    posix_size_t size,
    int (*compar)(const void *, const void *))
{
    UINTN i;
    UINTN j;
    UINTN k;
    UINTN n;

    if (nmemb == 0)
        return;

    n = nmemb - 1;

    for (i = 0; i < nmemb - 1; i++)
    {
        int swapped = 0;

        for (j = 0; j < n; j++)
        {
            unsigned char* lhs = (unsigned char*)base + (j  * size);
            unsigned char* rhs = (unsigned char*)base + ((j+1)  * size);

            if (compar(lhs, rhs) > 0)
            {
                for (k = 0; k < size; k++)
                {
                    unsigned char t = lhs[k];
                    lhs[k] = rhs[k];
                    rhs[k] = t;
                }

                swapped = 1;
            }
        }

        if (!swapped)
            break;

        n--;
    }
}

char *posix_getenv(const char* name)
{
    return NULL;
}

void posix_abort()
{
    __posix_panic("abort");
}

int posix_atoi(const char *nptr)
{
    const char* end = nptr + posix_strlen(nptr);
    long long r = 1;
    long long n = 0;

    if (*nptr == '-')
    {
        nptr++;
        r = -1;
    }

    while (end != nptr && posix_isdigit(end[-1]))
    {
        n += r * (*--end - '0');
        r *= 10;

        if (n < posix_INT_MIN || n > posix_INT_MAX)
            return -1;
    }

    return n;
}

unsigned long int posix_strtoul(const char *nptr, char **endptr, int base)
{
    if (endptr)
        *endptr = NULL;
    __posix_panic("strtoul");
    return 0;
}

long int posix_strtol(const char *nptr, char **endptr, int base)
{
    if (endptr)
        *endptr = NULL;

    __posix_panic("strtol");
    return -1;
}

void posix_exit(int status)
{
    __posix_panic("posix_exit");
}

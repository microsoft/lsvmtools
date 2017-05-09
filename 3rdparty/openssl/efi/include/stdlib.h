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
#ifndef _efibuild_stdlib_h
#define _efibuild_stdlib_h

#include <stddef.h>
#include <efi.h>
#include <efilib.h>

#define ALLOC_MAGIC 0XEA9D41BDA0837D71

void __panic(const char* func);
void __warn(const char* func);

typedef struct _AllocHeader
{
    UINTN size;
    UINTN magic;
}
AllocHeader;

static __inline__ void* malloc(size_t size)
{
    AllocHeader* h = (AllocHeader*)AllocatePool(
        sizeof(AllocHeader) + size);

    if (!h)
        return NULL;

    h->magic = ALLOC_MAGIC;
    h->size = size;
    return h + 1;
}

static __inline__ void* realloc(void* ptr, size_t size)
{
    UINTN oldsize;
    UINTN newsize;

    if (!ptr)
        return malloc(size);

    AllocHeader* h = ((AllocHeader*)ptr) - 1;

    if (h->magic != ALLOC_MAGIC)
        __panic("realloc:1");

    oldsize = sizeof(AllocHeader) + h->size;
    newsize = sizeof(AllocHeader) + size;

    if (!(h = ReallocatePool(h, oldsize, newsize)))
        return NULL;

    if (h->magic != ALLOC_MAGIC)
        __panic("realloc:2");

    h->size = size;
    return h + 1;
}

static __inline__ void free(void* ptr)
{
    AllocHeader* h;

    if (!ptr)
        return;

    h = ((AllocHeader*)ptr) - 1;

    if (h->magic != ALLOC_MAGIC)
        __warn("free");

    if (h->magic == ALLOC_MAGIC)
        FreePool(h);
}

static __inline__ void qsort(
    void *base, 
    size_t nmemb, 
    size_t size,
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
        BOOLEAN swapped = FALSE;

        for (j = 0; j < n; j++)
        {
            UINT8* lhs = (UINT8*)base + (j  * size);
            UINT8* rhs = (UINT8*)base + ((j+1)  * size);

            if (compar(lhs, rhs) > 0)
            {
                for (k = 0; k < size; k++)
                {
                    UINT8 t = lhs[k];
                    lhs[k] = rhs[k];
                    rhs[k] = t;
                }

                swapped = TRUE;
            }
        }

        if (!swapped)
            break;

        n--;
    }
}

static __inline__ char *getenv(const char* name)
{
    return NULL;
}

static __inline__ void abort()
{
    __panic("abort");
}

static __inline__ int atoi(const char *nptr)
{
    __panic("atoi");
    return -1;
}

static __inline__ unsigned long int strtoul(
    const char *nptr, 
    char **endptr, 
    int base)
{
    if (endptr)
        *endptr = NULL;
    __panic("strtoul");
    return 0;
}

static __inline__ long int strtol(const char *nptr, char **endptr, int base)
{
    if (endptr)
        *endptr = NULL;

    __panic("strtol");
    return -1;
}

#endif /* _efibuild_stdlib_h */

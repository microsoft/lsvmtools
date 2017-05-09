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
#ifndef _posix_stdlib_h
#define _posix_stdlib_h

#include <posix.h>
#include <stddef.h>

/* ATTN: move this someplace else! */
#ifndef lzma_attribute
# define lzma_attribute(attr)
#endif

void* posix_malloc(posix_size_t size);

void* posix_calloc(posix_size_t nmemb, posix_size_t size);

void* posix_realloc(void* ptr, posix_size_t size);

void posix_free(void* ptr);

void posix_qsort(
    void *base, 
    posix_size_t nmemb, 
    posix_size_t size,
    int (*compar)(const void *, const void *));

char *posix_getenv(const char* name);

void posix_abort();

int posix_atoi(const char *nptr);

unsigned long int posix_strtoul(const char *nptr, char **endptr, int base);

long int posix_strtol(const char *nptr, char **endptr, int base);

void posix_exit(int status);

#endif /* _posix_stdlib_h */

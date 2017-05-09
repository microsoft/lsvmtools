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
#ifndef _efibuild_stdio_h
#define _efibuild_stdio_h

#include <stddef.h>
#include <efi.h>
#include <efilib.h>

#define BUFSIZ 8192

typedef struct __FILE FILE;

#define stdout ((FILE*)0)
#define stderr ((FILE*)0)

void __panic(const char* func);

static __inline__ int printf(const char *format, ...)
{
    __panic("printf");
    return 0;
}

static __inline__ int fprintf(FILE *stream, const char *format, ...)
{
    __panic("fprintf");
    return 0;
}

static __inline__ int sscanf(const char *str, const char *format, ...)
{
    __panic("sscanf");
    return 0;
}

static __inline__ int vfprintf(FILE *stream, const char *format, va_list ap)
{
    __panic("vfprintf");
    return 0;
}

static __inline__ int sprintf(char *str, const char *format, ...)
{
    __panic("sprintf");
    return 0;
}

static __inline__ int fputs(const char *s, FILE *stream)
{
    __panic("fputs");
    return -1;
}

static __inline__ FILE *fopen(const char *path, const char *mode)
{
    __panic("fopen");
    return NULL;
}

static __inline__ int fclose(FILE *stream)
{
    __panic("fclose");
    return -1;
}

static __inline__ size_t fread(
    void *ptr, 
    size_t size, 
    size_t nmemb, 
    FILE *stream)
{
    __panic("fread");
    return 0;
}

static __inline__ size_t fwrite(
    const void *ptr, 
    size_t size, 
    size_t nmemb,
    FILE *stream)
{
    __panic("fwrite");
    return 0;
}

static __inline__ int feof(FILE *stream)
{
    __panic("feof");
    return 0;
}

static __inline__ int fseek(FILE *stream, long offset, int whence)
{
    __panic("stream");
    return -1;
}

static __inline__ int ferror(FILE *stream)
{
    __panic("ferror");
    return -1;
}

static __inline__ long ftell(FILE *stream)
{
    __panic("ftell");
    return -1;
}

static __inline__ int fflush(FILE *stream)
{
    __panic("fflush");
    return -1;
}

static __inline__ char *fgets(char *s, int size, FILE *stream)
{
    __panic("fgets");
    return NULL;
}

#endif /* _efibuild_stdio_ */

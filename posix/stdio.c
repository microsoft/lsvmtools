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
#include <stdio.h>

#if defined(BUILD_EFI)
# include <efi.h>
# include <efilib.h>
#endif

void __posix_panic(const char* func);

int posix_printf(const char *format, ...)
{
    __posix_panic("printf");
    return 0;
}

int posix_fprintf(posix_FILE *stream, const char *format, ...)
{
    __posix_panic("fprintf");
    return 0;
}

int posix_sscanf(const char *str, const char *format, ...)
{
    __posix_panic("sscanf");
    return 0;
}

int posix_vfprintf(
    posix_FILE *stream, 
    const char *format, 
    posix_va_list ap)
{
    __posix_panic("vfprintf");
    return 0;
}

int posix_sprintf(char *str, const char *format, ...)
{
    __posix_panic("sprintf");
    return 0;
}

int posix_vsprintf(char *str, const char *format, posix_va_list ap)
{
    __posix_panic("posix_vsprintf");
    return 0;
}

int posix_fputs(const char *s, posix_FILE *stream)
{
    __posix_panic("fputs");
    return -1;
}

int posix_putc(int c, posix_FILE *stream)
{
    __posix_panic("putc");
    return -1;
}

posix_FILE *posix_fopen(const char *path, const char *mode)
{
    __posix_panic("fopen");
    return NULL;
}

int posix_fclose(posix_FILE *stream)
{
    __posix_panic("fclose");
    return -1;
}

posix_size_t posix_fread(
    void *ptr, 
    posix_size_t size, 
    posix_size_t nmemb, 
    posix_FILE *stream)
{
    __posix_panic("fread");
    return 0;
}

posix_size_t posix_fwrite(
    const void *ptr, 
    posix_size_t size, 
    posix_size_t nmemb,
    posix_FILE *stream)
{
    __posix_panic("fwrite");
    return 0;
}

int posix_feof(posix_FILE *stream)
{
    __posix_panic("feof");
    return 0;
}

int posix_fseek(posix_FILE *stream, long offset, int whence)
{
    __posix_panic("stream");
    return -1;
}

int posix_ferror(posix_FILE *stream)
{
    __posix_panic("ferror");
    return -1;
}

long posix_ftell(posix_FILE *stream)
{
    __posix_panic("ftell");
    return -1;
}

int posix_fflush(posix_FILE *stream)
{
    __posix_panic("fflush");
    return -1;
}

char *posix_fgets(char *s, int size, posix_FILE *stream)
{
    __posix_panic("fgets");
    return NULL;
}

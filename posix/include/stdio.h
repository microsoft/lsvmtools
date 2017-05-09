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
#ifndef _posix_stdio_h
#define _posix_stdio_h

#include <posix.h>
#include <stddef.h>
#include <stdarg.h>

#if defined(BUILD_EFI)
# include <efi.h>
# include <efilib.h>
#else
typedef int posix_va_list;
#endif

#define posix_BUFSIZ 8192

typedef struct __posix_FILE posix_FILE;

#define posix_stdout ((posix_FILE*)0)
#define posix_stderr ((posix_FILE*)0)

int posix_printf(const char *format, ...);

int posix_fprintf(posix_FILE *stream, const char *format, ...);

int posix_sscanf(const char *str, const char *format, ...);

int posix_vfprintf(
    posix_FILE *stream, 
    const char *format, 
    posix_va_list ap);

int posix_vsprintf(
    char *str, 
    const char *format, 
    posix_va_list ap);

int posix_sprintf(char *str, const char *format, ...);

int posix_fputs(const char *s, posix_FILE *stream);

int posix_putc(int c, posix_FILE *stream);

posix_FILE *posix_fopen(const char *path, const char *mode);

int posix_fclose(posix_FILE *stream);

posix_size_t posix_fread(void *ptr, posix_size_t size, posix_size_t nmemb, posix_FILE *stream);

posix_size_t posix_fwrite(const void *ptr, posix_size_t size, posix_size_t nmemb, posix_FILE *stream);

int posix_feof(posix_FILE *stream);

int posix_fseek(posix_FILE *stream, long offset, int whence);

int posix_ferror(posix_FILE *stream);

long posix_ftell(posix_FILE *stream);

int posix_fflush(posix_FILE *stream);

char *posix_fgets(char *s, int size, posix_FILE *stream);

#endif /* _posix_stdio_ */

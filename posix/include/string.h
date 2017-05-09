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
#ifndef _posix_string_h
#define _posix_string_h

#include <posix.h>
#include <stddef.h>

#if defined(BUILD_EFI)
# include <efi.h>
# include <efilib.h>
#endif

void* posix_memset(void* s, int c, posix_size_t n);

void* posix_memcpy(
    void* s1, 
    const void* s2, 
    posix_size_t n);

void* posix_memcpy(void* s1, const void* s2, posix_size_t n);

void* posix_memmove(void* s1, const void* s2, posix_size_t n);

int posix_memcmp(
    const void* s1, 
    const void* s2, 
    posix_size_t n);

posix_size_t posix_strlen(const char *s);

int posix_strcmp(const char *s1, const char *s2);

int posix_strncmp(
    const char *s1, 
    const char *s2, 
    posix_size_t n);

char *posix_strcpy(char *dest, const char *src);

char* posix_strcat(char* dest, const char* str);

char* posix_strcat(char* dest, const char* str);

char *posix_strncpy(char *dest, const char *src, posix_size_t n);

int posix_strncasecmp(const char* s1, const char* s2, posix_size_t n);

int posix_strcasecmp(const char* s1, const char* s2);

char* posix_strchr(const char* s, char c);

char* posix_strrchr(const char* s, int c);

const char* posix_strstr(const char* src, const char* pattern);

void *posix_memchr(const void *s, int c, posix_size_t n);

char* posix_strdup(const char* s);

#endif /* _posix_string_h */

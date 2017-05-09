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
#ifndef _mbutils_config_h
#define _mbutils_config_h

#if !defined(_WIN32) && !defined(__linux__)
# error "unsupported platform"
#endif

#if defined(__linux__)
# define HAVE_TCG2
#endif

#if defined(__linux__)
# define PACKED __attribute__((packed))
#endif

#if defined(__linux__)
# define HAVE_OPENSSL
#endif

#if defined(BUILD_EFI)
# define HAVE_OPENSSL
#endif

#if defined(_WIN32)
# define PACKED
#endif

#if defined(_WIN32)
# include <windows.h>
#endif

/* define PATH_MAX */
#if defined(__linux__) && !defined(BUILD_EFI)
# include <limits.h>
#elif defined(BUILD_EFI)
# define PATH_MAX 256
#elif defined(_WIN32)
# define PATH_MAX MAX_PATH
#endif

#if defined(_WIN32)
/* Ignore: "warning C4706: assignment within conditional expression" */
# pragma warning(disable : 4706)
#endif

#define ARRSIZE(ARR) (sizeof(ARR) / sizeof(ARR[0]))

#if defined(BUILD_EFI)
void __efi_trace(const char* file, unsigned int line);
# define EFI_TRACE __efi_trace(__FILE__, __LINE__)
#else
# define EFI_TRACE
#endif

#endif /* _mbutils_config_h */

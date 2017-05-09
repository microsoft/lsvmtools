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

#if defined(BUILD_EFI)
# include <efi.h>
# include <efilib.h>
# define VSNPRINTF VSPrint
#else
# include <stdarg.h>
# include <stdio.h>
# define VSNPRINTF vsnprintf
#endif

#include "error.h"

void SetErr(
    Error* err, 
    const TCHAR* format, 
    ...)
{
    if (err)
    {
        TCHAR buf[sizeof(err->buf)];
        va_list ap;

        va_start(ap, format);
        VSNPRINTF(buf, sizeof(buf), (TCHAR*)format, ap);
        Tcscpy(err->buf, buf);
        va_end(ap);
    }
}

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
#include "print.h"

#if !defined(BUILD_EFI)
# include <stdarg.h>
#endif

#if defined(BUILD_EFI)
int __Printf(const CHAR16 *format, ...)
{
    CHAR16 buf[128];
    CHAR16 *fmt;
    UINTN n = (StrLen(format) + 1) * sizeof(CHAR16);
    va_list ap;
    int r;
    CHAR16* p;

    if (n >= sizeof(buf))
    {
        if (!(fmt = (CHAR16*)AllocatePool(n)))
            return 0;
    }
    else
        fmt = buf;

    CopyMem(fmt, (void*)format, n);

    /* Translate "%u" to "%d" */
    for (p = fmt; *p; p++)
    {
        if (p[0] == '%' && p[1] == 'u')
            p[1] = 'd';
    }

    va_start(ap, format);
    r = VPrint(fmt, ap);
    va_end(ap);

    if (fmt != buf)
        FreePool(fmt);

    return r;
}
#else /* !defined(BUILD_EFI) */
int __Printf(const char *format, ...)
{
    char buf[128];
    char *fmt;
    size_t n = strlen(format) + 1;
    va_list ap;
    int r;
    char* p;

    if (n >= sizeof(buf))
    {
        if (!(fmt = (char*)malloc(n)))
            return 0;
    }
    else
        fmt = buf;

    memcpy(fmt, format, n);

    /* Translate "%a" to "%s" */
    for (p = fmt; *p; p++)
    {
        if (p[0] == '%' && p[1] == 'a')
            p[1] = 's';
        if (p[0] == '%' && p[1] == '.' && p[2] == '*' && p[3] == 'a')
            p[3] = 's';
    }

    va_start(ap, format);
    r = vprintf(fmt, ap);
    va_end(ap);

    if (fmt != buf)
        free(fmt);

    return r;
}
#endif /* !defined(BUILD_EFI) */

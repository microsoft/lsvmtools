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
#include <string.h>
#include "log.h"
#include "console.h"

int errno;

int posix_errno;

/* Unused but linked function on libcrytoefi.a */
int RAND_poll(void)
{
    LOGE(L"RAND_poll()");
    return -1;
}

/* Unused but linked function on libcrytoefi.a */
void* UI_OpenSSL(void)
{
    LOGE(L"UI_OpenSSL()");
    return NULL;
}

/* Called from OpenSSL crypto library */
void __posix_panic(const char* func)
{
    LOGE(L"__posix_panic(): %a", func);
    *((int*)0) = 0;
}

/* Called from zlibefi library */
void __zlib_panic(const char* func)
{
    LOGE(L"__zlib_panic(): %a", func);
    *((int*)0) = 0;
}

/* Called from OpenSSL crypto library */
void __posix_warn(const char* func)
{
    if (func && strcmp(func, "strerror") == 0)
        return;

    LOGW(L"__posix_warn(): %a", func);
}

void __efi_trace(const char* file, unsigned int line)
{
    Print(L"EFI_TRACE: %a %d\n", file, line); Wait();
    LOGI(L"EFI_TRACE: %a %d", file, line);
}

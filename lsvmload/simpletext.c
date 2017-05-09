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
#include <lsvmutils/strings.h>
#include "simpletext.h"
#include "wait.h"

EFI_STATUS ClearScreen()
{
    EFI_STATUS status = EFI_SUCCESS;

    if ((status = uefi_call_wrapper(
        ST->ConOut->ClearScreen, 
        1, 
        ST->ConOut)) != EFI_SUCCESS)
    {
        goto done;
    }

done:

    return status;
}

EFI_STATUS SetColors(
    UINTN foreground,
    UINTN background)
{
    EFI_STATUS status = EFI_SUCCESS;
    UINTN colors = EFI_TEXT_ATTR(foreground, background);

    if ((status = uefi_call_wrapper(
        ST->ConOut->SetAttribute, 
        2, 
        ST->ConOut,
        colors)) != EFI_SUCCESS)
    {
        goto done;
    }

done:

    return status;
}

#if defined(NEED_OutputString)
EFI_STATUS OutputString(
    const CHAR16* str)
{
    EFI_STATUS status = EFI_SUCCESS;

    if ((status = uefi_call_wrapper(
        ST->ConOut->OutputString, 
        2, 
        ST->ConOut,
        (CHAR16*)str)) != EFI_SUCCESS)
    {
        goto done;
    }

done:

    return status;
}
#endif /* defined(NEED_OutputString) */

EFI_STATUS PrintErrorWait(
    const CHAR16* format, 
    ...)
{
    EFI_STATUS status = EFI_SUCCESS;
    va_list ap;
    CHAR16* wcs = NULL;

    /* Format the string */
    {
        va_start(ap, format);

        if (!(wcs = VPoolPrint((CHAR16*)format, ap)))
        {
            va_end(ap);
            status = EFI_OUT_OF_RESOURCES;
            goto done;
        }

        va_end(ap);
    }

    /* Print string in RED */
    SetColors(EFI_LIGHTRED, EFI_BLACK);
    Print(L"%s\n", Wcs(wcs));
    Wait();
    SetColors(EFI_WHITE, EFI_BLACK);

done:

    if (wcs)
        FreePool(wcs);

    return status;
}

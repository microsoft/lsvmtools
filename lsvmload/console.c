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
#include "console.h"
#include <lsvmutils/strings.h>
#include "log.h"

void WaitWithMessage(const CHAR16* message)
{
    Print(L"%s\n", message);

    for (;;)
    {
        EFI_STATUS status = WaitForSingleEvent(ST->ConIn->WaitForKey, 1000000);
        EFI_INPUT_KEY key;

        if (status == EFI_TIMEOUT)
            continue;

        uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2, ST->ConIn, &key);
        break;
    }
}

void Wait(void)
{
    WaitWithMessage(L"Press any key to continue...\n");
}

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

EFI_STATUS GetInput(
    const CHAR16 *prompt,
    BOOLEAN echo,
    CHAR16 *buf,
    UINTN bufSize)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    EFI_STATUS result;
    EFI_INPUT_KEY key;
    UINTN size = 0;

    /* Check parameters */
    if (!prompt || !buf || bufSize == 0)
    {
        status = EFI_INVALID_PARAMETER;
        goto done;
    }

    /* Initialize the buffer */
    buf[0] = '\0';

    /* Display the prompt if any */
    if (prompt) 
        OutputString(prompt);

    /* Loop until user pressed Enter key */
    for (;;)
    {
        CHAR16 c;

        /* Wait for user to press a key */
        WaitForSingleEvent(ST->ConIn->WaitForKey, 0);

        /* Read the next key stroke */
        if ((result = uefi_call_wrapper(
            ST->ConIn->ReadKeyStroke, 
            2, 
            ST->ConIn, 
            &key)) != EFI_SUCCESS)
        {
            status = result;
            goto done;
        }

        /* Get the next character */
        c = key.UnicodeChar;

        /* Check for enter key */
        if (c == '\n' || c == '\r')
        {
            /* User pressed enter key */
            OutputString(L"\r\n");
            break;
        }
        else if (c == '\b')
        {
            /* Handle backspace */
            if (size && echo)
            {
                /* Backspace, space over character, then backspace again */
                OutputString(L"\b \b");
                size--;
            }
        }
        else if (c >= ' ' && c <= '~')
        {
            /* Fail on buffer overflow */
            if (size + 1 == bufSize)
                goto done;

            /* Append character to output buffer */
            buf[size++] = c;
            buf[size] = '\0';

            /* Write the character to console? */
            if (echo)
                OutputString(&buf[size-1]);
        }
        else
        {
            /* Illegal character */
            break;
        }
    }

    status = EFI_SUCCESS;

done:
    return status;
}

#if 0
static EFI_STATUS EFIAPI _ReadKeyStroke(
    IN struct _SIMPLE_INPUT_INTERFACE *This,
    OUT EFI_INPUT_KEY *Key)
{
    return EFI_UNSUPPORTED;
}
#endif

#if 0
static EFI_STATUS EFIAPI _Reset(
    IN struct _SIMPLE_INPUT_INTERFACE *This,
    IN BOOLEAN ExtendedVerification)
{
    if (This)
        This->ReadKeyStroke = _ReadKeyStroke;

    return EFI_SUCCESS;
}
#endif

#if 0
EFI_STATUS DisableKeyboardInput(EFI_SYSTEM_TABLE *systemTable)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    EFI_EVENT event = NULL;

    if (!systemTable || !systemTable->ConIn)
        goto done;

    /* Create an EVT_TIMER event */
    if ((status = BS->CreateEvent(
        EVT_TIMER, 
        0, 
        NULL, 
        NULL, 
        &event)) != EFI_SUCCESS)
    {
        goto done;
    }

    systemTable->ConIn->WaitForKey = event;
    systemTable->ConIn->Reset = _Reset;
    systemTable->ConIn->ReadKeyStroke = _ReadKeyStroke;

    status = EFI_SUCCESS;

done:
    return status;
}
#endif

EFI_STATUS DisableKeyboardInput(EFI_SYSTEM_TABLE *systemTable)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    static EFI_GUID protocol = SIMPLE_TEXT_INPUT_PROTOCOL;
    SIMPLE_INPUT_INTERFACE sii;

    sii = *ST->ConIn;

    if (uefi_call_wrapper(
        BS->ReinstallProtocolInterface, 
        4, 
        &ST->ConsoleInHandle,
        &protocol,
        ST->ConIn,
        &sii) != EFI_SUCCESS)
    {
        LOGE(L"InstallSimpleInputInterface(): failed to install protocol");
        goto done;
    }

    status = EFI_SUCCESS;

done:
    return status;
}

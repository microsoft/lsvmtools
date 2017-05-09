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
#include "sleep.h"

EFI_STATUS Sleep(const UINT64 seconds)
{
    EFI_STATUS status = EFI_SUCCESS;
    EFI_EVENT event = NULL;
    UINTN index = 0;

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

    /* Set the timer to go off once */
    if ((status = BS->SetTimer(
        event,
        TimerPeriodic, 
        10000000 * seconds)) != EFI_SUCCESS)
    {
        goto done;
    }

    /* Wait for the timer event */
    if ((status = BS->WaitForEvent(1, &event, &index)) != EFI_SUCCESS)
    {
        goto done;
    }

done:

    if (event)
        BS->CloseEvent(event);

    return status;
}

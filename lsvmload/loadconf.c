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
#include <lsvmutils/error.h>
#include <lsvmutils/conf.h>
#include <lsvmutils/alloc.h>
#include <lsvmutils/efifile.h>
#include <lsvmutils/strings.h>
#include "paths.h"
#include "log.h"
#include "loadconf.h"

static int _confCallback(
    const char* name, 
    const char* value,
    void* callbackData,
    Error* err)
{
    int rc = -1;

    if (Strcmp(name, "LogLevel") == 0)
    {
        LogLevel logLevel;

        if (StrToLogLevel(value, &logLevel) != 0)
        {
            SetErr(err, L"bad log level: %a", Str(value));
            goto done;
        }

        SetLogLevel(logLevel);
    }
#if defined(ENABLE_FAULTS) /* Disabled in production */
    else if (Strcmp(name, "Fault") == 0)
    {
        if (AppendFault(value) != 0)
        {
            SetErr(err, L"unknown fault: %a", Str(value));
            goto done;
        }
    }
#endif
    else if (Strcmp(name, "EFIVendorDir") == 0)
    {
        if (!(globals.efiVendorDir = WcsStrdup(value)))
        {
            SetErr(err, L"out of memory");
            goto done;
        }
    }
    else if (Strcmp(name, "BootDeviceLUKS") == 0)
    {
        if (!ValidGUIDStr(value))
        {
            SetErr(err, L"invalid BootDevice option: expected UUID");
            goto done;
        }

        Strcpy(globals.bootDevice, value);
    }
    else if (Strcmp(name, "RootDeviceLUKS") == 0)
    {
        if (!ValidGUIDStr(value))
        {
            SetErr(err, L"invalid RootDevice option: expected UUID");
            goto done;
        }

        Strcpy(globals.rootDevice, value);
    }
    else if (Strcmp(name, "BootDevice") != 0 && Strcmp(name, "RootDevice") != 0)
    {
        // BootDevice/RootDevice are used elsewhere. Any other key is an error
        SetErr(err, L"unknown command: %a", Str(name));
        goto done;
    }

    rc = 0;

done:

    return rc;
}

int LoadConf(
    EFI_HANDLE imageHandle,
    Error* err)
{
    int rc = 0;
    void* data = NULL;
    UINTN size;
    unsigned int errorLine;

    ClearErr(err);

    if (FileExists(imageHandle, globals.lsvmconfPath) != EFI_SUCCESS)
    {
        SetErr(err, L"missing configuration file: %s", Wcs(globals.lsvmconfPath));
        rc = -1;
        goto done;
    }

    if (EFILoadFile(
        imageHandle, 
        globals.lsvmconfPath, 
        &data, 
        &size) != EFI_SUCCESS)
    {
        SetErr(err, L"file to load file: %s", Wcs(globals.lsvmconfPath));
        rc = -1;
        goto done;
    }

    if (ParseConf(
        (char*)data, 
        size, 
        _confCallback, 
        NULL,
        &errorLine, 
        err) != 0)
    {
        rc = -1;
        goto done;
    }

    /* Check mandatory 'EFIVendorDir' option */
    if (!globals.efiVendorDir)
    {
        LOGE(L"%s: Missing EFIVendorDir option", Wcs(globals.lsvmconfPath));
        rc = -1;
        goto done;
    }

    /* Check mandatory 'BootDevice' option */
    if (globals.bootDevice[0] == '\0')
    {
        LOGE(L"%s: Missing BootDevice option", Wcs(globals.lsvmconfPath));
        rc = -1;
        goto done;
    }

    /* Check mandatory 'RootDevice' option */
    if (globals.rootDevice[0] == '\0')
    {
        LOGE(L"%s: Missing RootDevice option", Wcs(globals.lsvmconfPath));
        rc = -1;
        goto done;
    }

done:

    if (data)
        Free(data);

    return rc;
}

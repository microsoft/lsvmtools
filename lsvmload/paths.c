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
#include <lsvmutils/eficommon.h>
#include <lsvmutils/strings.h>
#include "paths.h"
#include "console.h"
#include "logging.h"

static EFI_STATUS _GetProgramPath(
    IN EFI_HANDLE imageHandle,
    OUT CHAR16** path)
{
    EFI_STATUS status = EFI_SUCCESS;
    EFI_GUID guid = LOADED_IMAGE_PROTOCOL;
    EFI_LOADED_IMAGE* loadedImage;
    EFI_DEVICE_PATH* devicePath;

    /* Check input parameters */
    if (!path)
    {
        status = EFI_INVALID_PARAMETER;
        goto done;
    }

    *path = NULL;

    /* Initialize the output path */
    path[0] = '\0';

    /* Get the loaded image protocol interface */
    if ((status = uefi_call_wrapper(
        BS->HandleProtocol, 
        3, 
        imageHandle,
        &guid, 
        (void **)&loadedImage)) != EFI_SUCCESS) 
    {
        goto done;
    }

    /* Get the device path for this image */
    if (!(devicePath = loadedImage->FilePath))
    {
        status = EFI_INVALID_PARAMETER;
        goto done;
    }

    /* Get the path from the device path */
    if (!(*path = DevicePathToStr(loadedImage->FilePath)))
    {
        status = EFI_INVALID_PARAMETER;
        goto done;
    }

done:

    return status;
}

int Dirname(
    CHAR16* path)
{
    int rc = -1;
    CHAR16* slash = NULL;

    /* Check for null parameters */
    if (!path)
        goto done;

    /* Find the slash separating the base path from the program name */
    if (!(slash = Wcsrchr(path, '\\')) && !(slash = Wcsrchr(path, '/')))
        goto done;

    /* Check for absolute path */
    if (slash == path)
    {
        slash[1] = '\0';
    }
    else
    {
        /* Remove the program name component */
        slash[0] = '\0';
    }

    rc = 0;

done:

    return rc;
}

EFI_STATUS ResolvePaths(
    IN EFI_HANDLE imageHandle,
    Error* err)
{
    EFI_STATUS status = EFI_SUCCESS;
    CHAR16* dirname = NULL;

    /* Clear the error state */
    ClearErr(err);

    /* Get the full path of this program */
    if ((status = _GetProgramPath(
        imageHandle, 
        &globals.lsvmloadPath)) != EFI_SUCCESS)
    {
        SetErr(err, L"failed to get full program path");
        goto done;
    }

    /* Get the directory name from the program path */
    {
        if (!(dirname = StrDuplicate(globals.lsvmloadPath)))
        {
            status = EFI_OUT_OF_RESOURCES;
            SetErr(err, L"out of memory");
            goto done;
        }

        /* Get the directory name part of this path */
        if (Dirname(dirname) != 0)
        {
            SetErr(err, L"failed to obtain directory name of program path");
            status = EFI_UNSUPPORTED;
            goto done;
        }
    }

    /* lsvmconf */
    if (!(globals.lsvmconfPath = Wcsdup2(dirname, L"\\lsvmconf")))
    {
        status = EFI_OUT_OF_RESOURCES;
        SetErr(err, L"out of memory");
        goto done;
    }

    /* lsvmlog */
    if (!(globals.lsvmlogPath = Wcsdup2(dirname, L"\\lsvmlog")))
    {
        status = EFI_OUT_OF_RESOURCES;
        SetErr(err, L"out of memory");
        goto done;
    }

    /* sealed keys */
    if (!(globals.sealedKeysPath = Wcsdup2(dirname, L"\\sealedkeys")))
    {
        status = EFI_OUT_OF_RESOURCES;
        SetErr(err, L"out of memory");
        goto done;
    }

    /* specialize */
    if (!(globals.specializePath = Wcsdup2(dirname, L"\\specialization.aes")))
    {
        status = EFI_OUT_OF_RESOURCES;
        SetErr(err, L"out of memory");
        goto done;
    }

done:

    return status;
}

const CHAR16* UnixToDosPath(
    CHAR16 dosPath[PATH_SIZE], 
    const CHAR16* unixPath)
{
    CHAR16* p;

    Wcslcpy(dosPath, unixPath, PATH_SIZE);

    for (p = dosPath; *p; p++)
    {
        if (*p == '/')
            *p = '\\';
    }

    return dosPath;
}

const CHAR16* DosToUnixPath(
    CHAR16 unixPath[PATH_SIZE], 
    const CHAR16* dosPath)
{
    CHAR16* p;

    Wcslcpy(unixPath, dosPath, PATH_SIZE);

    for (p = unixPath; *p; p++)
    {
        if (*p == '\\')
            *p = '/';
    }

    return unixPath;
}

const CHAR16* Basename(
    const CHAR16* path)
{
    const CHAR16* p = path + StrLen(path);

    while (p != path && (p[-1] != '/' && p[-1] != '\\'))
        p--;

    return p;
}

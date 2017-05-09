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
#include "espwrap.h"
#include <lsvmutils/eficommon.h>
#include <lsvmutils/strings.h>
#include <lsvmutils/alloc.h>
#include <lsvmutils/ext2.h>
#include <lsvmutils/efibio.h>
#include <lsvmutils/strings.h>
#include "log.h"
#include "console.h"
#include "globals.h"
#include "bootfs.h"
#include "paths.h"
#include "logging.h"

static const CHAR16* _Basename(
    const CHAR16* path)
{
    const CHAR16* p = path + StrLen(path);

    while (p != path && (p[-1] != '/' && p[-1] != '\\'))
        p--;

    return p;
}

/*
**==============================================================================
**
** EFI_FILE
**
**==============================================================================
*/

static BOOLEAN _enableIOHooks;

static void _Suspend()
{
    _enableIOHooks = globals.enableIOHooks;
    globals.enableIOHooks = FALSE;
}

static void _Resume()
{
    globals.enableIOHooks = _enableIOHooks;
}

#define EFI_FILE_IMPL_MAGIC 0x37ca07b3

typedef struct _EFI_FILE_IMPL
{
    EFI_FILE base;
    UINT32 magic;
    struct _EFI_FILE_HANDLE* file;
    const UINT8* fileData;
    UINTN fileSize;
    UINTN fileOffset;
}
EFI_FILE_IMPL;

static EFI_STATUS EFIAPI _EFI_FILE_CloseHook(
    IN struct _EFI_FILE_HANDLE *file)
{
    return EFI_SUCCESS;
}

static EFI_STATUS EFIAPI _EFI_FILE_DeleteHook(
    IN struct _EFI_FILE_HANDLE *file)
{
    return EFI_UNSUPPORTED;
}

static EFI_STATUS EFIAPI _EFI_FILE_ReadHook(
    IN struct _EFI_FILE_HANDLE *file,
    IN OUT UINTN *bufferSize,
    OUT VOID *buffer)
{
    EFI_FILE_IMPL* impl = (EFI_FILE_IMPL*)file;
    UINTN rem;

    if (!impl->fileData)
        return EFI_UNSUPPORTED;

    rem = impl->fileSize - impl->fileOffset;

    if (rem == 0)
        return EFI_END_OF_FILE;

    if (rem < *bufferSize)
        *bufferSize = rem;

    Memcpy(buffer, impl->fileData + impl->fileOffset, *bufferSize);
    impl->fileOffset += *bufferSize;

    return EFI_SUCCESS;
}

static EFI_STATUS EFIAPI _EFI_FILE_WriteHook(
    IN struct _EFI_FILE_HANDLE *file,
    IN OUT UINTN *bufferSize,
    IN VOID *buffer)
{
    return EFI_UNSUPPORTED;
}

static EFI_STATUS EFIAPI _EFI_FILE_GetPositionHook(
    IN struct _EFI_FILE_HANDLE *file,
    OUT UINT64 *position)
{
    EFI_FILE_IMPL* impl = (EFI_FILE_IMPL*)file;

    if (!file || !position)
        return EFI_INVALID_PARAMETER;

    *position = impl->fileOffset;

    return EFI_SUCCESS;
}

static EFI_STATUS EFIAPI _EFI_FILE_SetPositionHook(
    IN struct _EFI_FILE_HANDLE  *file,
    IN UINT64 position)
{
    return EFI_UNSUPPORTED;
}

static EFI_STATUS EFIAPI _EFI_FILE_GetInfoHook(
    IN struct _EFI_FILE_HANDLE *file,
    IN EFI_GUID *informationType,
    IN OUT UINTN *bufferSize,
    OUT VOID *buffer)
{
    EFI_FILE_IMPL* impl = (EFI_FILE_IMPL*)file;
    EFI_FILE_INFO fileInfo;

    if (!impl->fileData)
        return EFI_UNSUPPORTED;

    Memset(&fileInfo, 0, sizeof(EFI_FILE_INFO));
    fileInfo.Size = impl->fileSize;
    fileInfo.FileSize = impl->fileSize;
    fileInfo.PhysicalSize = impl->fileSize;
    Memcpy(buffer, &fileInfo, sizeof(EFI_FILE_INFO));
    *bufferSize = sizeof(EFI_FILE_INFO);

    return EFI_SUCCESS;
}

static EFI_STATUS EFIAPI _EFI_FILE_FlushHook(
    IN struct _EFI_FILE_HANDLE *file)
{
    return EFI_SUCCESS;
}

static EFI_STATUS EFIAPI _EFI_FILE_SetInfoHook(
    IN struct _EFI_FILE_HANDLE *file,
    IN EFI_GUID *informationType,
    IN UINTN bufferSize,
    IN VOID *buffer)
{
    return EFI_UNSUPPORTED;
}

static EFI_STATUS EFIAPI _EFI_FILE_OpenHook(
    IN struct _EFI_FILE_HANDLE  *file,
    OUT struct _EFI_FILE_HANDLE **newHandle,
    IN CHAR16 *fileName,
    IN UINT64 openMode,
    IN UINT64 attributes);

static EFI_FILE _efi_file =
{
    0x0000000000010000, /* Revision */
    _EFI_FILE_OpenHook,
    _EFI_FILE_CloseHook,
    _EFI_FILE_DeleteHook,
    _EFI_FILE_ReadHook,
    _EFI_FILE_WriteHook,
    _EFI_FILE_GetPositionHook,
    _EFI_FILE_SetPositionHook,
    _EFI_FILE_GetInfoHook,
    _EFI_FILE_SetInfoHook,
    _EFI_FILE_FlushHook
};

static EFI_STATUS EFIAPI _EFI_FILE_OpenHook(
    IN struct _EFI_FILE_HANDLE  *file,
    OUT struct _EFI_FILE_HANDLE **newHandle,
    IN CHAR16 *fileName,
    IN UINT64 openMode,
    IN UINT64 attributes)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    EFI_FILE_IMPL* impl = NULL;

    _Suspend();

    /* Check parameters */
    if (!file || !newHandle || !fileName)
    {
        status = EFI_INVALID_PARAMETER;
        goto done;
    }

#if 0
    LOGI(L"Open{%s}", fileName);
#endif

    /* Allocate the wrapper EFI_FILE object */
    if (!(impl = (EFI_FILE_IMPL*)Calloc(1, sizeof(EFI_FILE_IMPL))))
    {
        status = EFI_OUT_OF_RESOURCES;
        goto done;
    }

    /* Set all the function pointers */
    Memcpy(impl, &_efi_file, sizeof(EFI_FILE));
    impl->magic = EFI_FILE_IMPL_MAGIC;
    impl->file = *newHandle;

    /* If this is grubx64.efi, then use preloaded memory */
    if (StriCmp(_Basename(fileName), L"grubx64.efi")  == 0 || 
        StriCmp(_Basename(fileName), L"grub.efi") == 0)
    {
        if (!globals.grubData || globals.grubSize == 0)
            goto done;

        impl->fileData = globals.grubData;
        impl->fileSize = globals.grubSize;
        impl->fileOffset = 0;
    }
    else
    {
        status = EFI_NOT_FOUND;
        goto done;
    }

    *newHandle = &impl->base;

    status = EFI_SUCCESS;

done:

    if (status != EFI_SUCCESS)
    {
        if (impl)
            Free(impl);
    }

    _Resume();
    return status;
}

/*
**==============================================================================
**
** EFI_FILE_IO_INTERFACE
**
**==============================================================================
*/

static EFI_VOLUME_OPEN _EFI_FILE_IO_INTERFACE_OpenVolume = NULL;

static EFI_STATUS EFIAPI _EFI_FILE_IO_INTERFACE_OpenVolumeHook(
    IN struct _EFI_FILE_IO_INTERFACE *this,
    OUT struct _EFI_FILE_HANDLE **root)
{
    EFI_STATUS status;

    /* Use original OpenVolume method to open this volume */
    status = _EFI_FILE_IO_INTERFACE_OpenVolume(this, root);

    /* Override the open method */
    if (root && *root && status == EFI_SUCCESS)
    {
        /* Install new method */
        (*root)->Open = _EFI_FILE_OpenHook;
    }

    return status;
}

EFI_STATUS WrapESPFileIO(
    EFI_HANDLE imageHandle)
{
    EFI_STATUS status = EFI_SUCCESS;
    EFI_LOADED_IMAGE* loadedImage = NULL;

    /* Obtain the LOADED_IMAGE_PROTOCOL */
    {
        EFI_GUID guid = LOADED_IMAGE_PROTOCOL;

        if ((status = uefi_call_wrapper(
            BS->HandleProtocol,
            3,
            imageHandle,
            &guid,
            (void**)&loadedImage)) != EFI_SUCCESS)
        {
            status = EFI_UNSUPPORTED;
            goto done;
        }
    }

    /* Install SIMPLE_FILE_SYSTEM_PROTOCOL hook */
    {
        EFI_GUID guid = SIMPLE_FILE_SYSTEM_PROTOCOL;
        EFI_FILE_IO_INTERFACE* fileIOInterface = NULL;

        /* Load old EFI_FILE_IO_INTERFACE */
        if ((status = uefi_call_wrapper(
            BS->HandleProtocol,
            3,
            loadedImage->DeviceHandle,
            &guid,
            (void**)&fileIOInterface)) != EFI_SUCCESS)
        {
            status = EFI_UNSUPPORTED;
            goto done;
        }

        /* Save the old function */
        _EFI_FILE_IO_INTERFACE_OpenVolume = fileIOInterface->OpenVolume;

        /* Install the hook */
        fileIOInterface->OpenVolume = _EFI_FILE_IO_INTERFACE_OpenVolumeHook;
    }

done:

    return status;
}

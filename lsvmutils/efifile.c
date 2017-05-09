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
#include "efifile.h"
#include "error.h"

#define FILE_HANDLE_MAGIC 0X84455D41

struct _EFIFile
{
    UINT32 magic;
    EFI_FILE* volume;
    EFI_FILE* file;
};

static BOOLEAN _ValidFile(
    IN EFIFile* efiFile)
{
    if (efiFile &&
        efiFile->magic == FILE_HANDLE_MAGIC &&
        efiFile->file &&
        efiFile->volume)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

EFI_STATUS FileExists(
    IN EFI_HANDLE imageHandle,
    IN const CHAR16* path)
{
    EFI_STATUS status = EFI_SUCCESS;
    EFIFile* efiFile = NULL;

    if (!(efiFile = OpenFile(imageHandle, path, EFI_FILE_MODE_READ, FALSE)))
    {
        status = EFI_NOT_FOUND;
        goto done;
    }

done:

    if (efiFile)
        CloseFile(efiFile);

    return status;
}

static EFI_STATUS _GetFileSizeFromHandle(
    IN EFI_FILE* file,
    OUT UINTN* size)
{
    EFI_STATUS status = EFI_SUCCESS;
    EFI_GUID fileInfoId = EFI_FILE_INFO_ID;
    EFI_FILE_INFO fileInfobuffer;
    EFI_FILE_INFO* fileInfo = &fileInfobuffer;
    UINTN fileInfoSize = sizeof(fileInfobuffer);

    /* Check the parameters */
    if (!file || !size)
    {
        status = EFI_INVALID_PARAMETER;
        goto done;
    }

    /* Get size of file */
    {
        /* Try with buffer == sizeof(EFI_FILE_INFO) */
        status = uefi_call_wrapper(
            file->GetInfo, 
            4, 
            file, 
            &fileInfoId,
            &fileInfoSize, 
            fileInfo);

        /* Buffer too small so reallocate and try again */
        if (status == EFI_BUFFER_TOO_SMALL) 
        {
            fileInfo = AllocatePool(fileInfoSize);

            if (!fileInfo) 
            {
                status = EFI_OUT_OF_RESOURCES;
                goto done;
            }

            status = uefi_call_wrapper(
                file->GetInfo,
                4, 
                file, 
                &fileInfoId,
                &fileInfoSize, 
                fileInfo);
        }

        if (status != EFI_SUCCESS)
            goto done;

        *size = fileInfo->FileSize;
    }

done:

    if (fileInfo != &fileInfobuffer)
        FreePool(fileInfo);

    return status;
}

#if defined(NEED_GetFileSize)
EFI_STATUS GetFileSize(
    IN EFI_HANDLE imageHandle,
    IN const CHAR16* path,
    OUT UINTN* size)
{
    EFI_STATUS status = EFI_SUCCESS;
    EFIFile* efiFile = NULL;

    /* Check arguments */
    if (!path || !size)
    {
        status = EFI_INVALID_PARAMETER;
        goto done;
    }

    /* Open the file */
    if (!(efiFile = OpenFile(imageHandle, path, EFI_FILE_MODE_READ, FALSE)))
    {
        status = EFI_NOT_FOUND;
        goto done;
    }

    /* Get the file size from the open handle */
    if ((status = _GetFileSizeFromHandle(efiFile->file, size)))
    {
        goto done;
    }

done:

    if (efiFile)
        CloseFile(efiFile);

    return status;
}
#endif /* defined(NEED_GetFileSize) */

EFIFile* OpenFile(
    EFI_HANDLE imageHandle,
    const CHAR16* path,
    IN UINT64 fileMode,
    IN BOOLEAN append)
{
    EFI_STATUS status = EFI_SUCCESS;
    EFI_GUID loadedImageProtocol = LOADED_IMAGE_PROTOCOL;
    EFI_LOADED_IMAGE* loadedImage = NULL;
    EFI_GUID simple_file_system_protocol = SIMPLE_FILE_SYSTEM_PROTOCOL;
    EFI_FILE_IO_INTERFACE* file_io_interface = NULL;
    EFI_FILE* volume = NULL;
    EFI_FILE* file = NULL;
    EFIFile* efiFile = NULL;

    /* Check parameters */
    if (!path)
    {
        status = EFI_INVALID_PARAMETER;
        goto done;
    }

    /* Obtain the LOADED_IMAGE_PROTOCOL */
    if ((status = uefi_call_wrapper(
        BS->HandleProtocol, 
        3, 
        imageHandle,
        &loadedImageProtocol, 
        (void**)&loadedImage)) != EFI_SUCCESS)
    {
        goto done;
    }

    /* Obtain the SIMPLE_FILE_SYSTEM_PROTOCOL */
    if ((status = uefi_call_wrapper(
        BS->HandleProtocol, 
        3, 
        loadedImage->DeviceHandle,
        &simple_file_system_protocol,
        (void**)&file_io_interface)) != EFI_SUCCESS)
    {
        goto done;
    }

    /* Open the volume */
    if ((status = uefi_call_wrapper(
        file_io_interface->OpenVolume, 
        2, 
        file_io_interface, 
        &volume)) != EFI_SUCCESS)
    {
        goto done;
    }

    /* Open the file */
    if ((status = uefi_call_wrapper(
        volume->Open, 
        5, 
        volume, 
        &file, 
        (CHAR16*)path,
        fileMode,
        0)) != EFI_SUCCESS)
    {
        goto done;
    }

    /* Open file for append */
    if (append)
    {
        UINTN fileSize;

        /* Get size of file from file handle */
        if ((status = _GetFileSizeFromHandle(
            file, 
            &fileSize)) != EFI_SUCCESS)
        {
            goto done;
        }

        /* Seek the end of the file (so data will be appended) */
        if ((status = uefi_call_wrapper(
            file->SetPosition, 
            2, 
            file, 
            fileSize)) != EFI_SUCCESS)
        {
            goto done;
        }
    }

    /* Create and initialize EFIFile */
    {
        if (!(efiFile = AllocatePool(sizeof(EFIFile))))
        {
            status = EFI_OUT_OF_RESOURCES;
            goto done;
        }

        efiFile->magic = FILE_HANDLE_MAGIC;
        efiFile->volume = volume;
        efiFile->file = file;
    }

done:

    if (status != EFI_SUCCESS)
    {
        if (file)
            uefi_call_wrapper(file->Close, 1, file);

        if (volume)
            uefi_call_wrapper(volume->Close, 1, volume);
    }

    return efiFile;
}

EFI_STATUS ReadFile(
    IN EFIFile* efiFile,
    IN void* data,
    IN UINTN size,
    IN UINTN* sizeRead)
{
    EFI_STATUS status = EFI_SUCCESS;

    /* Check parameters */
    if (!_ValidFile(efiFile) || !data || !sizeRead)
    {
        status = EFI_INVALID_PARAMETER;
        goto done;
    }

    /* Read the data */
    if ((status = uefi_call_wrapper(
        efiFile->file->Read, 
        3, 
        efiFile->file, 
        &size,
        data)) != EFI_SUCCESS)
    {
        goto done;
    }

    *sizeRead = size;

done:

    return status;
}

EFI_STATUS ReadFileN(
    EFIFile* efiFile,
    void* data,
    UINTN size)
{
    EFI_STATUS status = EFI_SUCCESS;
    UINTN totalRead = 0;

    /* Check parameters */
    if (!_ValidFile(efiFile) || !data)
    {
        status = EFI_INVALID_PARAMETER;
        goto done;
    }

    /* Read the from the file */
    {
        unsigned char* p = (unsigned char*)data;
        unsigned char* end = p + size;

        while (p != end)
        {
            UINTN n = end - p;
            UINTN sizeRead = 0;

            if ((status = ReadFile(
                efiFile, 
                p, 
                n,
                &sizeRead)) != EFI_SUCCESS)
            {
                goto done;
            }

            p += sizeRead;
            totalRead += sizeRead;
        }
    }

    if (totalRead != size)
    {
        status = EFI_DEVICE_ERROR;
        goto done;
    }

done:

    return status;
}

EFI_STATUS WriteFile(
    IN EFIFile* efiFile,
    IN const void* data,
    IN UINTN size,
    IN UINTN* sizeWritten)
{
    EFI_STATUS status = EFI_SUCCESS;

    /* Check parameters */
    if (!_ValidFile(efiFile) || !data || !sizeWritten)
    {
        status = EFI_INVALID_PARAMETER;
        goto done;
    }

    /* Read the data */
    if ((status = uefi_call_wrapper(
        efiFile->file->Write, 
        3, 
        efiFile->file, 
        &size,
        (void*)data)) != EFI_SUCCESS)
    {
        goto done;
    }

    *sizeWritten = size;

done:

    return status;
}

EFI_STATUS WriteFileN(
    EFIFile* efiFile,
    const void* data,
    UINTN size)
{
    EFI_STATUS status = EFI_SUCCESS;
    UINTN totalWritten = 0;

    /* Check parameters */
    if (!_ValidFile(efiFile) || !data)
    {
        status = EFI_INVALID_PARAMETER;
        goto done;
    }

    /* Write the data to the file */
    {
        const unsigned char* p = (unsigned char*)data;
        const unsigned char* end = p + size;

        while (p != end)
        {
            UINTN n = end - p;
            UINTN sizeWritten = 0;

            if ((status = WriteFile(
                efiFile, 
                p, 
                n,
                &sizeWritten)) != EFI_SUCCESS)
            {
                goto done;
            }

            p += sizeWritten;
            totalWritten += sizeWritten;
        }
    }

    if (size != totalWritten)
    {
        status = EFI_DEVICE_ERROR;
        goto done;
    }

done:

    return status;
}

EFI_STATUS FlushFile(
    EFIFile* efiFile)
{
    EFI_STATUS status;

    if (!_ValidFile(efiFile))
    {
        status = EFI_INVALID_PARAMETER;
        goto done;
    }

    if ((status = uefi_call_wrapper(
        efiFile->file->Flush, 
        1, 
        efiFile->file)) != EFI_SUCCESS)
    {
        goto done;
    }

done:

    return status;
}

EFI_STATUS DeleteFile(
    EFI_HANDLE imageHandle,
    const CHAR16* path)
{
    EFI_STATUS status = EFI_SUCCESS;
    EFIFile* efiFile = NULL;

    /* Open file */
    if (!(efiFile = OpenFile(
        imageHandle, 
        path, 
        EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ | EFI_FILE_MODE_CREATE,
        FALSE)))
    {
        status = EFI_NOT_FOUND;
        goto done;
    }

    /* Delete the file */
    if ((status = uefi_call_wrapper(
        efiFile->file->Delete, 
        1, 
        efiFile->file)) != EFI_SUCCESS)
    {
        goto done;
    }

    /* Note that deleting a file closes it, so null out file pointer */
    efiFile->file = NULL;

done:

    if (efiFile)
        CloseFile(efiFile);

    return status;
}

EFI_STATUS CloseFile(
    EFIFile* efiFile)
{
    EFI_STATUS status;

    if (!_ValidFile(efiFile))
    {
        status = EFI_INVALID_PARAMETER;
        goto done;
    }

    if (efiFile->file)
        uefi_call_wrapper(efiFile->file->Close, 1, efiFile->file);

    if (efiFile->volume)
        uefi_call_wrapper(efiFile->volume->Close, 1, efiFile->volume);

    FreePool(efiFile);

done:

    return status;
}

EFI_STATUS SeekFile(
    EFIFile* efiFile,
    UINT64 position)
{
    EFI_STATUS status;

    if (!_ValidFile(efiFile))
    {
        status = EFI_INVALID_PARAMETER;
        goto done;
    }

    if ((status = uefi_call_wrapper(
        efiFile->file->SetPosition, 
        2, 
        efiFile->file, 
        position)) != EFI_SUCCESS)
    {
        goto done;
    }

done:

    return status;
}

EFI_STATUS EFILoadFile(
    IN EFI_HANDLE imageHandle,
    IN const CHAR16* path,
    OUT void** data,
    OUT UINTN* size)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    EFIFile* efiFile = NULL;

    /* Check parameters */
    if (!path || !data || !size)
    {
        status = EFI_INVALID_PARAMETER;
        goto done;
    }

    /* Initialize output variables */
    *data = NULL;
    *size = 0;

    /* Open the file for read */
    if (!(efiFile = OpenFile(imageHandle, path, EFI_FILE_MODE_READ, FALSE)))
    {
        status = EFI_NOT_FOUND;
        goto done;
    }

    /* Get the size of the file */
    if ((status = _GetFileSizeFromHandle(
        efiFile->file, 
        size)) != EFI_SUCCESS)
    {
        goto done;
    }

    /* Allocate a buffer to hold file in memory */
    if (!(*data = AllocatePool(*size)))
    {
        status = EFI_OUT_OF_RESOURCES;
        goto done;
    }

    /* Read the entire file */
    if ((status = ReadFileN(efiFile, *data, *size)) != EFI_SUCCESS)
    {
        goto done;
    }

    status = EFI_SUCCESS;

done:

    if (status != EFI_SUCCESS)
    {
        if (*data)
            FreePool(*data);

        *data = NULL;
        *size = 0;
    }

    if (efiFile)
        CloseFile(efiFile);

    return status;
}

EFI_STATUS EFIPutFile(
    IN EFI_HANDLE imageHandle,
    IN const CHAR16* path,
    IN const void* data,
    IN UINTN size,
    Error* err)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    EFIFile* efiFile = NULL;

    ClearErr(err);

    /* Check parameters */
    if (!path || !data)
    {
        SetErr(err, TCS("EFIPutFile(): bad parameter"));
        status = EFI_INVALID_PARAMETER;
        goto done;
    }

    /* Delete file if it already exists */
    DeleteFile(imageHandle, path);

    /* Open the file for read */
    if (!(efiFile = OpenFile(
        imageHandle, 
        path, 
        EFI_FILE_MODE_READ|EFI_FILE_MODE_WRITE|EFI_FILE_MODE_CREATE,
        TRUE)))
    {
        SetErr(err, TCS("EFIPutFile(): OpenFile() failed"));
        status = EFI_NOT_FOUND;
        goto done;
    }

    /* Read the entire file */
    if ((status = WriteFileN(efiFile, data, size)) != EFI_SUCCESS)
    {
        SetErr(err, TCS("EFIPutFile(): WriteFileN() failed"));
        goto done;
    }

    status = EFI_SUCCESS;

done:

    if (efiFile)
        CloseFile(efiFile);

    return status;
}

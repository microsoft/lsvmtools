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
#include "efibio.h"
#include <lsvmutils/alloc.h>
#include <lsvmutils/strings.h>

/*
**==============================================================================
**
** Private definitions:
**
**==============================================================================
*/

static EFI_GUID _guid = BLOCK_IO_PROTOCOL;

EFI_STATUS LocateBlockIOHandles(
    EFI_HANDLE** handles,
    UINTN* numHandles)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    EFI_HANDLE* buffer = NULL;
    UINTN bufferSize = 0;

    /* Check parameters */
    if (!handles || !numHandles)
    {
        status = EFI_INVALID_PARAMETER;
        goto done;
    }

    /* Allocate space for 8 handles */
    {
        bufferSize = 8 * sizeof(EFI_HANDLE);

        if (!(buffer = (EFI_HANDLE*)Malloc(bufferSize)))
        {
            status = EFI_OUT_OF_RESOURCES;
            goto done;
        }
    }

    /* Locate the handle for the BLOCK_IO_PROTOCOL */
    if ((status = uefi_call_wrapper(
        BS->LocateHandle, 
        5,
        ByProtocol, /* SearchType */
        &_guid, /* Protocol */
        0, /* SearchKey (none) */
        &bufferSize,
        buffer)) != EFI_SUCCESS)
    {
        goto done;
    }

    /* If buffer is too small */
    if (status == EFI_BUFFER_TOO_SMALL)
    {
        Free(buffer);

        /* Allocate buffer */
        if (!(buffer = Malloc(bufferSize)))
        {
            status = EFI_OUT_OF_RESOURCES;
            goto done;
        }

        /* Locate the handle for the BLOCK_IO_PROTOCOL */
        if ((status = uefi_call_wrapper(
            BS->LocateHandle, 
            5,
            ByProtocol, /* SearchType */
            &_guid, /* Protocol */
            0, /* SearchKey (none) */
            &bufferSize,
            buffer)) != EFI_SUCCESS)
        {
            goto done;
        }
    }

done:

    if (status == EFI_SUCCESS)
    {
        *handles = buffer;
        *numHandles = bufferSize / sizeof(EFI_HANDLE);
    }
    else
    {
        Free(buffer);
        *handles = NULL;
        *numHandles = 0;
    }
    
    return status;
}

EFI_STATUS OpenBlockIOProtocol(
    EFI_HANDLE imageHandle,
    EFI_HANDLE handle,
    EFI_BLOCK_IO** blockIO)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    void* interface = NULL;

    *blockIO = NULL;

    if ((status = uefi_call_wrapper(
        BS->OpenProtocol,
        6,
        handle, /* Handle */
        &_guid, /* Protocol */
        &interface, /* Interface */
        imageHandle, /* AgentHandle */
        0, /* ControllerHandle */
        EFI_OPEN_PROTOCOL_GET_PROTOCOL)) != EFI_SUCCESS) /* Attributes */
    {
        goto done;
    }

    *blockIO = (EFI_BLOCK_IO*)interface;

done:
    return status;
}

static EFI_STATUS _CloseBlockIOProtocol(
    EFI_HANDLE imageHandle,
    EFI_HANDLE handle)
{
    EFI_STATUS status = EFI_UNSUPPORTED;

    if ((status = uefi_call_wrapper(
        BS->CloseProtocol,
        4,
        handle, /* Handle */
        &_guid, /* Protocol */
        imageHandle, /* AgentHandle */
        0)) != EFI_SUCCESS) /* ControllerHandle */
    {
        goto done;
    }

done:
    return status;
}

/*
**==============================================================================
**
** Public definitions:
**
**==============================================================================
*/

EFI_BIO* OpenBIO(
    EFI_HANDLE imageHandle,
    MatchBIO match,
    void* matchData)
{
    EFI_BIO* bio = NULL;
    EFI_HANDLE* handles = NULL;
    UINTN numHandles = 0;
    UINTN i;

    if (imageHandle == NULL || !match)
        goto done;

    /* Locate all the block IO handles */
    if (LocateBlockIOHandles(&handles, &numHandles) != EFI_SUCCESS)
        goto done;

    /* Find a BIO that passes the match function */
    for (i = 0; i < numHandles; i++)
    {
        EFI_BLOCK_IO* blockIO = NULL;

        if (OpenBlockIOProtocol(
            imageHandle, 
            handles[i], 
            &blockIO) != EFI_SUCCESS)
        {
            continue;
        }

        /* Initialize the new BIO */
        {
            if (!(bio = (EFI_BIO*)Calloc(1, sizeof(EFI_BIO))))
            {
                goto done;
            }

            bio->magic = BIO_MAGIC;
            bio->imageHandle = imageHandle;
            bio->handle = handles[i];
            bio->blockIO = blockIO;
        }

        /* See if it matches */
        if ((*match)(bio, matchData))
        {
            /* Found one! */
            break;
        }

        /* No match, so close the BIO */
        CloseBIO(bio);
        bio = NULL;
    }

done:

    if (handles)
        Free(handles);

    return bio;
}

BOOLEAN ValidBIO(
    const EFI_BIO* bio)
{
    if (bio == NULL || 
        bio->magic != BIO_MAGIC || 
        bio->imageHandle == NULL || 
        bio->handle == NULL || 
        bio->blockIO == NULL)
    {
        return FALSE;
    }

    return TRUE;
}

EFI_STATUS ReadBIO(
    EFI_BIO* bio, 
    UINTN blkno,
    void* data,
    UINTN size)
{
    EFI_STATUS status = EFI_UNSUPPORTED;

    if (!ValidBIO(bio) || !data)
        goto done;

    /* Read the blocks */
    if ((status = uefi_call_wrapper(
        bio->blockIO->ReadBlocks, 
        5, 
        bio->blockIO,
        bio->blockIO->Media->MediaId,
        blkno,
        size,
        data)) != EFI_SUCCESS)
    {
        goto done;
    }

    status = EFI_SUCCESS;

done:

    return status;
}

EFI_STATUS WriteBIO(
    EFI_BIO* bio, 
    UINTN blkno,
    const void* data,
    UINTN size)
{
    EFI_STATUS status = EFI_UNSUPPORTED;

    if (!ValidBIO(bio) || !data)
        goto done;

    /* Write the blocks */
    if ((status = uefi_call_wrapper(
        bio->blockIO->WriteBlocks, 
        5, 
        bio->blockIO,
        bio->blockIO->Media->MediaId,
        blkno,
        size,
        (void*)data)) != EFI_SUCCESS)
    {
        goto done;
    }

    status = EFI_SUCCESS;

done:

    return status;
}

EFI_STATUS CloseBIO(EFI_BIO* bio)
{
    EFI_STATUS status = EFI_UNSUPPORTED;

    if (!ValidBIO(bio))
        goto done;

    if ((status = _CloseBlockIOProtocol(
        bio->imageHandle,
        bio->handle)) != EFI_SUCCESS)
    {
        goto done;
    }

    Memset(bio, 0xDD, sizeof(EFI_BIO));
    Free(bio);

    status = EFI_SUCCESS;

done:
    return status;
}

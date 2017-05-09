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
#include "shim.h"
#include <lsvmutils/alloc.h>
#include <lsvmutils/peimage.h>
#include <lsvmutils/efifile.h>
#include <lsvmutils/strings.h>
#include "measure.h"
#include "globals.h"
#include "image.h"
#include "espwrap.h"
#include "diskbio.h"
#include "trace.h"
#include "devpath.h"
#include "bootbio.h"
#include "bootfs.h"
#include "efivfat.h"
#include "paths.h"
#include "logging.h"
#include "progress.h"

extern EFI_STATUS CreateDummyFS(
    EFI_HANDLE imageHandle);

static EFI_STATUS _LoadGRUB(
    EFI_HANDLE imageHandle,
    EFI_TCG2_PROTOCOL* tcg2Protocol,
    EXT2* bootfs)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    void* data = NULL;
    UINTN size;

    /* Check parameters */
    if (!tcg2Protocol || !bootfs)
        goto done;

    /* Load the image */
    if (LoadFileFromBootFS(
        imageHandle, 
        tcg2Protocol,
        bootfs,
        GRUB_PATH,
        &data, 
        &size) != EFI_SUCCESS)
    {
        goto done;
    }

    /* Dump the entire event log */
    if (GetLogLevel() >= DEBUG)
    {
        LogEventLog(tcg2Protocol, imageHandle);
    }

    /* Set the global variables */
    globals.grubData = data;
    globals.grubSize = size;

    status = EFI_SUCCESS;

done:

    if (status != EFI_SUCCESS)
    {
        if (data)
            Free(data);
    }

    return status;
}

EFI_STATUS StartShim(
    EFI_HANDLE imageHandle,
    EFI_SYSTEM_TABLE *systemTable,
    EFI_TCG2_PROTOCOL* tcg2Protocol,
    EXT2* bootfs)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    void* data = NULL;
    UINTN size;

    PutProgress(L"Loading %s", Wcs(SHIM_PATH));

    /* Load the bootloader image */
    if (LoadFileFromBootFS(
        imageHandle, 
        tcg2Protocol,
        bootfs,
        SHIM_PATH,
        &data, 
        &size) != EFI_SUCCESS)
    {
        LOGE(L"failed to load image: %s", Wcs(SHIM_PATH));
        goto done;
    }
    else
    {
        LOGI(L"Loaded image: %s", Wcs(SHIM_PATH));
    }

#if defined(CHECK_SHIM_CERTIFICATE)

    /* Check file against internal certificate */
    {
        extern unsigned char g_cert[];
        extern unsigned int g_cert_size;

        if (CheckCert(data, size, g_cert, g_cert_size) != 0)
        {
            LOGE(L"Cert check failed: %s", Wcs(SHIM_PATH));
            goto done;
        }

        LOGI(L"Cert check okay: %s", Wcs(SHIM_PATH));
    }

#endif /* defined(CHECK_SHIM_CERTIFICATE) */

    PutProgress(L"Loading %s", Wcs(GRUB_PATH));

    if (_LoadGRUB(imageHandle, tcg2Protocol, bootfs) != EFI_SUCCESS)
    {
        LOGE(L"failed to load image: %s", Wcs(GRUB_PATH));
        goto done;
    }
    else
    {
        LOGI(L"Loaded image: %s", Wcs(GRUB_PATH));
    }

    /* Install hooks to capture I/O from EFI child processess */
    {
        if (WrapESPFileIO(imageHandle) != EFI_SUCCESS)
        {
            LOGE(L"WrapESPFileIO() failed");
            goto done;
        }

        if (InstallRootBIO(imageHandle) != EFI_SUCCESS)
        {
            LOGE(L"InstallRootBIO() failed");
        }

#if 0
        if (CreateDummyFS(imageHandle) != EFI_SUCCESS)
        {
            LOGW(L"CreateDummyFS() failed");
        }
        else
        {
            LOGI(L"CreateDummyFS() ok");
        }
#endif

        if (WrapBootBIO(
            imageHandle, 
            tcg2Protocol,
            globals.bootkeyData, 
            globals.bootkeySize) != EFI_SUCCESS)
        {
            LOGE(L"StartShim(): WrapBootBIO() failed");
        }

#if 0
        /* Dump all the device paths */
        DevPathDumpAll();
#endif

        if (MapEFIVFAT(imageHandle) != 0)
        {
            LOGE(L"Failed to map ESP");
        }

#if 0
        if (InstallEFIBIO(imageHandle) != EFI_SUCCESS)
        {
            LOGE(L"InstallEFIBIO() failed");
        }
        else
        {
            LOGI(L"InstallEFIBIO() ok");
        }
#endif

        /* Enable I/O hooks */
        globals.enableIOHooks = TRUE;

        PutProgress(L"Executing %s", Wcs(SHIM_PATH));

        if (Exec(imageHandle, systemTable, data, size) != EFI_SUCCESS)
        {
            /* Disable I/O hooks so that we can log */
            globals.enableIOHooks = FALSE;

            LOGE(L"failed to execute shim");
            goto done;
        }
    }

    status = EFI_SUCCESS;

done:

    if (data)
        Free(data);

    return status;
}

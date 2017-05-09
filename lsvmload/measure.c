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
#include "measure.h"
#include <lsvmutils/eficommon.h>
#include <lsvmutils/strarr.h>
#include <lsvmutils/alloc.h>
#include <lsvmutils/tcg2.h>
#include <lsvmutils/peimage.h>
#include <lsvmutils/ext2.h>
#include <lsvmutils/tpm2.h>
#include <lsvmutils/sha.h>
#include <lsvmutils/efifile.h>
#include <lsvmutils/strings.h>
#include "log.h"
#include "paths.h"
#include "console.h"
#include "simpletext.h"
#include "faults.h"
#include "logging.h"
#include "globals.h"
#include "bootfs.h"

#if defined(HAVE_SSL)
# include <openssl/sha.h> 
#endif

extern void FailMeasuredBoot(EFI_TCG2_PROTOCOL* tcg2Protocol);

/*
**==============================================================================
**
** Local definitions
**
**==============================================================================
*/

#define PERFORM_PCR11_MEASUREMENTS

#if defined(NEED__DumpFile)
static int _DumpFile(
    const FileSys* fileSys,
    const char* path)
{
    int rc = 0;
    File* file;
    INTN n;
    char buf[1024];
    
    /* Open file for read */
    if (!(file = fileSys->Open(fileSys, path)))
    {
        rc = -1;
        goto done;
    }

    Print(L"<<<<<<<<<<\n");

    /* Read next block */
    while ((n = fileSys->Read(fileSys, file, buf, sizeof(buf)-1)) > 0)
    {
        INTN i;

        /* Translate non-printable characters to hex strings */
        for (i = 0; i < n; i++)
        {
            char c = buf[i];

            /* If printable */
            if ((c >= ' ' && c <= '~')) //  && c != '\n')
            {
                Print(L"%c", c);
            }
            else
            {
                CHAR16 hex[9];
                /* Note: gnu-efi does not support %02X */
                SPrint(hex, sizeof(hex), L"%08X", c);
                Print(L"<%s>", hex+6);
            }
        }
    }

    Print(L"\n>>>>>>>>>>\n");
    Wait();

done:

    /* Close the file */
    if (file)
        fileSys->Close(fileSys, file);

    return rc;
}
#endif /* defined(NEED__DumpFile) */

EFI_STATUS MeasureHash(
    EFI_TCG2_PROTOCOL *tcg2Protocol,
    EFI_HANDLE imageHandle,
    UINT32 pcrIndex,
    const CHAR16* name,
    const SHA1Hash* sha1,
    const SHA256Hash* sha256,
    OUT Error* err)
{
    EFI_STATUS status = EFI_SUCCESS;

    ClearErr(err);

    /* Mesure into PCR register */
    {
        if (TPM2X_ExtendPCR_SHA1(
            tcg2Protocol, 
            pcrIndex, 
            sha1) != TPM_RC_SUCCESS)
        {
            SetErr(err, L"TPM2X_ExtendPCR_SHA1() failed");
            status = EFI_UNSUPPORTED;
            goto done;
        }

        if (TPM2X_ExtendPCR_SHA256(
            tcg2Protocol, 
            pcrIndex, 
            sha256) != TPM_RC_SUCCESS)
        {
            SetErr(err, L"TPM2X_ExtendPCR_SHA256() failed");
            status = EFI_UNSUPPORTED;
            goto done;
        }
    }

    /* Log the hashes */
    LogSHA1(name, sha1);
    LogSHA256(name, sha256);

done:

    return status;
}

static EFI_STATUS _CheckFile(EFI_HANDLE imageHandle, const CHAR16* path)
{
    if (FileExists(imageHandle, path) != EFI_SUCCESS)
    {
        Print(L"File not found: %s\n", Wcs(path));
        return EFI_NOT_FOUND;
    }

    return EFI_SUCCESS;
}

static EFI_STATUS _CheckFileDependencies(EFI_HANDLE imageHandle)
{
    EFI_STATUS status = EFI_SUCCESS;

    if (_CheckFile(imageHandle, globals.lsvmloadPath) != EFI_SUCCESS)
        status = EFI_NOT_FOUND;

    if (_CheckFile(imageHandle, globals.lsvmconfPath) != EFI_SUCCESS)
        status = EFI_NOT_FOUND;

    return status;
}

/*
**==============================================================================
**
** Public definitions:
**
**==============================================================================
*/

static int _InitMeasuredBoot(
    EFI_TCG2_PROTOCOL* tcg2Protocol,
    EFI_HANDLE imageHandle,
    Error* err)
{
    int status = 0;

    ClearErr(err);

    if (TPM2X_SetDictionaryAttackLockReset(tcg2Protocol) != TPM_RC_SUCCESS)
    {
        SetErr(err, L"failed to reset dictionary attack");
        status = -1;
        goto done;
    }

    if (TPM2X_SetLockoutParams(tcg2Protocol) != TPM_RC_SUCCESS)
    {
        SetErr(err, L"failed to set lockout parameters");
        status = -1;
        goto done;
    }

done:

    return status;
}

EFI_STATUS Initialize(
    EFI_TCG2_PROTOCOL* tcg2Protocol,
    EFI_HANDLE imageHandle,
    Error* err)
{
    EFI_STATUS status = EFI_SUCCESS;

    /* Check for null parmeters */
    if (!tcg2Protocol || !err)
    {
        SetErr(err, L"null parameter");
        status = EFI_INVALID_PARAMETER;
        goto done;
    }

    /* Clear the error object */
    ClearErr(err);

    /* Check to see if expected files are in place */
    if (_CheckFileDependencies(imageHandle) != EFI_SUCCESS ||
        CheckFault("FILE_DEPENDENCY_CHECKS_FAILED"))
    {
        status = EFI_LOAD_ERROR;
        SetErr(err, L"file dependency checks failed");
        goto done;
    }

    /* Initialize measured boot */
    if (_InitMeasuredBoot(tcg2Protocol, imageHandle, err) != 0)
    {
        LOGE(L"failed to initialize measured boot: %s", Wcs(err->buf));
        status = EFI_LOAD_ERROR;
        goto done;
    }

done:

    return status;
}

EFI_STATUS MeasureBinary(
    const CHAR16* name,
    const void* data,
    UINTN size,
    SHA256Hash* hash)
{
    EFI_STATUS status = EFI_SUCCESS;
    SHA1Hash sha1;
    Error err;

    ClearErr(&err);

    /* Compute the hashes */
    {
        if (!ComputeSHA1(data, size, &sha1))
        {
            status = EFI_UNSUPPORTED;
            goto done;
        }

        if (!ComputeSHA256(data, size, hash))
        {
            status = EFI_UNSUPPORTED;
            goto done;
        }
    }

    /* Measure the binary */
    {
        if ((status = MeasureHash(
            globals.tcg2Protocol,
            globals.imageHandle,
            11,
            name,
            &sha1,
            hash,
            &err)) != EFI_SUCCESS)
        {
            LOGE(L"measurement of %s failed: %s", Wcs(name), Wcs(err.buf));
            goto done;
        }
    }

done:

    return status;
}

EFI_STATUS MeasurePEImage(
    UINT32 pcr,
    const CHAR16* name,
    const char* logDescription,
    const void* data,
    UINTN size,
    SHA1Hash* sha1,
    SHA256Hash* sha256)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    Error err;

    ClearErr(&err);

    /* Compute the hashes */
    if (ParseAndHashImage(data, size, sha1, sha256) != 0)
    {
        LOGE(L"failed to compute PE hash: %s", Wcs(name));
        goto done;
    }

    /* Hash-log-extend this image */
    if (HashLogExtendPEImage(
        globals.tcg2Protocol,
        pcr,
        (EFI_PHYSICAL_ADDRESS)data,
        size,
        logDescription) != EFI_SUCCESS)
    {
        LOGE(L"hash-log-extend failed: %s", Wcs(name));
        goto done;
    }

    /* Log the measurement */
    LogSHA1(name, sha1);
    LogSHA256(name, sha256);

    status = EFI_SUCCESS;

done:

    return status;
}

EFI_STATUS HashLogExtendData(
    EFI_TCG2_PROTOCOL *tcg2Protocol,
    UINT32 pcr,
    const UINT8* data,
    UINTN size)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    EFI_TCG2_EVENT *event = NULL;
    UINTN eventSize;
    char description[] = "Capping measurement";

    /* Allocate the event */
    {
        eventSize = sizeof(*event) - sizeof(event->Event) + sizeof(description);

        if (!(event = AllocatePool(eventSize)))
        {
            status = EFI_OUT_OF_RESOURCES;
            goto done;
        }
    }

    /* Initialize the event */
    event->Header.HeaderSize = sizeof(EFI_TCG2_EVENT_HEADER);
    event->Header.HeaderVersion = EFI_TCG2_EVENT_HEADER_VERSION;
    event->Header.PCRIndex = pcr;
    event->Header.EventType = EV_COMPACT_HASH;
    event->Size = eventSize;

    /* Set the event detail into the event */
    Memcpy(event->Event, description, sizeof(description));

    /* Call the TCG2 interface to hash-log-extend */
    if ((status = uefi_call_wrapper(
        tcg2Protocol->HashLogExtendEvent, 
        5, 
        tcg2Protocol,
        0, 
        (EFI_PHYSICAL_ADDRESS)data,
        (UINT64)size,
        event)) != EFI_SUCCESS)
    {
        goto done;
    }

    status = EFI_SUCCESS;

done:

    if (event)
        FreePool(event);

    return status;
}

EFI_STATUS HashLogExtendSeparator(
    EFI_TCG2_PROTOCOL *tcg2Protocol,
    UINT32 pcr)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    EFI_TCG2_EVENT *event = NULL;
    UINTN eventSize;
    UINT32 separator = 0;

    /* Allocate the event */
    {
        eventSize = sizeof(*event) - sizeof(event->Event) + sizeof(separator);

        if (!(event = AllocatePool(eventSize)))
        {
            status = EFI_OUT_OF_RESOURCES;
            goto done;
        }
    }

    /* Initialize the event */
    event->Header.HeaderSize = sizeof(EFI_TCG2_EVENT_HEADER);
    event->Header.HeaderVersion = EFI_TCG2_EVENT_HEADER_VERSION;
    event->Header.PCRIndex = pcr;
    event->Header.EventType = EV_SEPARATOR;
    event->Size = eventSize;

    /* Set the event detail into the event */
    Memcpy(event->Event, &separator, sizeof(separator));

    /* Call the TCG2 interface to hash-log-extend */
    if ((status = uefi_call_wrapper(
        tcg2Protocol->HashLogExtendEvent, 
        5, 
        tcg2Protocol,
        0, 
        (EFI_PHYSICAL_ADDRESS)&separator,
        (UINT64)sizeof(separator),
        event)) != EFI_SUCCESS)
    {
        goto done;
    }

    status = EFI_SUCCESS;

done:

    if (event)
        FreePool(event);

    return status;
}

EFI_STATUS HashLogExtendPEImage(
    EFI_TCG2_PROTOCOL *tcg2Protocol,
    UINT32 pcr,
    EFI_PHYSICAL_ADDRESS data, 
    UINTN size, 
    const char *description)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    EFI_TCG2_EVENT *event = NULL;
    UINTN descriptionSize;
    UINTN eventSize;
    UINT64 flags = 0;

    /* Allocate the event */
    {
        descriptionSize = Strlen(description) + 1;
        eventSize = sizeof(*event) - sizeof(event->Event) + descriptionSize;

        if (!(event = AllocatePool(eventSize)))
        {
            status = EFI_OUT_OF_RESOURCES;
            goto done;
        }
    }

    /* Initialize the event */
    event->Header.HeaderSize = sizeof(EFI_TCG2_EVENT_HEADER);
    event->Header.HeaderVersion = EFI_TCG2_EVENT_HEADER_VERSION;
    event->Header.PCRIndex = pcr;
    event->Header.EventType = EV_IPL;
    event->Size = eventSize;

    /* Set the event detail into the event */
    Memcpy(event->Event, description, descriptionSize);

    /* Set the flags */
#if 0
    flags |= EFI_TCG2_EXTEND_ONLY;
#endif
    flags |= PE_COFF_IMAGE;

    /* Call the TCG2 interface to hash-log-extend */
    if ((status = uefi_call_wrapper(
        tcg2Protocol->HashLogExtendEvent, 
        5, 
        tcg2Protocol,
        flags, 
        data, 
        (UINT64)size,
        event)) != EFI_SUCCESS)
    {
        goto done;
    }

    status = EFI_SUCCESS;

done:

    if (event)
        FreePool(event);

    return status;
}

EFI_STATUS MeasureLinuxScenario(
    EFI_TCG2_PROTOCOL *tcg2Protocol,
    EFI_HANDLE imageHandle)
{
    EFI_STATUS status = EFI_UNSUPPORTED;

    /* Little endian byte ordering for these values. */
    UINT8 ALLOW_PREBOOT_SEALING[] = { 0x10, 0x00, 0x00, 0x00 };
    UINT8 LINUX_SCENARIO_ID[] = { 0x02, 0x00, 0xF0, 0x00 };
    UINT8 LINUX_SCENARIO_VERSION[] = { 0x01, 0x00, 0x00, 0x00 };

    /* Measure ALLOW_PREBOOT_SEALING */
    if (HashLogExtendData(
        tcg2Protocol,
        SCENARIO_PCR,
        ALLOW_PREBOOT_SEALING,
        sizeof(ALLOW_PREBOOT_SEALING)) != EFI_SUCCESS)
    {
        LOGE(L"hash-log-extend failed: ALLOW_PREBOOT_SEALING");
        goto done;
    }

    /* Measure LINUX_SCENARIO_ID */
    if (HashLogExtendData(
        tcg2Protocol,
        SCENARIO_PCR,
        LINUX_SCENARIO_ID,
        sizeof(LINUX_SCENARIO_ID)) != EFI_SUCCESS)
    {
        LOGE(L"hash-log-extend failed: LINUX_SCENARIO_ID");
        goto done;
    }

    /* Measure LINUX_SCENARIO_VERSION */
    if (HashLogExtendData(
        tcg2Protocol,
        SCENARIO_PCR,
        LINUX_SCENARIO_VERSION,
        sizeof(LINUX_SCENARIO_VERSION)) != EFI_SUCCESS)
    {
        LOGE(L"hash-log-extend failed: LINUX_SCENARIO_VERSION");
        goto done;
    }

    status = EFI_SUCCESS;

done:
    return status;
}

EFI_STATUS LoadPrefixedFile(
    EFI_HANDLE imageHandle,
    EFI_TCG2_PROTOCOL* tcg2Protocol,
    EXT2* bootfs,
    const CHAR16* path,
    void** data,
    UINTN* size)
{
    EFI_STATUS status = EFI_UNSUPPORTED;

    /* Reject null parameters */
    if (!imageHandle || !tcg2Protocol || !path || !data || !size)
        goto done;

    *data = NULL;
    *size = 0;

    /* Check for "boot:" prefix */
    if (StrnCmp(path, L"boot:", 5) == 0)
    {
        /* Try to open the BIO for the LUKS parition */
        if (!bootfs)
        {
            LOGE(L"boot partition unavailable");
            goto done;
        }

        /* Try to load the file from the bootfs */
        if (LoadFileFromBootFS(
            imageHandle,
            tcg2Protocol,
            bootfs,
            path + 5, /* skip "boot:" prefix */
            data,
            size) != EFI_SUCCESS)
        {
            goto done;
        }
    }
    else if (StrnCmp(path, L"esp:", 4) == 0)
    {
        CHAR16 dosPath[PATH_SIZE];
        UnixToDosPath(dosPath, path + 4);

        if (EFILoadFile(imageHandle, dosPath, data, size) != EFI_SUCCESS)
            goto done;
    }
    else /* default to ESP */
    {
        CHAR16 dosPath[PATH_SIZE];
        UnixToDosPath(dosPath, path);

        if (EFILoadFile(imageHandle, dosPath, data, size) != EFI_SUCCESS)
            goto done;
    }

    status = EFI_SUCCESS;

done:

    if (status != EFI_SUCCESS)
    {
        if (*data)
        {
            Free(data);
            *data = NULL;
            *size = 0;
        }
    }

    return status;
}

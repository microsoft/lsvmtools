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
#include "kernel.h"
#include "config.h"
#include <lsvmutils/eficommon.h>
#include <lsvmutils/alloc.h>
#include <lsvmutils/peimage.h>
#include <lsvmutils/linux.h>
#include <lsvmutils/efifile.h>
#include "measure.h"
#include "log.h"
#include "logging.h"
#include "bootfs.h"
#include "paths.h"
#include "simpletext.h"
#include "strings.h"
#include "globals.h"
#include "trace.h"

#ifndef __x86_64__
# error "unsupported architecture"
#endif

#define MAX_SETUP_SECTS 64

#define BOOT_FLAG 0xAA55

#define SECTOR_SIZE ((UINTN)512)

static UINTN _ByteToPages(UINTN bytes)
{
    const UINTN pageSize = 4096;
    return (bytes + pageSize - 1) / pageSize;
}

typedef void(*HandoverFunction)(
    EFI_HANDLE imageHandle, 
    EFI_SYSTEM_TABLE *systemTable, 
    boot_params_t *bootParams);

static EFI_STATUS _InvokeHandoverFunction(
    void* kernelData,
    UINTN kernelSize,
    boot_params_t* params,
    UINT32 handoverOffset)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    HandoverFunction func;

    /* Check parameters */
    if (!kernelData || !kernelSize || !params || !handoverOffset)
    {
        status = EFI_INVALID_PARAMETER;
        goto done;
    }

    /* Check whether offset is out of range */
    if (handoverOffset >= kernelSize)
    {
        status = EFI_UNSUPPORTED;
        goto done;
    }

    /* X86-64-Bit requires adding 512 bytes more (one sector) */
    func = (HandoverFunction)((char*)kernelData + handoverOffset + 512);

    /* Clear interrupts */
    asm volatile ("cli");

    LOGI(L"Invoking EFI handover function");

    /* Invoke the function (should not return) */
    (*func)(globals.imageHandle, globals.systemTable, params);

    LOGE(L"EFI handover function returned");

    status = EFI_SUCCESS;

done:
    return status;
}

static EFI_STATUS _AllocatePages(
    EFI_ALLOCATE_TYPE type,
    EFI_PHYSICAL_ADDRESS address,
    UINTN numPages,
    void** ptr)
{
    EFI_STATUS status = EFI_UNSUPPORTED;

    if (address == 0 || !ptr || numPages == 0)
    {
        status = EFI_INVALID_PARAMETER;
        goto done;
    }

    *ptr = NULL;

    if ((status = uefi_call_wrapper(
        BS->AllocatePages,
        4,
        type,
        EfiLoaderData,
        numPages,
        &address)) != EFI_SUCCESS)
    {
        goto done;
    }

    if (!address)
    {
        LOGE(L"AllocatePages() returned zero address");
        status = EFI_OUT_OF_RESOURCES;
        goto done;
    }

    *ptr = (void*)address;
    status = EFI_SUCCESS;

done:

    return status;
}

static EFI_STATUS _FreePages(
    void* ptr,
    UINTN numPages)
{
    EFI_STATUS status = EFI_UNSUPPORTED;

    if (!ptr || numPages == 0)
    {
        status = EFI_INVALID_PARAMETER;
        goto done;
    }

    if ((status = uefi_call_wrapper(
        BS->FreePages,
        2,
        (EFI_PHYSICAL_ADDRESS)ptr,
        numPages)) != EFI_SUCCESS)
    {
        goto done;
    }

done:
    return status;
}

static EFI_STATUS _LoadInitrd(
    EXT2* bootfs,
    const CHAR16* initrdPath,
    void** initrdDataOut,
    UINTN* initrdSizeOut,
    UINT32* errorLine)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    EFIFile* file = NULL;
    void* data = NULL;
    UINTN size;
    void* initrdData = NULL; /* pointer to initrd pages */
    UINTN initrdSize = 0; /* aligned size of initrd */

    /* Check for null parameters */
    if (!initrdPath || !initrdDataOut || !initrdSizeOut || !errorLine)
    {
        status = EFI_INVALID_PARAMETER;
        *errorLine = __LINE__;
        goto done;
    }

    /* Initialize output parameters */
    *initrdDataOut = NULL;
    *initrdSizeOut = 0;
    *errorLine = 0;

    /* Do not allow initrd to be loaded from ESP */
    if (StrnCmp(initrdPath, L"esp:", 4) == 0)
    {
        LOGE(L"initrd must reside on an encrypted parition: %s", initrdPath);
        goto done;
    }

    /* Load the file into plain old memory */
    if ((status = LoadFileFromBootFS(
        globals.imageHandle, 
        globals.tcg2Protocol, 
        bootfs,
        initrdPath,
        &data, 
        &size)) != EFI_SUCCESS)
    {
        *errorLine = __LINE__;
        goto done;
    }

    /* Round size of the file to next multiple of 4 */
    initrdSize = (size + 3) / size * size;

    /* Allocate space to place this file into memory for the kernel */
    if ((status = _AllocatePages(
        AllocateMaxAddress,
        0x3fffffff,
        _ByteToPages(initrdSize),
        &initrdData)) != EFI_SUCCESS)
    {
        *errorLine = __LINE__;
        goto done;
    }

    /* Copy loaded file onto allocated memory */
    Memcpy(initrdData, data, size);

    /* Zero fill any padding bytes at the end */
    if (initrdSize > size)
        Memset((unsigned char*)initrdData + size, 0, initrdSize - size);

    /* Set output parameters */
    *initrdDataOut = initrdData;
    *initrdSizeOut = initrdSize;

    /* Success! */
    *errorLine = 0;
    status = EFI_SUCCESS;

done:

    if (file)
        CloseFile(file);

    if (data)
        Free(data);

    if (status != EFI_SUCCESS)
    {
        if (initrdData)
            _FreePages(initrdData, _ByteToPages(initrdSize));
    }

    return status;
}

static EFI_STATUS _CheckSetupHeader(
    setup_header_t* header)
{
    EFI_STATUS status = EFI_UNSUPPORTED;

    /* Check parameters */
    if (!header)
    {
        goto done;
    }

    /* Check the boot flag magic number */
    if (header->boot_flag != BOOT_FLAG)
    {
        goto done;
    }

    /* Check whether too many setup sections */
    if (header->setup_sects > MAX_SETUP_SECTS)
    {
        goto done;
    }

    /* Reject old versions of kernel (< 2.11) */
    if (header->version < MINIMUM_SUPPORTED_VERSION)
    {
        goto done;
    }

    /* If kernel does not have an EFI handover offset */
    if (header->handover_offset == 0)
    {
        goto done;
    }

    status = EFI_SUCCESS;

done:
    return status;
}

static EFI_STATUS _FormatCommandLine(
    char* str,
    UINTN size)
{
    EFI_STATUS status = EFI_UNSUPPORTED;

    if (!str || !globals.kernelPath || !globals.initrdPath || 
        !globals.rootDevice)
    {
        goto done;
    }

    *str = 0;

    /* Append BOOT_IMAGE=KernelPath argument */
    {
        if (Strlcat(str, "BOOT_IMAGE=", size) >= size)
            goto done;

        if (StrWcslcat(str, globals.kernelPath, size) >= size)
            goto done;
    }

    /* Append root=RootDevice argument */
    {
        if (Strlcat(str, " root=", size) >= size)
            goto done;

        if (StrWcslcat(str, globals.rootDevice, size) >= size)
            goto done;
    }

    /* Append "ro" argument */
    if (Strlcat(str, " ro", size) >= size)
        goto done;

    LOGI(L"kernel command line: {%a}", str);

    status = EFI_SUCCESS;

done:
    return status;
}

static EFI_STATUS _LoadKernel(
    EXT2* bootfs,
    const CHAR16* kernelPath,
    UINT32* handoverOffset,
    void** kernelDataOut,
    UINTN* kernelSizeOut,
    boot_params_t **paramsDataOut,
    UINT32* errorLine)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    setup_header_t sh;
    void* fileData = NULL;
    UINTN fileSize;
    UINTN numPages;
    boot_params_t* paramsData = NULL;
    const UINTN paramsSize = 16384;
    static char *cmdline = NULL;
    UINTN kernelStart;
    void* kernelData = NULL;
    UINTN kernelSize;

    /* Check parameters */
    if (!kernelPath || !handoverOffset || !kernelDataOut || !kernelSizeOut ||
        !paramsDataOut || !errorLine)
    {
        status = EFI_INVALID_PARAMETER;
        *errorLine = __LINE__;
        goto done;
    }

    /* Do not allow kernel to be loaded from ESP */
    if (StrnCmp(kernelPath, L"esp:", 4) == 0)
    {
        LOGE(L"kernel must reside on an encrypted parition: %s", kernelPath);
        goto done;
    }

    /* Load the kernel file into memory */
    if ((status = LoadFileFromBootFS(
        globals.imageHandle,
        globals.tcg2Protocol,
        bootfs,
        kernelPath,
        &fileData,
        &fileSize)) != EFI_SUCCESS)
    {
        *errorLine = __LINE__;
        goto done;
    }

    /* Check the certificate */
    {
        void* certData = NULL;
        UINTN certSize = 0;

        extern unsigned char g_cert[];
        extern unsigned int g_cert_size;
        certData = (void*)g_cert;
        certSize = g_cert_size;

        if (CheckCert(fileData, fileSize, certData, certSize) != 0)
        {
            /* ATTN: enforce this later */
            LOGE(L"Kernel certificate check failed");
        }
        else
        {
            LOGI(L"Certificate check okay");
        }
    }

    /* If kernel is too small to even have leading params */
    if (fileSize < paramsSize)
    {
        status = EFI_UNSUPPORTED;
        *errorLine = __LINE__;
        goto done;
    }

    /* Calculate the required number of pages */
    numPages = _ByteToPages(paramsSize);

    /* Allocate space for the parameters */
    if ((status = _AllocatePages(
        AllocateMaxAddress,
        0x3fffffff,
        numPages,
        (void**)&paramsData)) != EFI_SUCCESS)
    {
        *errorLine = __LINE__;
        goto done;
    }

    /* Zero-fill the memory */
    Memset(paramsData, 0, paramsSize);

    /* Get the header */
    Memcpy(&sh, (unsigned char*)fileData + SETUP_OFFSET, sizeof(sh));

    /* Check sanity of setup header */
    if ((status = _CheckSetupHeader(&sh)) != EFI_SUCCESS)
    {
        *errorLine = __LINE__;
        goto done;
    }

    /* Allocate space for the Linux command line */
    if ((status = _AllocatePages(
        AllocateMaxAddress,
        0x3fffffff,
        _ByteToPages(sh.cmdline_size + 1),
        (void**)&cmdline)) != EFI_SUCCESS)
    {
        *errorLine = __LINE__;
        goto done;
    }

    /* Build the command line parameters */
    if ((status = _FormatCommandLine(
        cmdline, 
        sh.cmdline_size + 1)) != EFI_SUCCESS)
    {
        *errorLine = __LINE__;
        goto done;
    }

    /* Set the pointer to the command line */
    sh.cmd_line_ptr = (UINT32)(EFI_PHYSICAL_ADDRESS)cmdline;

    /* Set the handover offset output parameter */
    *handoverOffset = sh.handover_offset;

    /* Find the start of the image (after all the setup sectors) */
    kernelStart = (sh.setup_sects + 1) * SECTOR_SIZE;

    /* Find the size of the kernel proper (excluding setup sectors) */
    kernelSize = fileSize - kernelStart;

    /* Allocate space for eventual uncompressed kernel (sh.init_size) */
    if ((status = _AllocatePages(
        AllocateAddress,
        sh.pref_address,
        _ByteToPages(sh.init_size),
        &kernelData)) != EFI_SUCCESS)
    {
        if ((status = _AllocatePages(
            AllocateMaxAddress,
            0x3fffffff,
            _ByteToPages(sh.init_size),
            &kernelData)) != EFI_SUCCESS)
        {
            *errorLine = __LINE__;
            goto done;
        }
    }

    /* Copy the kernel onto the allocated pages (excluding startup) */
    Memcpy(
        kernelData, 
        (unsigned char*)fileData + kernelStart,
        kernelSize);

    /* Set pointer to kernel */
    sh.code32_start = (UINT32)(EFI_PHYSICAL_ADDRESS)kernelData;

    /* Set the loader type (unknown boot loader) */
    sh.type_of_loader = 0xFF;

    /* Copy the leading bytes and header onto the params pages */
    Memcpy(paramsData, fileData, SETUP_OFFSET);
    Memcpy((unsigned char*)paramsData + SETUP_OFFSET, &sh, sizeof(sh));

    /* Set the output parameters */
    *kernelDataOut = kernelData;
    *kernelSizeOut = kernelSize;
    *paramsDataOut = paramsData;

    *errorLine = 0;
    status = EFI_SUCCESS;

done:

    if (fileData)
        Free(fileData);

    if (status != EFI_SUCCESS)
    {
        if (paramsData)
            _FreePages(paramsData, _ByteToPages(paramsSize));

        if (cmdline)
            _FreePages(cmdline, _ByteToPages(sh.cmdline_size + 1));

        if (kernelData)
            _FreePages(kernelData, _ByteToPages(sh.init_size));

        *kernelDataOut = NULL;
        *kernelSizeOut = 0;
        *paramsDataOut = NULL;
    }

    return status;
}

EFI_STATUS StartKernel(
    EXT2* bootfs)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    UINT32 handoverOffset = 0;
    void* kernelData = NULL;
    UINTN kernelSize = 0;
    boot_params_t *paramsData = NULL;
    void* initrdData = NULL;
    UINTN initrdSize = 0;
    UINT32 errorLine = 0;

    /* If the kernel and initrd paths are null, then fail now */
    if (!globals.kernelPath || !globals.initrdPath || !globals.rootDevice)
    {
        LOGE(L"Plese specify KernelPath and InitrdPath in lsvmconf");
        goto done;
    }

    /* Print status message */
    SetColors(EFI_YELLOW, EFI_BLACK);
    Print(L"Loading kernel: %s\n", globals.kernelPath);
    SetColors(EFI_BRIGHT, EFI_BLACK);

    /* Load the kernel */
    if ((status = _LoadKernel(
        bootfs,
        globals.kernelPath,
        &handoverOffset,
        &kernelData,
        &kernelSize,
        &paramsData,
        &errorLine)) != EFI_SUCCESS)
    {
        LOGE(L"Failed to load kernel: %d", errorLine);
        goto done;
    }

    LOGI(L"loaded kernel: {%s}", globals.kernelPath);

    /* Print status message */
    SetColors(EFI_YELLOW, EFI_BLACK);
    Print(L"Loading initrd: %s\n", globals.initrdPath);
    SetColors(EFI_BRIGHT, EFI_BLACK);

    /* Load the initial ramdisk */
    if ((status = _LoadInitrd(
        bootfs,
        globals.initrdPath,
        &initrdData,
        &initrdSize,
        &errorLine)) != EFI_SUCCESS)
    {
        LOGE(L"Failed to load initrd: %d", errorLine);
        goto done;
    }

    LOGI(L"loaded initrd: {%s}", globals.initrdPath);

    /* Log the PCRs one final fime */
    if (GetLogLevel() >= DEBUG)
        LogPCRs(globals.tcg2Protocol, globals.imageHandle, L"PCR Values");

    /* Update the params with the initrd info */
    paramsData->setup.ramdisk_size = initrdSize;
    paramsData->setup.ramdisk_image = (UINT32)(EFI_PHYSICAL_ADDRESS)initrdData;

    /* Jump to the EFI entry point in the kernel */
    if ((status = _InvokeHandoverFunction(
        kernelData,
        kernelSize,
        paramsData,
        handoverOffset)) != EFI_SUCCESS)
    {
        LOGE(L"Failed to invoke EFI handover function");
        goto done;
    }

    LOGE(L"Returned from InvokeHandoverFunction()");

    /* Failure! Function above should never return */
    status = EFI_UNSUPPORTED;

done:
    return status;
}

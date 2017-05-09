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
#include "tcg2.h"
#include "strings.h"
#include "print.h"

#if !defined(BUILD_EFI)
# include <stdlib.h>
# include <string.h>
# include <stdio.h>
#endif

#if defined(_WIN32)
# include <tbs.h>
#endif

#if !defined(BUILD_EFI) && defined(__linux__)
# include <unistd.h>
# include <fcntl.h>
#endif

const TCHAR *TCG2_StatusToStr(EFI_STATUS status)
{
    typedef struct _Pair
    {
        EFI_STATUS status;
        const TCHAR *string;
    }
    Pair;

    static const Pair arr[] =
    {
        { EFI_SUCCESS, TCS("EFI_SUCCESS") },
        { EFI_LOAD_ERROR, TCS("EFI_LOAD_ERROR") },
        { EFI_INVALID_PARAMETER, TCS("EFI_INVALID_PARAMETER") },
        { EFI_UNSUPPORTED, TCS("EFI_UNSUPPORTED") },
        { EFI_BAD_BUFFER_SIZE, TCS("EFI_BAD_BUFFER_SIZE") },
        { EFI_BUFFER_TOO_SMALL, TCS("EFI_BUFFER_TOO_SMALL") },
        { EFI_NOT_READY, TCS("EFI_NOT_READY") },
        { EFI_DEVICE_ERROR, TCS("EFI_DEVICE_ERROR") },
        { EFI_WRITE_PROTECTED, TCS("EFI_WRITE_PROTECTED") },
        { EFI_OUT_OF_RESOURCES, TCS("EFI_OUT_OF_RESOURCES") },
        { EFI_VOLUME_CORRUPTED, TCS("EFI_VOLUME_CORRUPTED") },
        { EFI_VOLUME_FULL, TCS("EFI_VOLUME_FULL") },
        { EFI_NO_MEDIA, TCS("EFI_NO_MEDIA") },
        { EFI_MEDIA_CHANGED, TCS("EFI_MEDIA_CHANGED") },
        { EFI_NOT_FOUND, TCS("EFI_NOT_FOUND") },
        { EFI_ACCESS_DENIED, TCS("EFI_ACCESS_DENIED") },
        { EFI_NO_RESPONSE, TCS("EFI_NO_RESPONSE") },
        { EFI_NO_MAPPING, TCS("EFI_NO_MAPPING") },
        { EFI_TIMEOUT, TCS("EFI_TIMEOUT") },
        { EFI_NOT_STARTED, TCS("EFI_NOT_STARTED") },
        { EFI_ALREADY_STARTED, TCS("EFI_ALREADY_STARTED") },
        { EFI_ABORTED, TCS("EFI_ABORTED") },
        { EFI_ICMP_ERROR, TCS("EFI_ICMP_ERROR") },
        { EFI_TFTP_ERROR, TCS("EFI_TFTP_ERROR") },
        { EFI_PROTOCOL_ERROR, TCS("EFI_PROTOCOL_ERROR") },
        { EFI_INCOMPATIBLE_VERSION, TCS("EFI_INCOMPATIBLE_VERSION") },
        { EFI_SECURITY_VIOLATION, TCS("EFI_SECURITY_VIOLATION") },
        { EFI_CRC_ERROR, TCS("EFI_CRC_ERROR") },
        { EFI_END_OF_MEDIA, TCS("EFI_END_OF_MEDIA") },
        { EFI_END_OF_FILE, TCS("EFI_END_OF_FILE") },
        { EFI_INVALID_LANGUAGE, TCS("EFI_INVALID_LANGUAGE") },
        { EFI_COMPROMISED_DATA, TCS("EFI_COMPROMISED_DATA") },
    };
    UINT32 n = sizeof(arr) / sizeof(arr[0]);
    UINT32 i;

    for (i = 0; i < n; i++)
    {
        if (arr[i].status == status)
            return arr[i].string;
    }

    return TCS("UNKNOWN");
}

/*
**==============================================================================
**
** !defined(BUILD_EFI)
**
**==============================================================================
*/

#if !defined(BUILD_EFI)

typedef struct _EFI_TCG2_PROTOCOL_EXT
{
    EFI_TCG2_PROTOCOL protocol;

#if defined(__linux__)
    int fd;
#endif

#if defined(_WIN32)
    TBS_HCONTEXT context;
#endif
}
EFI_TCG2_PROTOCOL_EXT;

static EFI_STATUS _GetCapabilityCallback(
    IN EFI_TCG2_PROTOCOL *This,
    IN OUT EFI_TCG2_BOOT_SERVICE_CAPABILITY *ProtocolCapability)
{
    EFI_TCG2_PROTOCOL_EXT* ext = (EFI_TCG2_PROTOCOL_EXT*)This;
    EFI_TCG2_BOOT_SERVICE_CAPABILITY _capability =
    {
        sizeof(EFI_TCG2_BOOT_SERVICE_CAPABILITY),
        { 1, 1 }, /* StructureVersion */
        { 1, 1 }, /* ProtocolVersion */
        0, /* HashAlgorithmBitmap */
        0, /* SupportedEventLogs */
        1, /* TPMPresentFlag */
        4096, /* MaxCommandSize */
        4096, /* MaxResponseSize */
        0, /* ManufacturerID*/
        24, /* NumberOfPcrBanks*/
        24, /* ActivePcrBanks*/
    };

    if (!This || !ProtocolCapability)
        return EFI_INVALID_PARAMETER;

    if (ProtocolCapability->Size < sizeof(EFI_TCG2_BOOT_SERVICE_CAPABILITY))
        return EFI_BUFFER_TOO_SMALL;

    memcpy(ProtocolCapability, &_capability, sizeof(_capability));

#if defined(__linux__)
    ProtocolCapability->TPMPresentFlag = (ext->fd >= 0) ? 1 : 0;
#endif

#if defined(_WIN32)
    ProtocolCapability->TPMPresentFlag = ext->context ? 1 : 0;
#endif

    return EFI_SUCCESS;
}

static EFI_STATUS _SubmitCommandCallback(
    IN EFI_TCG2_PROTOCOL *This,
    IN UINT32 InputSize,
    IN UINT8 *Input,
    IN UINT32 OutputSize,
    IN UINT8 *Output)
{
    EFI_TCG2_PROTOCOL_EXT* ext = (EFI_TCG2_PROTOCOL_EXT*)This;
    const size_t HEADER_SIZE = sizeof(UINT16) + sizeof(UINT32) + sizeof(UINT32);

#if defined(__linux__)

    if (!ext || ext->fd < 0)
        return EFI_INVALID_PARAMETER;

    {
        ssize_t n;

        if (write(ext->fd, Input, InputSize) != InputSize)
            return EFI_DEVICE_ERROR;

        if ((n = read(ext->fd, Output, OutputSize)) < HEADER_SIZE)
            return EFI_DEVICE_ERROR;
    }

#endif  /* defined(__linux__) */

#if defined(_WIN32)

    if (!ext || !ext->context)
        return EFI_INVALID_PARAMETER;

    {
        TBS_RESULT result;
        UINT32 tmpSize = OutputSize;

        if ((result = Tbsip_Submit_Command(
            ext->context,
            TBS_COMMAND_LOCALITY_ZERO,
            TBS_COMMAND_PRIORITY_NORMAL,
            Input,
            InputSize,
            Output,
            &tmpSize)) != TBS_SUCCESS)
        {
            return EFI_DEVICE_ERROR;
        }
    }

#endif /* defined(_WIN32) */

    return EFI_SUCCESS;
}

EFI_TCG2_PROTOCOL *TCG2_GetProtocol()
{
    static EFI_TCG2_PROTOCOL _protocol =
    {
        _GetCapabilityCallback,
        NULL,
        NULL,
        _SubmitCommandCallback,
        NULL,
        NULL,
        NULL
    };
    EFI_TCG2_PROTOCOL_EXT* ext;

    if (!(ext = (EFI_TCG2_PROTOCOL_EXT*)malloc(sizeof(EFI_TCG2_PROTOCOL_EXT))))
        return NULL;

    Memset(ext, 0, sizeof(EFI_TCG2_PROTOCOL_EXT));
    ext->protocol = _protocol;

#if defined(__linux__)
    {
        ext->fd = open("/dev/tpm0", O_RDWR);
    }
#endif /* defined(__linux__) */

#if defined(_WIN32)
    {
        TBS_RESULT result;
        TBS_CONTEXT_PARAMS2 params;

        /* Initialize the context parameters */
        Memset(&params, 0, sizeof(params));
        params.version = TPM_VERSION_20;
        params.includeTpm20 = 1;
        
        /* Create the context */
        if ((result = Tbsi_Context_Create(
            (TBS_CONTEXT_PARAMS*)&params, 
            &ext->context)) != TBS_SUCCESS)
        {
            free(ext);
            return NULL;
        }

        return &ext->protocol;
    }
#endif /* defined(_WIN32) */

    return &ext->protocol;
}

void TCG2_ReleaseProtocol(EFI_TCG2_PROTOCOL* protocol)
{
    EFI_TCG2_PROTOCOL_EXT* ext = (EFI_TCG2_PROTOCOL_EXT*)protocol;


#if defined(__linux__)
    if (ext->fd != -1)
        close(ext->fd);
#endif

#if defined(_WIN32)
    if (ext->context)
        Tbsip_Context_Close(ext->context);
#endif

    free(protocol);
}

EFI_STATUS TCG2_GetCapability(
    EFI_TCG2_PROTOCOL *protocol,
    EFI_TCG2_BOOT_SERVICE_CAPABILITY* capability)
{
    EFI_STATUS status = EFI_SUCCESS;

    capability->Size = sizeof(EFI_TCG2_BOOT_SERVICE_CAPABILITY);
    status = (*protocol->GetCapability)(protocol, capability);

    if (status != EFI_SUCCESS)
        return status;

    return status;
}

EFI_STATUS TCG2_SubmitCommand(
    EFI_TCG2_PROTOCOL *protocol,
    IN UINT32 InputParameterBlockSize,
    IN UINT8 *InputParameterBlock,
    IN UINT32 OutputParameterBlockSize,
    IN UINT8 *OutputParameterBlock)
{
    return protocol->SubmitCommand(
        protocol,
        InputParameterBlockSize,
        InputParameterBlock,
        OutputParameterBlockSize,
        OutputParameterBlock);
}

#endif /* !defined(BUILD_EFI) */

/*
**==============================================================================
**
** defined(BUILD_EFI)
**
**==============================================================================
*/

#if defined(BUILD_EFI)

EFI_TCG2_PROTOCOL *TCG2_GetProtocol()
{
    EFI_STATUS status;
    EFI_GUID guid = EFI_TCG2_PROTOCOL_GUID;
    EFI_TCG2_PROTOCOL *protocol = NULL;

    status = uefi_call_wrapper(
        BS->LocateProtocol, 
        3, 
        &guid, 
        NULL, 
        (void**)&protocol);

    if (status != EFI_SUCCESS)
        return NULL;

    return protocol;
}

EFI_STATUS TCG2_GetCapability(
    EFI_TCG2_PROTOCOL *protocol,
    EFI_TCG2_BOOT_SERVICE_CAPABILITY* capability)
{
    EFI_STATUS status = EFI_SUCCESS;

    if (!protocol)
        protocol = TCG2_GetProtocol();

    capability->Size = sizeof(EFI_TCG2_BOOT_SERVICE_CAPABILITY);
    status = (*protocol->GetCapability)(protocol, capability);

    if (status != EFI_SUCCESS)
        return status;

    return status;
}

EFI_STATUS TCG2_SubmitCommand(
    EFI_TCG2_PROTOCOL *protocol,
    IN UINT32 InputParameterBlockSize,
    IN UINT8 *InputParameterBlock,
    IN UINT32 OutputParameterBlockSize,
    IN UINT8 *OutputParameterBlock)
{
    if (!protocol)
        protocol = TCG2_GetProtocol();

    return protocol->SubmitCommand(
        protocol,
        InputParameterBlockSize,
        InputParameterBlock,
        OutputParameterBlockSize,
        OutputParameterBlock);
}

#endif /* defined(BUILD_EFI) */

/*
**==============================================================================
**
** Common:
**
**==============================================================================
*/

void DumpTCG2Capability(
    const EFI_TCG2_BOOT_SERVICE_CAPABILITY* capability)
{
    Print(TCS("struct EFI_TCG2_BOOT_SERVICE_CAPABILITY\n"));
    Print(TCS("{\n"));

    Print(TCS("    Size{%d}\n"), capability->Size);

    Print(TCS("    StructureVersion.Major{%d}\n"), 
        capability->StructureVersion.Major);

    Print(TCS("    StructureVersion.Minor{%d}\n"), 
        capability->StructureVersion.Minor);

    Print(TCS("    ProtocolVersion.Major{%d}\n"), 
        capability->ProtocolVersion.Major);

    Print(TCS("    ProtocolVersion.Minor{%d}\n"), 
        capability->ProtocolVersion.Minor);

    Print(TCS("    HashAlgorithmBitmap{%08X}\n"), 
        capability->HashAlgorithmBitmap);

    Print(TCS("    SupportedEventLogs{%08X}\n"), 
        capability->SupportedEventLogs);

    Print(TCS("    TPMPresentFlag{%d}\n"), 
        capability->TPMPresentFlag);

    Print(TCS("    MaxCommandSize{%d}\n"), 
        capability->MaxCommandSize);

    Print(TCS("    MaxResponseSize{%d}\n"), 
        capability->MaxResponseSize);

    Print(TCS("    ManufacturerID{%08X}\n"), 
        capability->ManufacturerID);

    Print(TCS("    NumberOfPcrBanks{%08X}\n"), 
        capability->NumberOfPcrBanks);

    Print(TCS("    ActivePcrBanks{%08X}\n"), 
        capability->ActivePcrBanks);

    Print(TCS("}\n"));
}

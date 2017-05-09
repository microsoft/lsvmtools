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
#ifndef _tcg2_h
#define _tcg2_h

#include "config.h"
#include "eficommon.h"

typedef struct _EFI_TCG2_PROTOCOL EFI_TCG2_PROTOCOL;
typedef unsigned long ULONG;

/*
**==============================================================================
**
** EFI_TCG2_PROTOCOL_GUID
**
**==============================================================================
*/

#define EFI_TCG2_PROTOCOL_GUID \
    {0x607f766c,0x7455,0x42be,{0x93,0x0b,0xe4,0xd7,0x6d,0xb2,0x72,0x0f}}

/*
**==============================================================================
**
** Forward type definitions
**
**==============================================================================
*/

typedef struct _EFI_TCG2_PROTOCOL EFI_TCG2_PROTOCOL;

/*
**==============================================================================
**
** EFI_TCG2_GET_CAPABILITY
**
**==============================================================================
*/

typedef UINT32 EFI_TCG2_EVENT_LOG_BITMAP;

typedef UINT32 EFI_TCG2_EVENT_ALGORITHM_BITMAP;

typedef struct _EFI_TCG2_VERSION 
{ 
    UINT8 Major; 
    UINT8 Minor; 
} 
EFI_TCG2_VERSION;

typedef struct _EFI_TCG2_BOOT_SERVICE_CAPABILITY 
{
    UINT8 Size;
    EFI_TCG2_VERSION StructureVersion; 
    EFI_TCG2_VERSION ProtocolVersion;
    EFI_TCG2_EVENT_ALGORITHM_BITMAP HashAlgorithmBitmap;
    EFI_TCG2_EVENT_LOG_BITMAP SupportedEventLogs;
    BOOLEAN TPMPresentFlag;
    UINT16 MaxCommandSize;
    UINT16 MaxResponseSize;
    UINT32 ManufacturerID;
    UINT32  NumberOfPcrBanks;
    EFI_TCG2_EVENT_ALGORITHM_BITMAP ActivePcrBanks;
}
EFI_TCG2_BOOT_SERVICE_CAPABILITY;

typedef EFI_STATUS (EFIAPI *EFI_TCG2_GET_CAPABILITY)(
    IN EFI_TCG2_PROTOCOL *This,
    IN OUT EFI_TCG2_BOOT_SERVICE_CAPABILITY *ProtocolCapability);

/*
**==============================================================================
**
** EFI_TCG2_GET_EVENT_LOG
**
**==============================================================================
*/

#define EV_POST_CODE ((TCG_EVENTTYPE) 0x00000001)
#define EV_NO_ACTION ((TCG_EVENTTYPE) 0x00000003)
#define EV_SEPARATOR ((TCG_EVENTTYPE) 0x00000004)
#define EV_S_CRTM_CONTENTS ((TCG_EVENTTYPE) 0x00000007)
#define EV_S_CRTM_VERSION ((TCG_EVENTTYPE) 0x00000008)
#define EV_CPU_MICROCODE ((TCG_EVENTTYPE) 0x00000009)
#define EV_TABLE_OF_DEVICES ((TCG_EVENTTYPE) 0x0000000B)

/* Vendor events */
#define EV_COMPACT_HASH ((TCG_EVENTTYPE) 0x0000000C)
#define EV_IPL ((TCG_EVENTTYPE) 0x0000000D)

#define EV_EFI_EVENT_BASE ((TCG_EVENTTYPE) 0x80000000)
#define EV_EFI_VARIABLE_DRIVER_CONFIG (EV_EFI_EVENT_BASE + 1)
#define EV_EFI_VARIABLE_BOOT (EV_EFI_EVENT_BASE + 2)
#define EV_EFI_BOOT_SERVICES_APPLICATION (EV_EFI_EVENT_BASE + 3)
#define EV_EFI_BOOT_SERVICES_DRIVER (EV_EFI_EVENT_BASE + 4)
#define EV_EFI_RUNTIME_SERVICES_DRIVER (EV_EFI_EVENT_BASE + 5)
#define EV_EFI_GPT_EVENT (EV_EFI_EVENT_BASE + 6)
#define EV_EFI_ACTION (EV_EFI_EVENT_BASE + 7)
#define EV_EFI_PLATFORM_FIRMWARE_BLOB (EV_EFI_EVENT_BASE + 8)
#define EV_EFI_HANDOFF_TABLES (EV_EFI_EVENT_BASE + 9)
#define EV_EFI_VARIABLE_AUTHORITY (EV_EFI_EVENT_BASE + 0xE0)

typedef UINT32 TCG_PCRINDEX;
typedef UINT32 TCG_EVENTTYPE;
typedef UINT8 TCG_DIGEST[20]; /* SHA1 */

typedef struct _TCG_PCR_EVENT 
{
    TCG_PCRINDEX PCRIndex;
    TCG_EVENTTYPE EventType;
    TCG_DIGEST Digest;
    UINT32 EventSize;
    UINT8 Event[];
}
TCG_PCR_EVENT;

#define EFI_TCG2_EVENT_LOG_FORMAT_TCG_1_2 0x00000001

#define EFI_TCG2_EVENT_LOG_FORMAT_TCG_2 0x00000002

typedef UINT32 EFI_TCG2_EVENT_LOG_FORMAT;

typedef UINT64 EFI_PHYSICAL_ADDRESS;

typedef EFI_STATUS (EFIAPI *EFI_TCG2_GET_EVENT_LOG)( 
    IN EFI_TCG2_PROTOCOL *This, 
    IN EFI_TCG2_EVENT_LOG_FORMAT EventLogFormat, 
    OUT EFI_PHYSICAL_ADDRESS *EventLogLocation, 
    OUT EFI_PHYSICAL_ADDRESS *EventLogLastEntry, 
    OUT BOOLEAN *EventLogTruncated);

/*
**==============================================================================
**
** EFI_TCG2_HASH_LOG_EXTEND_EVENT
**
**==============================================================================
*/

#define EFI_TCG2_EVENT_HEADER_VERSION  1
#define EFI_TCG2_EXTEND_ONLY 0x0000000000000001
#define PE_COFF_IMAGE 0x0000000000000010

typedef UINT32 TCG_PCRINDEX;

typedef UINT32 TCG_EVENTTYPE;

typedef struct _EFI_TCG2_EVENT_HEADER 
{
    UINT32 HeaderSize; 
    UINT16 HeaderVersion; 
    TCG_PCRINDEX PCRIndex; 
    TCG_EVENTTYPE EventType; 
} 
PACKED
EFI_TCG2_EVENT_HEADER;

typedef struct _EFI_TCG2_EVENT 
{
    UINT32 Size; 
    EFI_TCG2_EVENT_HEADER Header; 
    UINT8 Event[1]; 
}
PACKED
EFI_TCG2_EVENT;

/* ATTN: experimental */
typedef struct _EFI_COMPACT_HASH_EVENT
{
    UINT32 PCRIndex;
    UINT32 EventType;
    UINT8 Digest[20];
    UINT32 EventDataSize;
    UINT8 Event[1];
}
PACKED
EFI_COMPACT_HASH_EVENT;

typedef EFI_STATUS (EFIAPI *EFI_TCG2_HASH_LOG_EXTEND_EVENT)( 
    IN EFI_TCG2_PROTOCOL *This,
    IN UINT64 Flags,
    IN EFI_PHYSICAL_ADDRESS DataToHash,
    IN UINT64 DataToHashLen,
    IN EFI_TCG2_EVENT *EfiTcgEvent);

/*
**==============================================================================
**
** EFI_TCG2_SUBMIT_COMMAND
**
**==============================================================================
*/

typedef EFI_STATUS (EFIAPI *EFI_TCG2_SUBMIT_COMMAND)(
    IN EFI_TCG2_PROTOCOL *This,
    IN UINT32 InputParameterBlockSize,
    IN UINT8 *InputParameterBlock,
    IN UINT32 OutputParameterBlockSize,
    IN UINT8 *OutputParameterBlock);

/*
**==============================================================================
**
** EFI_TCG2_GET_ACTIVE_PCR_BANKS
**
**==============================================================================
*/

typedef void *EFI_TCG2_GET_ACTIVE_PCR_BANKS;

/*
**==============================================================================
**
** EFI_TCG2_SET_ACTIVE_PCR_BANKS
**
**==============================================================================
*/

typedef void *EFI_TCG2_SET_ACTIVE_PCR_BANKS;

/*
**==============================================================================
**
** EFI_TCG2_GET_RESULT_OF_SET_ACTIVE_PCR_BANKS
**
**==============================================================================
*/

typedef void *EFI_TCG2_GET_RESULT_OF_SET_ACTIVE_PCR_BANKS;

/*
**==============================================================================
**
** EFI_TCG2_PROTOCOL
**
**==============================================================================
*/

struct _EFI_TCG2_PROTOCOL 
{
    EFI_TCG2_GET_CAPABILITY GetCapability;
    EFI_TCG2_GET_EVENT_LOG GetEventLog;
    EFI_TCG2_HASH_LOG_EXTEND_EVENT HashLogExtendEvent;
    EFI_TCG2_SUBMIT_COMMAND SubmitCommand;
    EFI_TCG2_GET_ACTIVE_PCR_BANKS GetActivePcrBanks;
    EFI_TCG2_SET_ACTIVE_PCR_BANKS SetActivePcrBanks;
    EFI_TCG2_GET_RESULT_OF_SET_ACTIVE_PCR_BANKS GetResultOfSetActivePcrBanks;
};

/*
**==============================================================================
**
** TCG2_GetProtocol()
**
**==============================================================================
*/

EFI_TCG2_PROTOCOL *TCG2_GetProtocol();

/*
**==============================================================================
**
** TCG2_ReleaseProtocol()
**
**==============================================================================
*/

void TCG2_ReleaseProtocol(
    EFI_TCG2_PROTOCOL* protocol);

/*
**==============================================================================
**
** TCG2_GetCapability()
**
**==============================================================================
*/

EFI_STATUS TCG2_GetCapability(
    EFI_TCG2_PROTOCOL *protocol,
    EFI_TCG2_BOOT_SERVICE_CAPABILITY* capability);

/*
**==============================================================================
**
** TCG2_SubmitCommand()
**
**==============================================================================
*/

EFI_STATUS TCG2_SubmitCommand(
    EFI_TCG2_PROTOCOL *protocol,
    IN UINT32 InputParameterBlockSize,
    IN UINT8 *InputParameterBlock,
    IN UINT32 OutputParameterBlockSize,
    IN UINT8 *OutputParameterBlock);

/*
**==============================================================================
**
** DumpTCG2Capability()
**
**==============================================================================
*/

void DumpTCG2Capability(
    const EFI_TCG2_BOOT_SERVICE_CAPABILITY* capability);

#endif /* _tcg2_h */

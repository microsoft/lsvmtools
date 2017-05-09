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
#ifndef _lsvmtool_policy_h
#define _lsvmtool_policy_h

#include "config.h"
#include <lsvmutils/eficommon.h>
#include <lsvmutils/tcg2.h>
#include <lsvmutils/tpm2.h>
#include <lsvmutils/error.h>
#include <lsvmutils/sha.h>
#include <lsvmutils/vars.h>

#define POLICY_MAX_ENTRIES 16
#define POLICY_MAX_LINE 1024

typedef enum _PolicyEntryType
{
    POLICY_EFIVAR, /* Hash an EFI variable */
    POLICY_PEIMAGE, /* Hash of a PE/COFF image */
    POLICY_BINARY,  /* Hash of raw binary data */
    POLICY_BINARY32,  /* Hash of raw binary data (32-bit padded) */
    POLICY_CAP, /* Cap a PCR */
    POLICY_PCR, /* Explicitly add PCR to the measurement */
}
PolicyEntryType;

typedef struct _PolicyEntry
{
    /* The path of the file being sealed */
    char path[POLICY_MAX_LINE];

    /* The type of this entry */
    PolicyEntryType type;

    /* The PCR to be (predictively) extended */
    UINT32 pcr;
}
PolicyEntry;

typedef struct _Policy
{
    PolicyEntry entries[POLICY_MAX_ENTRIES];
    UINT32 nentries;
    UINT32 pcrMask;
}
Policy;

typedef struct _PCRBanks
{
    UINT32 pcrMask;
    SHA1Hash pcrs1[MAX_PCRS];
    SHA256Hash pcrs256[MAX_PCRS];
}
PCRBanks;

int LoadPolicy(
    const char* path, 
    Policy* policy,
    Error* err);

void DumpPolicy(
    const Policy* policy);

int MeasurePolicy(
    const Vars* vars,
    EFI_TCG2_PROTOCOL *protocol,
    const Policy* policy,
    BOOLEAN log,
    PCRBanks* banks,
    Error* err);

int SealPolicy(
    const Vars* vars,
    EFI_TCG2_PROTOCOL *protocol,
    Policy* policy,
    BOOLEAN log,
    const BYTE dataIn[TPM2X_MAX_DATA_SIZE],
    UINT16 sizeIn,
    BYTE dataOut[TPM2X_BLOB_SIZE],
    UINTN* sizeOut,
    Error* err);

#endif /* _lsvmtool_policy_h */

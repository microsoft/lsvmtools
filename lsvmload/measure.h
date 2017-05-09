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
#ifndef _measure_h
#define _measure_h

#include "config.h"
#include <lsvmutils/tcg2.h>
#include <lsvmutils/sha.h>
#include <lsvmutils/error.h>
#include <lsvmutils/ext2.h>
#include "log.h"

/* PCR assignments */
#define SHIM_PCR 11
#define GRUB_PCR 11
#define CAPPING_PCR 11
#define SCENARIO_PCR 11

EFI_STATUS Initialize(
    EFI_TCG2_PROTOCOL* tcg2Protocol,
    EFI_HANDLE imageHandle,
    Error* err);

EFI_STATUS MeasureBinary(
    const CHAR16* name,
    const void* data,
    UINTN size,
    SHA256Hash* hash);

EFI_STATUS MeasurePEImage(
    UINT32 pcr,
    const CHAR16* name,
    const char* logDescription,
    const void* data,
    UINTN size,
    SHA1Hash* sha1,
    SHA256Hash* sha256);

EFI_STATUS MeasureHash(
    EFI_TCG2_PROTOCOL *tcg2Protocol,
    EFI_HANDLE imageHandle,
    UINT32 pcrIndex,
    const CHAR16* name,
    const SHA1Hash* sha1,
    const SHA256Hash* sha256,
    OUT Error* err);

EFI_STATUS HashLogExtendData(
    EFI_TCG2_PROTOCOL *tcg2Protocol,
    UINT32 pcr,
    const UINT8* data,
    UINTN size);

EFI_STATUS HashLogExtendSeparator(
    EFI_TCG2_PROTOCOL *tcg2Protocol,
    UINT32 pcr);

EFI_STATUS HashLogExtendPEImage(
    EFI_TCG2_PROTOCOL *tcg2Protocol,
    UINT32 pcr,
    EFI_PHYSICAL_ADDRESS hashData, 
    UINTN hashSize, 
    const char *description);

EFI_STATUS MeasureLinuxScenario(
    EFI_TCG2_PROTOCOL *tcg2Protocol,
    EFI_HANDLE imageHandle);

EFI_STATUS LoadPrefixedFile(
    EFI_HANDLE imageHandle,
    EFI_TCG2_PROTOCOL* tcg2Protocol,
    EXT2* bootfs,
    const CHAR16* path,
    void** data,
    UINTN* size);

#endif /* _measure_h */

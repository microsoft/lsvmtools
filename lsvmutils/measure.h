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

#include <lsvmutils/tpm2.h>
#include <lsvmutils/error.h>
#include <lsvmutils/strarr.h>
#include <lsvmutils/tpmbuf.h>
#include <lsvmutils/vars.h>

#define GUID_STR_SIZE (sizeof(EFI_GUID)*2 + 1)

void DumpSHA1(const SHA1Hash* sha1);

void DumpSHA256(const SHA256Hash* sha256);

int StrToGUID(
    const char* str,
    EFI_GUID* guid);

void GUIDToStr(
    EFI_GUID* guid, 
    char str[GUID_STR_SIZE]);

BYTE* HexStrToBinary(
    const char* hexstr,
    UINT32* size);

int LoadEFIVar(
    const char* guidstr,
    const char* name,
    unsigned char** dataOut,
    UINTN* sizeOut);

int HashVariable(
    IN EFI_GUID *guid,
    IN const char *name,
    IN void *data,
    IN UINTN data_size,
    OUT SHA1Hash* sha1,
    OUT SHA256Hash* sha256);

int Measure(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN BOOLEAN predictive,
    IN UINT32 pcr,
    IN const SHA1Hash* sha1,
    IN const SHA256Hash* sha256,
    IN OUT SHA1Hash* pcr_sha1,
    IN OUT SHA256Hash* pcr_sha256);

int MeasureSeparator(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN BOOLEAN predictive,
    IN UINT32 pcr,
    IN OUT SHA1Hash* pcr_sha1,
    IN OUT SHA256Hash* pcr_sha256,
    OUT SHA1Hash* sha1,
    OUT SHA256Hash* sha256);

int MeasurePEImage(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN BOOLEAN predictive,
    IN UINT32 pcr,
    const char* path,
    IN OUT SHA1Hash* pcr_sha1,
    IN OUT SHA256Hash* pcr_sha256,
    OUT SHA1Hash* sha1,
    OUT SHA256Hash* sha256);

int MeasureBinaryFile(
    const Vars* vars,
    IN EFI_TCG2_PROTOCOL *protocol,
    IN BOOLEAN predictive,
    IN UINT32 pcr,
    const char* initrd,
    IN OUT SHA1Hash* pcr_sha1,
    IN OUT SHA256Hash* pcr_sha256,
    OUT SHA1Hash* sha1,
    OUT SHA256Hash* sha256);

#if 0
int MeasureConfigFile(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN BOOLEAN predictive,
    IN UINT32 pcr,
    const char* path,
    IN OUT SHA1Hash* pcr_sha1,
    IN OUT SHA256Hash* pcr_sha256,
    OUT SHA1Hash* sha1,
    OUT SHA256Hash* sha256);
#endif

int MeasureDirectory(
    IN EFI_TCG2_PROTOCOL *protocol,
    const char* ext2fsPath,
    IN BOOLEAN predictive,
    IN UINT32 pcr,
    const char* path,
    const StrArr* exclusions,
    IN OUT SHA1Hash* pcr_sha1,
    IN OUT SHA256Hash* pcr_sha256,
    OUT SHA1Hash* sha1,
    OUT SHA256Hash* sha256);

int MeasureEFIVariable(
    const Vars* vars,
    IN EFI_TCG2_PROTOCOL *protocol,
    IN BOOLEAN predictive,
    IN UINT32 pcr,
    const char* path,
    IN OUT SHA1Hash* pcr_sha1,
    IN OUT SHA256Hash* pcr_sha256,
    OUT SHA1Hash* sha1,
    OUT SHA256Hash* sha256);

int MeasureCap(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN BOOLEAN predictive,
    IN UINT32 pcr,
    IN OUT SHA1Hash* pcr_sha1,
    IN OUT SHA256Hash* pcr_sha256,
    OUT SHA1Hash* sha1,
    OUT SHA256Hash* sha256);

int MeasureBinaryFilePad32(
    const Vars* vars,
    IN EFI_TCG2_PROTOCOL *protocol,
    IN BOOLEAN predictive,
    IN UINT32 pcr,
    const char* path,
    IN OUT SHA1Hash* pcr_sha1,
    IN OUT SHA256Hash* pcr_sha256,
    OUT SHA1Hash* sha1,
    OUT SHA256Hash* sha256);

#endif /* _measure_h */

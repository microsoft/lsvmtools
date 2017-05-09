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
#ifndef _specialize_h
#define _specialize_h

#include <lsvmutils/eficommon.h>
#include <lsvmutils/tcg2.h>
#include <lsvmutils/blkdev.h>
#include <lsvmutils/ext2.h>

/*
 *  Specialization file structure.
 *  Format is SECURE_SPECIALIZATION_HEADER || (IV | HMAC | Ciphertext)
 */
typedef struct SECURE_SPECIALIZATION_HEADER_V1 {
    UINT32 Length;
    UINT32 Version;
    UINT32 Mode;
    UINT32 IVLength;
    UINT32 IVOffset;
    UINT32 HMACLength;
    UINT32 HMACOffset;
    UINT32 CipherLength;
    UINT32 CipherOffset;
} SECURE_SPECIALIZATION_HEADER_V1, *PSECURE_SPECIALIZATION_HEADER_V1;

#define SECURE_SPECIALIZATION_HEADER_V1_SIZE    (sizeof(SECURE_SPECIALIZATION_HEADER_V1))
#define SECURE_SPECIALIZATION_HEADER_VERSION_1  (0x0001)

#define SECURE_SPECIALIZATION           SECURE_SPECIALIZATION_HEADER_V1
#define PSECURE_SPECIALIZATION          PSECURE_SPECIALIZATION_HEADER_V1
#define SECURE_SPECIALIZATION_SIZE      SECURE_SPECIALIZATION_HEADER_V1_SIZE
#define SECURE_SPECIALIZATION_VERSION   (SECURE_SPECIALIZATION_HEADER_VERSION_1)

#define SECURE_SPECIALIZATION_MODE_START            0
#define SECURE_SPECIALIZATION_MODE_AES_CBC          1
#define SECURE_SPECIALIZATION_MODE_AES_CBC_SHA256   2
#define SECURE_SPECIALIZATION_MODE_END              3

#define IS_SPECIALIZATION_MODE_SUPPORTED(x) \
    ((x) > SECURE_SPECIALIZATION_MODE_START && (x) < SECURE_SPECIALIZATION_MODE_END)

int LoadDecryptCopySpecializeFile(
    EFI_HANDLE imageHandle,
    Blkdev* bootdev,
    EXT2* bootfs,
    const CHAR16* specializePath);

#endif /* _specialize_h */

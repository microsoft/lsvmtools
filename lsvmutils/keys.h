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
#ifndef _utils_keys_h
#define _utils_keys_h

#include "config.h"
#include "eficommon.h"

#define KEYS_BOOTKEY_TYPE 1
#define KEYS_ROOTKEY_TYPE 2

#define KEYS_SEALED_HEADER_SIZE 2
typedef struct _KEYS_SEALED_HEADER {
    UINT16 KeyCount;
} KEYS_SEALED_HEADER;

#define KEYS_SEALED_KEY_HEADER_SIZE 4
typedef struct _KEYS_SEALED_KEY_HEADER {
    UINT16 KeyType;
    UINT16 KeySize;
} KEYS_SEALED_KEY_HEADER;

int SplitKeys(
    const UINT8* inData,
    UINTN inSize,
    UINT8** bootkey,
    UINT8** rootkey,
    UINTN* bootkeySize,
    UINTN* rootkeySize);

int CombineKeys(
    const UINT8* bootkey,
    const UINT8* rootkey,
    UINT16 bootkeySize,
    UINT16 rotokeySize,
    UINT8** outData,
    UINTN* outDataSize);

#endif /* _utils_keys_h */

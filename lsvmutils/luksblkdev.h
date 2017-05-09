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
#ifndef _luksblddev_h
#define _luksblddev_h

#include "config.h"
#include "blkdev.h"

BOOLEAN IsRawLUKSDevice(
    Blkdev* rawdev);

Blkdev* LUKSBlkdevFromMasterkey(
    Blkdev* rawdev,
    const UINT8* masterkey,
    UINT32 masterkeyBytes);

Blkdev* LUKSBlkdevFromRawBytes(
    Blkdev* rawdev,
    const UINT8* passphraseData,
    UINTN passphraseBytes);

Blkdev* LUKSBlkdevFromPassphrase(
    Blkdev* rawdev,
    const char *passphrase);

int LUKSBlkdevGetMasterKey(
    Blkdev* dev,
    const UINT8** masterkeyData,
    UINTN* masterkeyBytes);

#endif /* _luksblddev_h */

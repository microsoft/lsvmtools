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
#ifndef _mbutils_efivarauth_h
#define _mbutils_efivarauth_h

#include "config.h"
#include "eficommon.h"

typedef struct 
{
    UINT32 dwLength;
    UINT16 wRevision;
    UINT16 wCertificateType;
    UINT16 bCertificate[0];
} 
WIN_CERTIFICATE;

typedef struct 
{
    WIN_CERTIFICATE Hdr;
    EFI_GUID CertType;
    UINT8 CertData[0];
} 
WIN_CERTIFICATE_EFI_GUID;

typedef struct 
{
    EFI_TIME TimeStamp;
    WIN_CERTIFICATE_EFI_GUID AuthInfo;
}
EFI_VARIABLE_AUTHENTICATION_2;

#endif /* _mbutils_efivarauth_h */

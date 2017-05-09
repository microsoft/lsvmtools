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
#ifndef _eficompat_h
#define _eficompat_h

#include "config.h"

#ifndef _eficommon_h
# error "do not include 'eficompat.h' directly (include 'eficommon.h')"
#endif

#define EFI_VARIABLE_NON_VOLATILE                          0x00000001
#define EFI_VARIABLE_BOOTSERVICE_ACCESS                    0x00000002
#define EFI_VARIABLE_RUNTIME_ACCESS                        0x00000004
#define EFI_VARIABLE_HARDWARE_ERROR_RECORD                 0x00000008
#define EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS            0x00000010
#define EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS 0x00000020
#define EFI_VARIABLE_APPEND_WRITE                          0x00000040

/*
**==============================================================================
**
** Architecture:
**
**==============================================================================
*/

/* Windows */
#if defined(_WIN32)
# if defined(_WIN64)
#  define __EFI_64_BIT
# else
#  define __EFI_32_BIT
# endif
#endif

/* Linux */
#if defined(__linux__)
# if defined(__i386)
#  define __EFI_32_BIT
# elif defined(__x86_64)
#  define __EFI_64_BIT
# endif
#endif

#if !defined(__EFI_32_BIT) && !defined(__EFI_64_BIT)
# error "unsupported"
#endif

/*
**==============================================================================
**
** Calling conventions:
**
**==============================================================================
*/

#ifdef _MSC_EXTENSIONS
# define EFIAPI __cdecl
#elif defined(HAVE_USE_MS_ABI)
# define EFIAPI __attribute__((ms_abi))
#else
# define EFIAPI
#endif

/*
**==============================================================================
**
** Basic types:
**
**==============================================================================
*/

#define TRUE 1

#define FALSE 0

typedef unsigned char BYTE;

typedef char INT8;

typedef unsigned char UINT8;

typedef short INT16;

typedef unsigned short UINT16;

typedef int INT32;

typedef unsigned int UINT32;

typedef long long INT64;

typedef unsigned long long UINT64;

typedef UINT16 CHAR16;

typedef BYTE BOOLEAN;

#if defined(__EFI_32_BIT)
typedef UINT32 UINTN;
typedef INT32 INTN;
#elif defined(__EFI_64_BIT)
typedef UINT64 UINTN;
typedef INT64 INTN;
#else
# error "unsupported"
#endif

/*
**==============================================================================
**
** Parameter tags:
**
**==============================================================================
*/

#define IN
#define OUT
#define OPTIONAL

/*
**==============================================================================
**
** EFI_HANDLE
**
**==============================================================================
*/

typedef void *EFI_HANDLE;

/*
**==============================================================================
**
** EFI_SYSTEM_TABLE
**
**==============================================================================
*/

typedef long EFI_SYSTEM_TABLE;

/*
**==============================================================================
**
** EFI_LOADED_IMAGE
**
**==============================================================================
*/

typedef struct _EFI_LOADED_IMAGE
{
    void* ImageBase;
    UINTN ImageSize;
    void* LoadOptions;
    UINTN LoadOptionsSize;
}
EFI_LOADED_IMAGE;

/*
**==============================================================================
**
** EFI_STATUS:
**
**==============================================================================
*/

typedef UINTN EFI_STATUS;

#define EFIERR(a) (0x80000000 | a)
#define EFIWARN(a) (a)

#define EFI_SUCCESS 0
#define EFI_LOAD_ERROR EFIERR(1)
#define EFI_INVALID_PARAMETER EFIERR(2)
#define EFI_UNSUPPORTED EFIERR(3)
#define EFI_BAD_BUFFER_SIZE EFIERR(4)
#define EFI_BUFFER_TOO_SMALL EFIERR(5)
#define EFI_NOT_READY EFIERR(6)
#define EFI_DEVICE_ERROR EFIERR(7)
#define EFI_WRITE_PROTECTED EFIERR(8)
#define EFI_OUT_OF_RESOURCES EFIERR(9)
#define EFI_VOLUME_CORRUPTED EFIERR(10)
#define EFI_VOLUME_FULL EFIERR(11)
#define EFI_NO_MEDIA EFIERR(12)
#define EFI_MEDIA_CHANGED EFIERR(13)
#define EFI_NOT_FOUND EFIERR(14)
#define EFI_ACCESS_DENIED EFIERR(15)
#define EFI_NO_RESPONSE EFIERR(16)
#define EFI_NO_MAPPING EFIERR(17)
#define EFI_TIMEOUT EFIERR(18)
#define EFI_NOT_STARTED EFIERR(19)
#define EFI_ALREADY_STARTED EFIERR(20)
#define EFI_ABORTED EFIERR(21)
#define EFI_ICMP_ERROR EFIERR(22)
#define EFI_TFTP_ERROR EFIERR(23)
#define EFI_PROTOCOL_ERROR EFIERR(24)
#define EFI_INCOMPATIBLE_VERSION EFIERR(25)
#define EFI_SECURITY_VIOLATION EFIERR(26)
#define EFI_CRC_ERROR EFIERR(27)
#define EFI_END_OF_MEDIA EFIERR(28)
#define EFI_END_OF_FILE EFIERR(31)
#define EFI_INVALID_LANGUAGE EFIERR(32)
#define EFI_COMPROMISED_DATA EFIERR(33)
#define EFI_WARN_UNKOWN_GLYPH EFIWARN(1)
#define EFI_WARN_DELETE_FAILURE EFIWARN(2)
#define EFI_WARN_WRITE_FAILURE EFIWARN(3)
#define EFI_WARN_BUFFER_TOO_SMALL EFIWARN(4)

/*
**==============================================================================
**
** EFI_GUID
**
**==============================================================================
*/

typedef struct _EFI_GUID
{          
    UINT32 Data1;
    UINT16 Data2;
    UINT16 Data3;
    UINT8 Data4[8]; 
} 
EFI_GUID;

/*
**==============================================================================
**
** EFI_TIME
**
**==============================================================================
*/

typedef struct 
{
    UINT16 Year;
    UINT8 Month;
    UINT8 Day;
    UINT8 Hour;
    UINT8 Minute;
    UINT8 Second;
    UINT8 Pad1;
    UINT32 Nanosecond;
    INT16 TimeZone;
    UINT8 Daylight;
    UINT8 Pad2;
}
EFI_TIME;

#endif /* _eficompat_h */

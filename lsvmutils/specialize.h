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
#ifndef _lsvmutils_specialize_h
#define _lsvmutils_specialize_h

#include <lsvmutils/error.h>

#define SPEC_UNATTEND_FILENAME "unattend.xml"
#define SPEC_SPECIALIZE_FILENAME "specialize"
#define SPEC_WCSTRLEN(x) (sizeof((x))/2 - 1)
/*
 * This is the format of the Ciphertext:
 * SPECIALIZATION_CLEAR_DATA_HEADER || ENTRY1 || (NAME1 | PAYLOAD1) || Entry2 || ... 
 */ 
typedef struct _SPECIALIZATION_CLEAR_DATA_HEADER
{
    UINT32 Length;
    UINT32 FileCount;

} SPECIALIZATION_CLEAR_DATA_HEADER, *PSPECIALIZATION_CLEAR_DATA_HEADER;

#define SPECIALIZATION_CLEAR_DATA_HEADER_SIZE   (sizeof(SPECIALIZATION_CLEAR_DATA_HEADER))

typedef struct _SPECIALIZATION_CLEAR_DATA_FILE_ENTRY
{
    UINT32 FileType;
    UINT32 FileNameSize;
    UINT32 FileNameOffset;
    UINT32 FilePayloadSize;
    UINT32 FilePayloadOffset;

} SPECIALIZATION_CLEAR_DATA_FILE_ENTRY, *PSPECIALIZATION_CLEAR_DATA_FILE_ENTRY;

#define SPECIALIZATION_CLEAR_DATA_FILE_ENTRY_SIZE   (sizeof(SPECIALIZATION_CLEAR_DATA_FILE_ENTRY))

typedef struct _SPECIALIZATION_RESULT
{
    char* FileName;
    UINT8* PayloadData;
    UINT32 PayloadSize; 
} SPECIALIZATION_RESULT, *PSPECIALIZATION_RESULT;

/* Parses the specialization file format and returns the list of files + data in the
 * SPECIALIZATION_RESULT format.. */
int ExtractSpecFiles(
    const UINT8* data,
    UINTN size,
    SPECIALIZATION_RESULT** result,
    UINTN* resultSize
    );

void FreeSpecFiles(
    SPECIALIZATION_RESULT* r,
    UINTN rSize);

#endif /* _lsvmutils_specialize_h */

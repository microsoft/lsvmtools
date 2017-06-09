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
#include "config.h"
#include "eficommon.h"
#include "specialize.h"
#include "alloc.h"
#include "strings.h"

static BOOLEAN _Equals(CHAR16* p1, char* p2, UINTN s1, UINTN s2)
{
    UINTN i;
    if (s1 != s2)
    {
        return FALSE;
    }
    for (i = 0; i < s1; i++)
    {
        if (p1[i] != (CHAR16) p2[i])
        {
            return FALSE;
        }
    }
    return TRUE;
}

static int _WriteSingleSpecFile(
    const UINT8* dataCur,
    SPECIALIZATION_CLEAR_DATA_FILE_ENTRY* entry,
    SPECIALIZATION_RESULT* result)
{
    /* FileNameSize does not have null terminator. */
    UINTN i = 0;
    CHAR16* fileName = NULL;
    CHAR16* fileNameTmp = NULL;
    UINT32 fileNameSize = 0;
    UINT32 resultFileNameSize = 0;
    int rc = -1;

    result->FileName = NULL;
    result->PayloadData = NULL;

    fileName = (CHAR16*) (dataCur + entry->FileNameOffset);
    fileNameSize = entry->FileNameSize;

    /* Adjust the some special files to the right name. */
    if (_Equals(fileName,
                SPEC_UNATTEND_FILENAME,
                fileNameSize / 2,
                sizeof(SPEC_UNATTEND_FILENAME) - 1))
    {
        fileNameTmp = WcsStrdup(SPEC_SPECIALIZE_FILENAME);
        if (fileNameTmp == NULL)
        {
            goto CleanupErr;
        }
        fileName = fileNameTmp;
        fileNameSize = (sizeof(SPEC_SPECIALIZE_FILENAME) - 1) * sizeof(CHAR16);
    }
    
    resultFileNameSize = fileNameSize / sizeof(CHAR16) + 1;
    result->FileName = (char *) Malloc(resultFileNameSize);
    if (result->FileName == NULL)
    {
        goto CleanupErr;
    }

    result->PayloadData = (UINT8*) Malloc(entry->FilePayloadSize);
    if (result->PayloadData == NULL)
    {
        goto CleanupErr;
    }
 
    for (i = 0; i < resultFileNameSize; i++)
    {
        result->FileName[i] = (char) fileName[i];
    }
    fileName[resultFileNameSize] = 0;
    
    Memcpy(result->PayloadData, dataCur + entry->FilePayloadOffset, entry->FilePayloadSize);
    result->PayloadSize = entry->FilePayloadSize;
    rc = 0;
    goto Cleanup;

CleanupErr:
    if (result->FileName != NULL)
    {
        Free(result->FileName);
    }

    if (result->PayloadData != NULL)
    {
        Free(result->PayloadData);
    }

Cleanup:
    if (fileNameTmp != NULL)
    {
        Free(fileNameTmp);
    }
    return -rc;
}

static void _CleanupSpecFilesList(
    SPECIALIZATION_RESULT* result,
    UINTN resultSize)
{
    UINTN i = 0;
    for (i = 0; i < resultSize; i++)
    {
        Free(result->FileName);
        Free(result->PayloadData);
    }
}


static int _WriteSpecResults(
    SPECIALIZATION_CLEAR_DATA_HEADER* hdr,
    const UINT8* data,
    UINTN size,
    SPECIALIZATION_RESULT* result,
    UINTN resultSize)
{
    const UINT8* dataCur = data;
    UINT32 i = 0;

    if (resultSize != hdr->FileCount)
    {
        goto Cleanup;
    }

    for (i = 0; i < hdr->FileCount; i++)
    {
        SPECIALIZATION_CLEAR_DATA_FILE_ENTRY* entry;
        UINT32 max;

        /* First validate inputs. */
        if (dataCur + SPECIALIZATION_CLEAR_DATA_FILE_ENTRY_SIZE > data + size)
        {
            goto Cleanup;
        }

        entry = (SPECIALIZATION_CLEAR_DATA_FILE_ENTRY*) dataCur; 
        if (dataCur + entry->FileNameOffset + entry->FileNameSize > data + size ||
            dataCur + entry->FilePayloadOffset + entry->FilePayloadSize > data + size ||
            entry->FileNameSize % 2 != 0)
        {
            goto Cleanup;
        }

        if (_WriteSingleSpecFile(dataCur, entry, &result[i]) != 0)
        {
            goto Cleanup;
        }

        /* Advance to next entry. */
        max = entry->FilePayloadOffset + entry->FilePayloadSize;
        if (max < entry->FileNameOffset + entry->FileNameSize)
        {
            max = entry->FileNameOffset + entry->FileNameSize;
        }
        dataCur += max;
    }
    return 0;

Cleanup:
    /* We allocated some memory from _WriteSingleSpecFile that we should clean up. */
    _CleanupSpecFilesList(result, i);
    return -1;
}

int ExtractSpecFiles(
    const UINT8* data,
    UINTN size,
    SPECIALIZATION_RESULT** result,
    UINTN* resultSize)
{
    SPECIALIZATION_CLEAR_DATA_HEADER* hdr;
    SPECIALIZATION_RESULT* resultLocal = NULL;
    int rc = -1;

     /* Check size of data */
    if (size < sizeof(SPECIALIZATION_CLEAR_DATA_HEADER))
    {
        goto Cleanup;
    }

    /* Now parse the spec files. */
    hdr = (SPECIALIZATION_CLEAR_DATA_HEADER*) data;
    data += sizeof(*hdr);
    size -=  sizeof(*hdr);

    resultLocal = (SPECIALIZATION_RESULT*) Malloc(hdr->FileCount * sizeof(SPECIALIZATION_RESULT));
    if (resultLocal == NULL)
    {
        goto Cleanup;
    }

    if (_WriteSpecResults(hdr, data, size, resultLocal, hdr->FileCount) != 0)
    {
        goto Cleanup;
    }

    *result = resultLocal;
    *resultSize = hdr->FileCount;
    return 0;

Cleanup:
    if (resultLocal != NULL)
    {
        Free(resultLocal);
    }
    return rc;
}

void FreeSpecFiles(
    SPECIALIZATION_RESULT* r,
    UINTN rSize)
{
    _CleanupSpecFilesList(r, rSize);
    Free(r);
}

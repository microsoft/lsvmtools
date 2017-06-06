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

static int _GetSpecFile(
    SPECIALIZATION_CLEAR_DATA_HEADER* hdr,
    const UINT8* data,
    UINTN size,
    const UINT8** out,
    UINTN* outSize)
{
    const UINT8* dataCur;
    UINT32 i;

    dataCur = data;
    for (i = 0; i < hdr->FileCount; i++)
    {
        SPECIALIZATION_CLEAR_DATA_FILE_ENTRY* entry;
        UINT32 max;
        BOOLEAN isSpecFile = FALSE;
	
        /* First validate inputs. */
        if (dataCur + SPECIALIZATION_CLEAR_DATA_FILE_ENTRY_SIZE > data + size)
        {
            return -1;
        }

        /* Right now, we only check the unattend or autounattend file. */
        entry = (SPECIALIZATION_CLEAR_DATA_FILE_ENTRY*) dataCur; 

        isSpecFile = Memcmp(dataCur + entry->FileNameOffset,
                            SPEC_UNATTEND_FILENAME,
                            sizeof(SPEC_UNATTEND_FILENAME) - sizeof(CHAR16));

        isSpecFile = isSpecFile ||
                     Memcmp(dataCur + entry->FileNameOffset,
                            SPEC_UNATTEND_FILENAME_ALTERNATE,
                            sizeof(SPEC_UNATTEND_FILENAME_ALTERNATE) - sizeof(CHAR16));

        /* Found the specialization file. */
        if (isSpecFile)
        {
            *out = dataCur + entry->FilePayloadOffset;
            *outSize = entry->FilePayloadSize;
            return 0;
        }

        /* Advance to next entry. */
        max = entry->FilePayloadOffset + entry->FilePayloadSize;
        if (max < entry->FileNameOffset + entry->FileNameSize)
        {
            max = entry->FileNameOffset + entry->FileNameSize;
        }
        dataCur += max;
    }
    /* Spec file not found, so return an error. */
    return -1;
}

int FindSpecFile(
    const UINT8* data,
    UINTN size,
    const UINT8** out,
    UINTN* outSize)
{
    SPECIALIZATION_CLEAR_DATA_HEADER* hdr;
    int rc = -1;

     /* Check size of data */
    if (size < sizeof(SPECIALIZATION_CLEAR_DATA_HEADER))
    {
        goto done;
    }

    /* Now parse the spec files. */
    hdr = (SPECIALIZATION_CLEAR_DATA_HEADER*) data;
    data += sizeof(*hdr);
    size -=  sizeof(*hdr);
    if (_GetSpecFile(hdr, data, size, out, outSize) != 0)
    {
        goto done;
    }

    rc = 0;

done:
    return rc;
}

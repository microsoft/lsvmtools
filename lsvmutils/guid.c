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
#include "guid.h"
#include "dump.h"

void MakeGUID(EFI_GUID* guid, UINT64 x, UINT64 y)
{
    /* 84B9702C-702C-702C-A6A6-A6A6A6A6A6A6 */
    guid->Data1 = x & 0x00000000FFFFFFFF;
    guid->Data2 = (x & 0x0000FFFF00000000) >> 32;
    guid->Data3 = (x & 0xFFFF000000000000) >> 48;
    guid->Data4[0] = y & 0x00000000000000FF;
    guid->Data4[1] = (y & 0x000000000000FF00) >> 8;
    guid->Data4[2] = (y & 0x0000000000FF0000) >> 16;
    guid->Data4[3] = (y & 0x00000000FF000000) >> 24;
    guid->Data4[4] = (y & 0x000000FF00000000) >> 32;
    guid->Data4[5] = (y & 0x0000FF0000000000) >> 40;
    guid->Data4[6] = (y & 0x00FF000000000000) >> 48;
    guid->Data4[7] = (y & 0xFF00000000000000) >> 56;
}

void SplitGUID(const EFI_GUID* guid, UINT64* x, UINT64* y)
{
    /* 84B9702C-702C-702C-A6A6-A6A6A6A6A6A6 */

    *x = 
        ((UINT64)guid->Data1) |
        (((UINT64)guid->Data2) << 32) |
        (((UINT64)guid->Data3) << 48);

    *y = 
        ((UINT64)guid->Data4[0]) |
        (((UINT64)guid->Data4[1]) << 8) |
        (((UINT64)guid->Data4[2]) << 16) |
        (((UINT64)guid->Data4[3]) << 24) |
        (((UINT64)guid->Data4[4]) << 32) |
        (((UINT64)guid->Data4[5]) << 40) |
        (((UINT64)guid->Data4[6]) << 48) |
        (((UINT64)guid->Data4[7]) << 56);
}

void MakeGUIDFromBytes(EFI_GUID* guid, const UINT8 x[16])
{
    Memset(guid, 0, sizeof(EFI_GUID));

    /* 84B9702C-702C-702C-A6A6-A6A6A6A6A6A6 */
    guid->Data1 = 
        ((UINT32)x[0]) << 24 | 
        ((UINT32)x[1]) << 16 | 
        ((UINT32)x[2]) << 8 | 
        ((UINT32)x[3]);

    guid->Data2 = ((UINT32)x[4]) << 8 | ((UINT32)x[5]);
    guid->Data3 = ((UINT32)x[6]) << 8 | ((UINT32)x[7]);

    guid->Data4[0] = x[8];
    guid->Data4[1] = x[9];
    guid->Data4[2] = x[10];
    guid->Data4[3] = x[11];
    guid->Data4[4] = x[12];
    guid->Data4[5] = x[13];
    guid->Data4[6] = x[14];
    guid->Data4[7] = x[15];
}

BOOLEAN ValidGUIDStr(
    const char* str)
{
    UINTN i;

    /* Example: 9f2d4c84-a449-4b54-91cd-003b58397b56 */

    if (Strlen(str) != GUID_STRING_SIZE - 1)
        return FALSE;

    for (i = 0; i < GUID_STRING_SIZE - 1; i++)
    {
        char c = str[i];

        if (i == 8 || i == 13 || i == 18 || i == 23)
        {
            if (c != '-')
                return FALSE;
        }
        else
        {
            if (c != tolower(c))
                return FALSE;

            if (!isxdigit(c))
                return FALSE;
        }
    }

    return TRUE;
}

void FormatGUID(
    TCHAR buf[GUID_STRING_SIZE],
    const EFI_GUID* guid)
{
    UINTN n = 0;
    UINTN m = 0;
    UINTN i;

    n += FormatHexByte(&buf[n], (guid->Data1 & 0xFF000000) >> 24);
    n += FormatHexByte(&buf[n], (guid->Data1 & 0x00FF0000) >> 16);
    n += FormatHexByte(&buf[n], (guid->Data1 & 0x0000FF00) >> 8);
    n += FormatHexByte(&buf[n], guid->Data1 & 0x000000FF);
    buf[n++] = '-';

    n += FormatHexByte(&buf[n], (guid->Data2 & 0xFF00) >> 8);
    n += FormatHexByte(&buf[n], guid->Data2 & 0x00FF);
    buf[n++] = '-';

    n += FormatHexByte(&buf[n], (guid->Data3 & 0xFF00) >> 8);
    n += FormatHexByte(&buf[n], guid->Data3 & 0x00FF);
    buf[n++] = '-';

    n += FormatHexByte(&buf[n], guid->Data4[m++]);
    n += FormatHexByte(&buf[n], guid->Data4[m++]);
    buf[n++] = '-';

    n += FormatHexByte(&buf[n], guid->Data4[m++]);
    n += FormatHexByte(&buf[n], guid->Data4[m++]);
    n += FormatHexByte(&buf[n], guid->Data4[m++]);
    n += FormatHexByte(&buf[n], guid->Data4[m++]);
    n += FormatHexByte(&buf[n], guid->Data4[m++]);
    n += FormatHexByte(&buf[n], guid->Data4[m++]);
    buf[n++] = '\0';

    for (i = 0; i < n; i++)
        buf[i] = Tolower(buf[i]);
}

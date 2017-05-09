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
#include "dump.h"
#include "print.h"
#include "strings.h"

static const char _hexchar[] =
{
    '0', '1', '2', '3', '4', '5', '6', '7', 
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

UINTN FormatHexByte(TCHAR buf[3], UINT8 x)
{
    buf[0] = _hexchar[(0xF0 & x) >> 4];
    buf[1] = _hexchar[0x0F & x];
    buf[2] = '\0';
    return 2;
}

void HexDump(
    const void* data_,
    UINT32 size)
{
    UINT32 i;
    const UINT8* data = (const UINT8*)data_;

    Print(TCS("TPM2_HexDump: %d bytes\n"), size);
    Print(TCS("00000000 "));

    for (i = 0; i < size; i++)
    {
        unsigned char c = data[i];
        TCHAR buf[3];

        FormatHexByte(buf, c);
        Print(TCS("%s "), buf);

        if ((i + 1) % 16 == 0)
        {
            Print(TCS("\n"));
            Print(TCS("%08d "), i+1);
        }
    }

    Print(TCS("\n"));
}

void ASCIIDump(
    const void* data_,
    UINT32 size)
{
    UINT32 i;
    const UINT8* data = (const UINT8*)data_;

    Print(TCS("TPM2_ASCIIDump: %d bytes\n"), (int)size);

    for (i = 0; i < size; i++)
    {
        unsigned char c = data[i];
        TCHAR buf[3];

        if (c >= ' ' && c <= '~')
            Print(TCS("%c"), c);
        else
        {
            FormatHexByte(buf, c);
            Print(TCS("<%s>"), buf);
        }

        if (i + 1 != size && !((i + 1) % 80))
            Print(TCS("\n"));
    }

    Print(TCS("\n"));
}

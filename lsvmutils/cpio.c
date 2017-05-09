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
#include "cpio.h"
#include <lsvmutils/print.h>
#include <lsvmutils/alloc.h>
#include <lsvmutils/strings.h>
#if !defined(BUILD_EFI)
#include <lsvmutils/file.h>
#endif /* !defined(BUILD_EFI) */
#include "print.h"

#define CPIO_BLOCK_SIZE ((UINTN)512)

/* This is an empty CPIO archive (with just a trailer) */
static unsigned char _empty_archive[] =
{
    0x30, 0x37, 0x30, 0x37, 0x30, 0x31, 0x30, 0x30, 
    0x32, 0x43, 0x30, 0x43, 0x42, 0x38, 0x30, 0x30, 
    0x30, 0x30, 0x34, 0x31, 0x45, 0x44, 0x30, 0x30, 
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 
    0x30, 0x30, 0x30, 0x30, 0x30, 0x32, 0x35, 0x37, 
    0x45, 0x30, 0x33, 0x36, 0x34, 0x43, 0x30, 0x30, 
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 
    0x30, 0x30, 0x30, 0x30, 0x46, 0x43, 0x30, 0x30, 
    0x30, 0x30, 0x30, 0x30, 0x30, 0x32, 0x30, 0x30, 
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 
    0x30, 0x30, 0x30, 0x30, 0x30, 0x32, 0x30, 0x30, 
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x2E, 0x00, 
    0x30, 0x37, 0x30, 0x37, 0x30, 0x31, 0x30, 0x30, 
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 
    0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x30, 0x30, 
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 
    0x30, 0x30, 0x30, 0x30, 0x30, 0x42, 0x30, 0x30, 
    0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x54, 0x52, 
    0x41, 0x49, 0x4C, 0x45, 0x52, 0x21, 0x21, 0x21, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

unsigned int _empty_archive_size = ARRSIZE(_empty_archive);

static void _DumpHeader(const CPIOHeader* self)
{
    PRINTF0("=== CPIOHeader\n");
    PRINTF("magic{%.*a}\n", (int)sizeof(self->magic), self->magic);
    PRINTF("ino{%.*a}\n", (int)sizeof(self->ino), self->ino);
    PRINTF("mode{%.*a}\n", (int)sizeof(self->mode), self->mode);
    PRINTF("uid{%.*a}\n", (int)sizeof(self->uid), self->uid);
    PRINTF("gid{%.*a}\n", (int)sizeof(self->gid), self->gid);
    PRINTF("nlink{%.*a}\n", (int)sizeof(self->nlink), self->nlink);
    PRINTF("mtime{%.*a}\n", (int)sizeof(self->mtime), self->mtime);
    PRINTF("filesize{%.*a}\n", (int)sizeof(self->filesize), self->filesize);
    PRINTF("devmajor{%.*a}\n", (int)sizeof(self->devmajor), self->devmajor);
    PRINTF("devminor{%.*a}\n", (int)sizeof(self->devminor), self->devminor);
    PRINTF("rdevmajor{%.*a}\n", (int)sizeof(self->rdevmajor), self->rdevmajor);
    PRINTF("rdevminor{%.*a}\n", (int)sizeof(self->rdevminor), self->rdevminor);
    PRINTF("namesize{%.*a}\n", (int)sizeof(self->namesize), self->namesize);
    PRINTF("check{%.*a}\n", (int)sizeof(self->check), self->check);
}

static int _HexToInt(
    const char* str,
    unsigned int len)
{
    const char* p;
    int r = 1;
    int x = 0;

    for (p = str + len; p != str; p--)
    {
        int xdigit = p[-1];
        int d;

        if (xdigit >= '0' && xdigit <= '9')
        {
            d = xdigit - '0';
        }
        else if (xdigit >= 'A' && xdigit <= 'F')
        {
            d = (xdigit - 'A') + 10;
        }
        else
            return -1;

        x += r * d;
        r *= 16;
    }

    return x;
}

static char _HexDigit(
    unsigned int x)
{
    switch (x)
    {
        case 0x0: return '0';
        case 0x1: return '1';
        case 0x2: return '2';
        case 0x3: return '3';
        case 0x4: return '4';
        case 0x5: return '5';
        case 0x6: return '6';
        case 0x7: return '7';
        case 0x8: return '8';
        case 0x9: return '9';
        case 0xA: return 'A';
        case 0xB: return 'B';
        case 0xC: return 'C';
        case 0xD: return 'D';
        case 0xE: return 'E';
        case 0xF: return 'F';
    }

    return '\0';
}

static void _IntToHex(
    char buf[8],
    unsigned int x)
{
    buf[0] = _HexDigit((x & 0xF0000000) >> 28);
    buf[1] = _HexDigit((x & 0x0F000000) >> 24);
    buf[2] = _HexDigit((x & 0x00F00000) >> 20);
    buf[3] = _HexDigit((x & 0x000F0000) >> 16);
    buf[4] = _HexDigit((x & 0x0000F000) >> 12);
    buf[5] = _HexDigit((x & 0x00000F00) >> 8);
    buf[6] = _HexDigit((x & 0x000000F0) >> 4);
    buf[7] = _HexDigit((x & 0x0000000F) >> 0);
}

static int _RoundUpToMultiple(UINTN x, UINTN m)
{
    return (int)((x + (m - 1)) / m * m);
}

int CPIOMatchMagicNumber(const char* s)
{
    return 
        s[0] == '0' && s[1] == '7' && s[2] == '0' &&
        s[3] == '7' && s[4] == '0' && s[5] == '1';
}

int CPIOTest(const void* data, UINTN size)
{
    int rc = -1;

    if (size < sizeof(CPIOHeader))
        goto done;

    if (!CPIOMatchMagicNumber(((const CPIOHeader*)data)->magic))
        goto done;

    rc = 0;

done:
    return rc;
}

static int _GetFileMode(const CPIOHeader* header)
{
    return _HexToInt(header->mode, 8);
}

static int _GetFileSize(const CPIOHeader* header)
{
    return _HexToInt(header->filesize, 8);
}

static int _GetNameSize(const CPIOHeader* header)
{
    return _HexToInt(header->namesize, 8);
}

static const char* _GetName(const CPIOHeader* header)
{
    return (char*)(header + 1);
}

/* Get total size of an entry: HEADER + NAME + DATA + PADDING */
int CPIOGetEntrySize(const CPIOHeader* header)
{
    int filesize;
    int namesize;

    if ((filesize = _GetFileSize(header)) == -1)
        return -1;

    if ((namesize = _GetNameSize(header)) == -1)
        return -1;

    return 
        _RoundUpToMultiple(sizeof(CPIOHeader) + namesize, 4) +
        _RoundUpToMultiple(filesize, 4);
}

static UINTN _ComputeHeaderSize(
    const char* path,
    UINTN fileSize)
{
    UINTN size = 0;
    
    size += sizeof(CPIOHeader);
    size += strlen(path) + 1;
    size = _RoundUpToMultiple(size, 4);
    size += fileSize;
    size = _RoundUpToMultiple(size, 4);

    return size;
}

static UINTN _ComputeTrailerSize()
{
    UINTN size = 0;
    
    size += sizeof(CPIOHeader);
    size += sizeof("TRAILER!!!");
    size = _RoundUpToMultiple(size, 4);

    return size;
}

int CPIOCheckEntry(
    const CPIOHeader* header,
    const void* cpioEnd)
{
    int rc = 0;
    int remaining;
    int size;

    /* Calculate the remaining space */
    remaining = (int)((char*)cpioEnd - (char*)header);

    /* If not enough space left for another header */
    if (sizeof(CPIOHeader) > remaining)
    {
        rc = -1;
        goto done;
    }

    /* Check magic number */
    if (!CPIOMatchMagicNumber(header->magic))
    {
        rc = -1;
        goto done;
    }

    /* Get total size of this entry */
    if ((size = CPIOGetEntrySize(header)) == -1)
    {
        rc = -1;
        goto done;
    }

    /* If not enough space left */
    if (size > remaining)
    {
        rc = -1;
        goto done;
    }

done:
    return rc;
}

int CPIONextHeader(
    const CPIOHeader** header,
    const void* cpioEnd)
{
    int rc = 0;
    int size;
    const char* name;

    /* Check parameters */
    if (!header || !*header)
    {
        rc = -1;
        goto done;
    }

    /* Check for valid entry */
    if (CPIOCheckEntry(*header, cpioEnd) != 0)
    {
        *header = NULL;
        rc = -1;
        goto done;
    }

    /* Get the name */
    name = (const char*)(*header + 1);

    /* If this is the trailer, then no more headers */
    if (strcmp(name, "TRAILER!!!") == 0)
    {
        *header = NULL;
        rc = 0;
        goto done;
    }

    /* Get total size of this entry */
    if ((size = CPIOGetEntrySize(*header)) == -1)
    {
        rc = -1;
        goto done;
    }

    /* Get the next header */
    *header = (CPIOHeader*)((char*)(*header) + size);

    /* Check for valid entry */
    if (CPIOCheckEntry(*header, cpioEnd) != 0)
    {
        *header = NULL;
        rc = -1;
        goto done;
    }

done:
    return rc;
}

static int _FindHeader(
    const void* cpioData,
    UINTN cpioSize,
    const char* path,
    const CPIOHeader** headerOut)
{
    int rc = -1;
    const CPIOHeader* header = (const CPIOHeader*)cpioData;
    const void* cpioEnd = (char*)cpioData + cpioSize;

    *headerOut = NULL;

    while (header)
    {
        if (CPIOCheckEntry(header, cpioEnd) != 0)
            goto done;

        if (strcmp(_GetName(header), path) == 0)
        {
            *headerOut = header;
            rc = 0;
            goto done;
        }

        if (CPIONextHeader(&header, cpioEnd) != 0)
            goto done;
    }

done:
    return rc;
}

int CPIONew(
    void** data,
    UINTN* size)
{
    int rc = -1;

    if (!data || !size)
        return -1;

    if (!(*data = Malloc(_empty_archive_size)))
        goto done;

    Memcpy(*data, _empty_archive, _empty_archive_size);
    *size = _empty_archive_size;

    rc = 0;

done:
    return rc;
}

int CPIOAddFile(
    const void* cpioDataIn,
    UINTN cpioSizeIn,
    const char* path,
    const void* fileData,
    UINTN fileSize,
    unsigned int mode,
    void** cpioDataOut,
    UINTN* cpioSizeOut)
{
    int rc = -1;
    const CPIOHeader* trailer = NULL;
    CPIOHeader newHeader;

    /* Check parameters */
    if (!cpioDataIn || !cpioSizeIn || !path || !cpioDataOut || !cpioSizeOut)
        goto done;

    /* Initialize output parameters */
    *cpioDataOut = NULL;
    *cpioSizeOut = 0;

    /* Find the CPIO trailer */
    if (_FindHeader(cpioDataIn, cpioSizeIn, "TRAILER!!!", &trailer) != 0)
        goto done;

    /* Compute the size of the new CPIO archive */
    *cpioSizeOut = (char*)trailer - (char*)cpioDataIn;
    *cpioSizeOut += _ComputeHeaderSize(path, fileSize);
    *cpioSizeOut += _ComputeTrailerSize();
    *cpioSizeOut = _RoundUpToMultiple(*cpioSizeOut, CPIO_BLOCK_SIZE);

    /* Initialize the header for the new file */
    newHeader = *trailer;
    _IntToHex(newHeader.ino, 0x12345678);
    _IntToHex(newHeader.mode, mode);
    _IntToHex(newHeader.uid, 0);
    _IntToHex(newHeader.gid, 0);
    _IntToHex(newHeader.nlink, 1);
    _IntToHex(newHeader.mtime, 0x56734BA4); /* hardcode a time */
    _IntToHex(newHeader.filesize, (unsigned int)fileSize);
    _IntToHex(newHeader.devmajor, 8);
    _IntToHex(newHeader.devminor, 2);
    _IntToHex(newHeader.rdevmajor, 0);
    _IntToHex(newHeader.rdevminor, 0);
    _IntToHex(newHeader.namesize, (unsigned int)(strlen(path) + 1));
    _IntToHex(newHeader.check, 0);

#if 0
    _DumpHeader(&newHeader);
#endif

    /* Allocate the memory for the new CPIO archive */
    if (!(*cpioDataOut = Calloc(1, *cpioSizeOut)))
        goto done;

    /* Copy over old CPIO archive to new CPIO archive (exclude trailer) */
    Memcpy(*cpioDataOut, cpioDataIn, (char*)trailer - (char*)cpioDataIn);

    /* Append new file and trailer */
    {
        UINTN offset = (char*)trailer - (char*)cpioDataIn;
        char* p = (char*)*cpioDataOut;

        /* New file header */
        Memcpy(p + offset, &newHeader, sizeof(CPIOHeader));
        offset += sizeof(CPIOHeader);

        /* New file name (pad to 4-byte boundary) */
        Memcpy(p + offset, path, strlen(path) + 1);
        offset += strlen(path) + 1;
        offset = _RoundUpToMultiple(offset, 4);

        /* New file data (pad to 4-byte boundary) */
        if (fileData)
        {
            Memcpy(p + offset, fileData, fileSize);
            offset += fileSize;
        }

        offset = _RoundUpToMultiple(offset, 4);

        /* Trailer header */
        Memcpy(p + offset, trailer, sizeof(CPIOHeader));
        offset += sizeof(CPIOHeader);

        /* New file name */
        Memcpy(p + offset, "TRAILER!!!", sizeof("TRAILER!!!"));
        offset += sizeof("TRAILER!!!");

        /* Pad to 4-byte boundary */
        offset = _RoundUpToMultiple(offset, 4);

        /* Pad to block size */
        offset = _RoundUpToMultiple(offset, CPIO_BLOCK_SIZE);

        /* Sanity check the resulting size */
        if (offset != *cpioSizeOut)
            goto done;
    }

    rc = 0;

done:

    if (rc != 0)
    {
        if (cpioDataOut && *cpioDataOut)
        {
            Free(*cpioDataOut);
            *cpioDataOut = NULL;
            *cpioSizeOut = 0;
        }
    }

    return rc;
}

int CPIORemoveFile(
    const void* cpioDataIn,
    UINTN cpioSizeIn,
    const char* path,
    void** cpioDataOut,
    UINTN* cpioSizeOut)
{
    int rc = -1;
    const CPIOHeader* header = NULL;
    int offset = 0;

    /* Check parameters */
    if (!cpioDataIn || !cpioSizeIn || !path || !cpioDataOut || !cpioSizeOut)
        goto done;

    /* Initialize output parameters */
    *cpioDataOut = NULL;
    *cpioSizeOut = 0;

    /* Find the CPIO header for this path */
    if (_FindHeader(cpioDataIn, cpioSizeIn, path, &header) != 0)
        goto done;

    *cpioSizeOut = 0;

    /* Allocate the memory for the new CPIO archive */
    if (!(*cpioDataOut = Calloc(1, cpioSizeIn)))
        goto done;

    /* Copy over each header (skipping the one that matches 'path') */
    {
        header = (const CPIOHeader*)cpioDataIn;
        const void* cpioEnd = (unsigned char*)cpioDataIn + cpioSizeIn;
        int found = 0;

        while (header)
        {
            if (CPIOCheckEntry(header, cpioEnd) != 0)
                goto done;

            if (strcmp(_GetName(header), path) == 0)
            {
                /* Skip this one */
                found = 1;
            }
            else
            {
                int size = CPIOGetEntrySize(header);
                Memcpy((unsigned char*)*cpioDataOut + offset, header, size);
                offset += size;
            }

            if (CPIONextHeader(&header, cpioEnd) != 0)
                goto done;
        }

        if (!found)
            goto done;
    }

    /* Calculate the size of the output CPIO buffer */
    {
        offset = _RoundUpToMultiple(offset, CPIO_BLOCK_SIZE);
        *cpioSizeOut = offset;
    }

    rc = 0;

done:

    if (rc != 0)
    {
        if (cpioDataOut && *cpioDataOut)
        {
            Free(*cpioDataOut);
            *cpioDataOut = NULL;
            *cpioSizeOut = 0;
        }
    }

    return rc;
}

int CPIOGetFile(
    const void* cpioData,
    UINTN cpioSize,
    const char* src,
    void** destData,
    UINTN* destSize)
{
    int rc = -1;
    const CPIOHeader* header = NULL;
    UINTN headerSize;
    void* fileData = NULL;
    UINTN fileSize;

    /* Check parameters */
    if (!cpioData || !cpioSize || !src || !destData || !destSize)
        goto done;

    /* Initialize output parameters */
    *destData = NULL;
    *destSize = 0;

    /* Find the CPIO header for this path */
    if (_FindHeader(cpioData, cpioSize, src, &header) != 0)
        goto done;

    /* Determine size of this file */
    fileSize = _GetFileSize(header);

    /* Allocate the memory for this file */
    if (fileSize && !(fileData = Malloc(fileSize)))
        goto done;

    /* Compute the full full size of header (including path) */
    headerSize = sizeof(CPIOHeader);
    headerSize += _GetNameSize(header);
    headerSize = _RoundUpToMultiple(headerSize, 4);

    /* Copy the file to output buffer */
    Memcpy(fileData, (unsigned char*)header + headerSize, fileSize);

    /* Set output parameters */
    *destData = fileData;
    *destSize = fileSize;

    rc = 0;

done:

    if (rc != 0 && fileData)
        Free(fileData);

    return rc;
}

int CPIODumpFile(
    const void* cpioData,
    UINTN cpioSize)
{
    int rc = -1;

    /* Check parameters */
    if (!cpioData)
        goto done;

    /* Is data too small to contain at least one header? */
    if (cpioSize < sizeof(CPIOHeader))
        goto done;

    /* Dump the file */
    {
        const CPIOHeader* header = (const CPIOHeader*)cpioData;
        const void* cpioEnd = (char*)cpioData + cpioSize;

        while (header)
        {
            if (CPIOCheckEntry(header, cpioEnd) != 0)
                goto done;

            _DumpHeader(header);
            PRINTF("name{%a}\n", _GetName(header));

            if (CPIONextHeader(&header, cpioEnd) != 0)
                goto done;
        }
    }

    rc = 0;

done:

    return rc;
}

int CPIOCheckFile(
    const void* cpioData,
    UINTN cpioSize,
    const char* path)
{
    int rc = -1;

    /* Check parameters */
    if (!cpioData || !path)
        goto done;

    /* Is data too small to contain at least one header? */
    if (cpioSize < sizeof(CPIOHeader))
        goto done;

    /* Check the file */
    {
        const CPIOHeader* header = (const CPIOHeader*)cpioData;
        const void* cpioEnd = (char*)cpioData + cpioSize;

        while (header)
        {
            if (CPIOCheckEntry(header, cpioEnd) != 0)
                goto done;

            if (strcmp(_GetName(header), path) == 0)
                goto done;  

            if (CPIONextHeader(&header, cpioEnd) != 0)
                goto done;
        }
    }

    rc = 0;

done:

    return rc;
}

int CPIOSplitFile(
    const void* cpioData,
    UINTN cpioSize,
    const void* subfilesData[CPIO_MAX_SUBFILES],
    UINTN subfilesSize[CPIO_MAX_SUBFILES],
    UINTN* numSubfiles)
{
    int rc = -1;

    /* Reject bad parameters */
    if (!cpioData || !cpioSize || !subfilesData || !subfilesSize ||
        !numSubfiles)
    {
        goto done;
    }

    /* Set the number of subfiles */
    *numSubfiles = 0;

    /* Find the next subfile */
    while (cpioSize)
    {
        const void* cpioEnd = (const unsigned char*)cpioData + cpioSize;
        const unsigned char* subfileData = (unsigned char*)cpioData;
        UINTN subfileSize = 0;
        BOOLEAN isCPIO = TRUE;

        /* Check for overflow */
        if (*numSubfiles >= CPIO_MAX_SUBFILES)
            goto done;

        /* Determine size of this subfile */
        if (cpioSize >= sizeof(CPIOHeader))
        {
            const CPIOHeader* header = (const CPIOHeader*)cpioData;

            while (header)
            {
                if (!CPIOMatchMagicNumber(header->magic))
                {
                    subfileSize = cpioSize;
                    isCPIO = FALSE;
                    break;
                }

                if (CPIOCheckEntry(header, cpioEnd) != 0)
                    goto done;

                subfileSize += CPIOGetEntrySize(header);

                if (CPIONextHeader(&header, cpioEnd) != 0)
                    goto done;
            }

            if (isCPIO)
            {
                /* Pad to 4-byte boundary */
                subfileSize = _RoundUpToMultiple(subfileSize, 4);

                /* Pad to block size */
                subfileSize = _RoundUpToMultiple(subfileSize, CPIO_BLOCK_SIZE);
            }
        }
        else 
        {
            subfileSize = cpioSize;
        }

        /* Set cpioData:cpioSize to refer to next subfile */
        cpioData = subfileData + subfileSize;
        cpioSize -= subfileSize;

        /* Insert next subfile into caller's array */
        subfilesData[*numSubfiles] = subfileData;
        subfilesSize[*numSubfiles] = subfileSize;
        (*numSubfiles)++;
    }

    rc = 0;

done:

    return rc;
}

BOOLEAN CPIOIsFile(
    const void* cpioData,
    UINTN cpioSize,
    const char* path)
{
    const CPIOHeader* header = NULL;

    /* Check parameters */
    if (!cpioData || !cpioSize || !path)
        return FALSE;

    /* Find the CPIO header for this path */
    if (_FindHeader(cpioData, cpioSize, path, &header) != 0)
        return FALSE;

    if (!(_GetFileMode(header) & CPIO_MODE_IFREG))
        return FALSE;

    return TRUE;
}

BOOLEAN CPIOIsDir(
    const void* cpioData,
    UINTN cpioSize,
    const char* path)
{
    const CPIOHeader* header = NULL;

    /* Check parameters */
    if (!cpioData || !cpioSize || !path)
        return FALSE;

    /* Find the CPIO header for this path */
    if (_FindHeader(cpioData, cpioSize, path, &header) != 0)
        return FALSE;

    if (!(_GetFileMode(header) & CPIO_MODE_IFDIR))
        return FALSE;

    return TRUE;
}

int CPIOFindFilesByBaseName(
    const void* cpioData,
    UINTN cpioSize,
    const char* baseName,
    StrArr* arr)
{
    int rc = -1;

    /* Clear the output array */
    if (arr)
        StrArrRelease(arr);

    /* Check parameters */
    if (!cpioData || !baseName || !arr)
        goto done;

    /* Is data too small to contain at least one header? */
    if (cpioSize < sizeof(CPIOHeader))
        goto done;

    /* Dump the file */
    {
        const CPIOHeader* header = (const CPIOHeader*)cpioData;
        const void* cpioEnd = (char*)cpioData + cpioSize;

        while (header)
        {
            const char* bn;
            const char* path;

            if (CPIOCheckEntry(header, cpioEnd) != 0)
                goto done;

            if (!(path = _GetName(header)))
                goto done;

            if ((bn = Strrchr(path, '/')))
                bn++;
            else
                bn = path;

            if (Strcmp(baseName, bn) == 0)
            {
                if (StrArrAppend(arr, path) != 0)
                    goto done;
            }

            if (CPIONextHeader(&header, cpioEnd) != 0)
                goto done;
        }
    }

    rc = 0;

done:

    return rc;
}


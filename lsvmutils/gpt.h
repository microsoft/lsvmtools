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
#ifndef _lsvmutils_gpt_h
#define _lsvmutils_gpt_h

#include "config.h"
#include <lsvmutils/eficommon.h>
#include <lsvmutils/guid.h>
#include <lsvmutils/blkdev.h>

#if !defined(BUILD_EFI)
# include <stdio.h>
#endif

#define GPT_BLOCK_SIZE 512

#define GPT_MAX_ENTRIES 128

typedef struct _GPTHeader
{
    char signature[8];
    UINT32 revision;
    UINT32 headerSize;
    UINT32 headerCRC32;
    UINT32 reserved;
    UINT64 primaryLBA;
    UINT64 backupLBA;
    UINT64 firstUsableLBA;
    UINT64 lastUsableLBA;
    UINT64 uniqueGUID1;
    UINT64 uniqueGUID2;
    UINT64 firstEntryLBA;
    UINT32 numberOfEntries;
    UINT32 sizeOfEntry;
    UINT32 entriesCRC32;
    UINT8 padding[420];
}
PACKED
GPTHeader;

typedef struct _GPTEntry
{
    UINT64 typeGUID1;
    UINT64 typeGUID2;
    UINT64 uniqueGUID1;
    UINT64 uniqueGUID2;
    UINT64 startingLBA;
    UINT64 endingLBA;
    UINT64 attributes;
    UINT16 typeName[36];
}
PACKED
GPTEntry;

typedef struct _GPT
{
    UINT8 mbr[GPT_BLOCK_SIZE];
    GPTHeader header;
    GPTEntry entries[GPT_MAX_ENTRIES];
}
GPT;

#if !defined(BUILD_EFI)
static __inline void DumpGPTHeader(const GPTHeader* h)
{
    printf("signature{%.*s}\n", 8, h->signature);
    printf("revision{%u}\n", h->revision);
    printf("headerSize{%u}\n", h->headerSize);
    printf("headerCRC32{%u}\n", h->headerCRC32);
    printf("reserved{%u}\n", h->reserved);
    printf("primaryLBA{%llu}\n", h->primaryLBA);
    printf("backupLBA{%llu}\n", h->backupLBA);
    printf("firstUsableLBA{%llu}\n", h->firstUsableLBA);
    printf("lastUsableLBA{%llu}\n", h->lastUsableLBA);

    printf("uniqueGUID{");
    DumpGUID2(h->uniqueGUID1, h->uniqueGUID2);
    printf("}\n");

    printf("firstEntryLBA{%llu}\n", h->firstEntryLBA);
    printf("numberOfEntries{%u}\n", h->numberOfEntries);
    printf("sizeOfEntry{%u}\n", h->sizeOfEntry);
    printf("entriesCRC32{%u}\n", h->entriesCRC32);
}
#endif /* !defined(BUILD_EFI) */

#if !defined(BUILD_EFI)
static __inline void DumpGPTEntry(const GPTEntry* e)
{
    char s[36];
    UINTN i;

    printf("typeGUID{");
    DumpGUID2(e->typeGUID1, e->typeGUID2);
    printf("}\n");

    printf("uniqueGUID{");
    DumpGUID2(e->uniqueGUID1, e->uniqueGUID2);
    printf("}\n");

    printf("startingLBA{%llu}\n", e->startingLBA);
    printf("endingLBA{%llu}\n", e->endingLBA);
    printf("attributes{%llu}\n", e->attributes);

    for (i = 0; i < sizeof(s); i++)
        s[i] = (char)e->typeName[i];

    printf("typeName{%.*s}\n", (int)sizeof(s), s);
}
#endif /* !defined(BUILD_EFI) */

int ReadGPT(
    Blkdev* dev,
    GPT* gpt);

int LoadGPT(
    const char* path, 
    GPT* gpt);

UINTN CountGPTEntries(
    const GPT* gpt);

UINT32 LBAToParitionNumber(
    const GPT* gpt,
    UINT64 lba);

#endif /* _lsvmutils_gpt_h */

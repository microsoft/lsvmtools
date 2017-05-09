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
#include "gpt.h"
#include "blkdev.h"

int ReadGPT(
    Blkdev* dev,
    GPT* gpt)
{
    int rc = -1;

    /* Check parameters */
    if (!dev || !gpt)
        goto done;

    /* Read the whole GPT into memory */
    if (BlkdevRead(dev, 0, gpt, sizeof(GPT)) != 0)
        goto done;

    /* Check whether GPT partition */
    if (Memcmp(gpt->header.signature, "EFI PART", 8) != 0)
        goto done;

    rc = 0;

done:
    return rc;
}

#if !defined(BUILD_EFI)
int LoadGPT(
    const char* path, 
    GPT* gpt)
{
    int rc = -1;
    Blkdev* dev = NULL;

    /* Check parameters */
    if (!path)
        goto done;

    /* Open the raw root partition */
    if (!(dev = BlkdevOpen(path, BLKDEV_ACCESS_RDONLY, 0)))
        goto done;

    /* Read the GPT into memory */
    if (ReadGPT(dev, gpt) != 0)
        goto done;

    rc = 0;

done:

    if (dev)
        dev->Close(dev);

    return rc;
}
#endif /* !defined(BUILD_EFI) */

UINTN CountGPTEntries(
    const GPT* gpt)
{
    UINTN n = 0;
    UINTN i;

    if (!gpt)
        return 0;

    for (i = 0; i < GPT_MAX_ENTRIES && gpt->entries[i].typeGUID1; i++)
        n++;

    return n;
}

UINT32 LBAToParitionNumber(
    const GPT* gpt,
    UINT64 lba)
{
    UINTN i;

    for (i = 0; i < GPT_MAX_ENTRIES; i++)
    {
        const GPTEntry* e = &gpt->entries[i];

        if (e->typeGUID1 == 0)
            break;

        if (lba >= e->startingLBA && lba <= e->endingLBA)
            return i + 1;
    }

    /* Not found */
    return 0;
}

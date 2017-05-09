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
#include <xz/lzmaextras.h>
#include <zlib/zlibextras.h>
#include "initrd.h"
#include "cpio.h"
#include "alloc.h"
#include "buf.h"
#include "print.h"
#include "ext2.h"
#include "strings.h"
#include "alloc.h"
#include "heap.h"

/* Whether to decompress initrd */
#if 1
# define DECOMPRESS
#endif

/*
**==============================================================================
**
** Common implementation:
**
**==============================================================================
*/

static int _DeleteKeyboardDriver(
    const void* cpioDataIn,
    UINTN cpioSizeIn,
    void** cpioDataOut,
    UINTN* cpioSizeOut)
{
    int rc = -1;
    StrArr arr = STRARR_INITIALIZER;
    UINTN i;
    void* cpioData = NULL;
    UINTN cpioSize;

    if (cpioDataOut)
        *cpioDataOut = NULL;

    if (cpioSizeOut)
        *cpioSizeOut = 0;

    /* Reject bad parameters */
    if (!cpioDataIn || !cpioSizeIn || !cpioDataOut || !cpioSizeOut)
    {
        goto done;
    }

    /* Make an in-memory copy of the archive */
    {
        if (!(cpioData = Memdup(cpioDataIn, cpioSizeIn)))
            goto done;

        cpioSize = cpioSizeIn;
    }

    /* Find all HyperV keyboard drivers and remove them */
    if (CPIOFindFilesByBaseName(
        cpioData, 
        cpioSize, 
        "hyperv-keyboard.ko",
        &arr) != 0)
    {
        goto done;
    }

    /* Remove each file in array */
    for (i = 0; i < arr.size; i++)
    {
        void* data = NULL;
        UINTN size;

        if (CPIORemoveFile(
            cpioData,
            cpioSize,
            arr.data[i],
            &data,
            &size) != 0)
        {
            goto done;
        }

        Free(cpioData);
        cpioData = data;
        cpioSize = size;
    }

    rc = 0;

    *cpioDataOut = cpioData;
    *cpioSizeOut = cpioSize;

done:

    StrArrRelease(&arr);

    if (rc != 0)
        Free(cpioData);

    return rc;
}

static int _InjectFiles(
    const void* cpioDataIn,
    UINTN cpioSizeIn,
    const void* bootkeyData,
    UINTN bootkeySize,
    const void* rootkeyData,
    UINTN rootkeySize,
    void** cpioDataOut,
    UINTN* cpioSizeOut)
{
    int rc = -1;
    void *cpioData = NULL;
    UINTN cpioSize = 0;

    /* Check parameters */
    if (!cpioDataIn || !cpioSizeOut || !bootkeyData || !bootkeySize ||
        !rootkeyData || !rootkeySize || !cpioDataOut || !cpioDataOut)
    {
        goto done;
    }

    /* Initialize output parameters */
    *cpioDataOut = NULL;
    *cpioSizeOut = 0;

    /* Make an in-memory copy of the archive */
    {
        if (!(cpioData = Memdup(cpioDataIn, cpioSizeIn)))
            goto done;

        cpioSize = cpioSizeIn;
    }

    /* Create 'etc' directory */
    if (CPIOIsDir(cpioData, cpioSize, "etc") == FALSE)
    {
        void* p = NULL;
        UINTN n;

        if (CPIOAddFile(
            cpioData,
            cpioSize,
            "etc",
            NULL,
            0,
            CPIO_MODE_IFDIR | 0755,
            &p,
            &n) != 0)
        {
            goto done;
        }

        Free(cpioData);
        cpioData = p;
        cpioSize = n;
    }

    /* Create 'etc/lsvmload' directory */
    if (CPIOIsDir(cpioData, cpioSize, "etc/lsvmload") == FALSE)
    {
        void* p = NULL;
        UINTN n;

        if (CPIOAddFile(
            cpioData,
            cpioSize,
            "etc/lsvmload",
            NULL,
            0,
            CPIO_MODE_IFDIR | 0755,
            &p,
            &n) != 0)
        {
            goto done;
        }

        Free(cpioData);
        cpioData = p;
        cpioSize = n;
    }

    /* Remove 'etc/lsvmload/bootkey' if it already exists */
    if (CPIOIsFile(cpioData, cpioSize, "etc/lsvmload/bootkey") == TRUE)
    {
        void* p = NULL;
        UINTN n;

        if (CPIORemoveFile(
            cpioData,
            cpioSize,
            "etc/lsvmload/bootkey",
            &p,
            &n) != 0)
        {
            goto done;
        }

        Free(cpioData);
        cpioData = p;
        cpioSize = n;
    }

    /* Create 'etc/lsvmload/bootkey' file */
    {
        void* p = NULL;
        UINTN n;

        if (CPIOAddFile(
            cpioData,
            cpioSize,
            "etc/lsvmload/bootkey",
            bootkeyData,
            bootkeySize,
            CPIO_MODE_IFREG | 0755,
            &p,
            &n) != 0)
        {
            goto done;
        }

        Free(cpioData);
        cpioData = p;
        cpioSize = n;
    }

    /* Remove 'etc/lsvmload/rootkey' if it already exists */
    if (CPIOIsFile(cpioData, cpioSize, "etc/lsvmload/rootkey") == TRUE)
    {
        void* p = NULL;
        UINTN n;

        if (CPIORemoveFile(
            cpioData,
            cpioSize,
            "etc/lsvmload/rootkey",
            &p,
            &n) != 0)
        {
            goto done;
        }

        Free(cpioData);
        cpioData = p;
        cpioSize = n;
    }

    /* Create 'etc/lsvmload/rootkey' file */
    {
        void* p = NULL;
        UINTN n;

        if (CPIOAddFile(
            cpioData,
            cpioSize,
            "etc/lsvmload/rootkey",
            rootkeyData,
            rootkeySize,
            CPIO_MODE_IFREG | 0755,
            &p,
            &n) != 0)
        {
            goto done;
        }

        Free(cpioData);
        cpioData = p;
        cpioSize = n;
    }

    /* Remove 'etc/lsvmload/specialize' if it already exists */
    if (CPIOIsFile(cpioData, cpioSize, "etc/lsvmload/specialize") == TRUE)
    {
        void* p = NULL;
        UINTN n;

        if (CPIORemoveFile(
            cpioData,
            cpioSize,
            "etc/lsvmload/specialize",
            &p,
            &n) != 0)
        {
            goto done;
        }

        Free(cpioData);
        cpioData = p;
        cpioSize = n;
    }

    /* Delete the Hyper-V keyboard driver */
    {
        void* p = NULL;
        UINTN n;

        if (_DeleteKeyboardDriver(
            cpioData,
            cpioSize,
            &p,
            &n) != 0)
        {
            goto done;
        }

        Free(cpioData);
        cpioData = p;
        cpioSize = n;
    }

    rc = 0;

    *cpioDataOut = cpioData;
    *cpioSizeOut = cpioSize;

done:

    if (rc != 0)
        Free(cpioData);

    return rc;
}

int InitrdMakeArchive(
    const void* bootkeyData,
    UINTN bootkeySize,
    const void* rootkeyData,
    UINTN rootkeySize,
    void** cpioDataOut,
    UINTN* cpioSizeOut)
{
    int rc = -1;
    void* cpioData = NULL;
    UINTN cpioSize;

    /* Check parameters */
    if (!cpioDataOut || !cpioSizeOut)
        goto done;

    /* Initialize output parameters */
    *cpioDataOut = NULL;
    *cpioSizeOut = 0;

    /* Create new CPIO archive */
    if (CPIONew(&cpioData, &cpioSize) != 0)
        goto done;

    /* Inject the keys into the new CPIO archive */
    {
        void* p = NULL;
        UINTN n;

        if (_InjectFiles(
            cpioData, 
            cpioSize, 
            bootkeyData, 
            bootkeySize,
            rootkeyData, 
            rootkeySize,
            &p,
            &n) != 0)
        {
            goto done;
        }

        Free(cpioData);
        cpioData = p;
        cpioSize = n;
    }

    /* Set output parameters */
    *cpioDataOut = cpioData;
    *cpioSizeOut = cpioSize;

    rc = 0;

done:

    if (rc != 0)
    {
        if (cpioData)
            Free(cpioData);
    }

    return rc;
}

/*
**==============================================================================
**
** InitrdInjectFiles(): compressed implementation:
**
**==============================================================================
*/

#if defined(DECOMPRESS)

int InitrdInjectFiles(
    const void* initrdData,
    UINTN initrdSize,
    const void* bootkeyData,
    UINTN bootkeySize,
    const void* rootkeyData,
    UINTN rootkeySize,
    void** initrdDataOut,
    UINTN* initrdSizeOut)
{
    int rc = -1;
    const void* subfilesData[CPIO_MAX_SUBFILES];
    UINTN subfilesSize[CPIO_MAX_SUBFILES];
    UINTN numSubfiles = 0;
    Heap heap = HEAP_INITIALIZER;
    Buf buf = BUF_INITIALIZER;
    UINTN i;

    if (initrdDataOut)
        *initrdDataOut = NULL;

    if (initrdSizeOut)
        *initrdSizeOut = 0;

    /* Reject bad parameters */
    if (!initrdData || !initrdSize || !bootkeyData || !bootkeySize ||
        !rootkeyData || !rootkeySize || !initrdDataOut || !initrdSizeOut)
    {
        goto done;
    }

    /* Split initrd into subfiles */
    if (CPIOSplitFile(
        initrdData, 
        initrdSize, 
        subfilesData, 
        subfilesSize, 
        &numSubfiles) != 0)
    {
        goto done;
    }

    /* If no subfiles found, then fail */
    if (numSubfiles == 0)
        goto done;

    /* Decompress all compressed subfiles */
    for (i = 0; i < numSubfiles; i++)
    {
        if (lzmaextras_test(subfilesData[i], subfilesSize[i]) == 0)
        {
            unsigned char* data = NULL;
            unsigned long size = 0;
            void* newData = NULL;
            UINTN newSize = 0;

            if (lzmaextras_decompress(
                subfilesData[i],
                subfilesSize[i],
                &data,
                &size) != 0)
            {
                goto done;
            }

            /* Free this later */
            HeapAdd(&heap, data);

            /* Inject the keys into this existing CPIO archive */
            if (_InjectFiles(
                data,
                size,
                bootkeyData,
                bootkeySize,
                rootkeyData,
                rootkeySize,
                &newData,
                &newSize) != 0)
            {
                goto done;
            }

            /* Free this later */
            HeapAdd(&heap, newData);

            subfilesData[i] = newData;
            subfilesSize[i] = newSize;
        }
        else if (zlibextras_test(subfilesData[i], subfilesSize[i]) == 0)
        {
            unsigned char* data = NULL;
            unsigned long size = 0;
            void* newData = NULL;
            UINTN newSize = 0;

            if (zlibextras_decompress(
                subfilesData[i],
                subfilesSize[i],
                &data,
                &size) != 0)
            {
                goto done;
            }

            /* Free this later */
            HeapAdd(&heap, data);

            /* Inject the keys into this existing CPIO archive */
            if (_InjectFiles(
                data,
                size,
                bootkeyData,
                bootkeySize,
                rootkeyData,
                rootkeySize,
                &newData,
                &newSize) != 0)
            {
                goto done;
            }

            /* Free this later */
            HeapAdd(&heap, newData);

            subfilesData[i] = newData;
            subfilesSize[i] = newSize;
        }
        else if (CPIOTest(subfilesData[i], subfilesSize[i]) != 0)
        {
            /* Unknown subfile type */
            goto done;
        }
    }

    /* Create new initrd image */
    for (i = 0; i < numSubfiles; i++)
    {
        if (BufAppend(&buf, subfilesData[i], subfilesSize[i]) != 0)
            goto done;
    }

    *initrdDataOut = buf.data;
    *initrdSizeOut = buf.size;

    rc = 0;

done:

    if (rc != 0)
        BufRelease(&buf);

    HeapFree(&heap);

    return rc;
}

#endif /* defined(DECOMPRESS) */

/*
**==============================================================================
**
** InitrdInjectFiles(): uncompressed implementation:
**
**==============================================================================
*/

#if !defined(DECOMPRESS)

int InitrdInjectFiles(
    const void* initrdData,
    UINTN initrdSize,
    const void* bootkeyData,
    UINTN bootkeySize,
    const void* rootkeyData,
    UINTN rootkeySize,
    void** initrdDataOut,
    UINTN* initrdSizeOut)
{
    int rc = -1;
    const void* subfilesData[CPIO_MAX_SUBFILES];
    UINTN subfilesSize[CPIO_MAX_SUBFILES];
    UINTN numSubfiles = 0;
    void* subfileData = NULL;
    UINTN subfileSize = 0;
    Buf buf = BUF_INITIALIZER;

    if (initrdDataOut)
        *initrdDataOut = NULL;

    if (initrdSizeOut)
        *initrdSizeOut = 0;

    /* Reject bad parameters */
    if (!initrdData || !initrdSize || !bootkeyData || !bootkeySize ||
        !rootkeyData || !rootkeySize || !initrdDataOut || !initrdSizeOut)
    {
        goto done;
    }

    /* Split initrd into subfiles */
    if (CPIOSplitFile(
        initrdData, 
        initrdSize, 
        subfilesData, 
        subfilesSize, 
        &numSubfiles) != 0)
    {
        goto done;
    }

    if (numSubfiles == 0)
        goto done;

    /* If first subfile is an uncompressed CPIO archive, update it. 
     * Else, prepend a new CPIO archive with the keys.
     */
    if (subfilesSize[0] >= sizeof(CPIOHeader) &&
        CPIOMatchMagicNumber(((const CPIOHeader*)subfilesData[0])->magic))
    {
        /* Inject the keys into this existing CPIO archive */
        if (_InjectFiles(
            subfilesData[0],
            subfilesSize[0],
            bootkeyData,
            bootkeySize,
            rootkeyData,
            rootkeySize,
            &subfileData,
            &subfileSize) != 0)
        {
            goto done;
        }

        /* Create new initrd image */
        {
            UINTN i;

            if (BufAppend(&buf, subfileData, subfileSize) != 0)
                goto done;

            for (i = 1; i < numSubfiles; i++)
            {
                if (BufAppend(&buf, subfilesData[i], subfilesSize[i]) != 0)
                    goto done;
            }
        }
    }
    else
    {
        /* Create a new CPIO archive that contains these keys */
        if (InitrdMakeArchive(
            bootkeyData,
            bootkeySize,
            rootkeyData,
            rootkeySize,
            &subfileData,
            &subfileSize) != 0)
        {
            goto done;
        }

        /* Create new initrd image */
        {
            UINTN i;

            if (BufAppend(&buf, subfileData, subfileSize) != 0)
                goto done;

            for (i = 0; i < numSubfiles; i++)
            {
                if (BufAppend(&buf, subfilesData[i], subfilesSize[i]) != 0)
                    goto done;
            }
        }
    }

    *initrdDataOut = buf.data;
    *initrdSizeOut = buf.size;

    rc = 0;

done:

    if (rc != 0)
        BufRelease(&buf);

    if (subfileData)
        Free(subfileData);

    return rc;
}

#endif /* !defined(DECOMPRESS) */

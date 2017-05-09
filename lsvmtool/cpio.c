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
#include <lsvmutils/cpio.h>
#include <lsvmutils/file.h>
#include <lsvmutils/alloc.h>
#include <stdlib.h>
#include "cpio.h"

int CPIOGetFileByPath(
    const char* cpioPath,
    const char* src,
    const char* dest)
{
    int rc = -1;
    void* cpioData = NULL;
    size_t cpioSize = 0;
    void* fileData = NULL;
    UINTN fileSize;

    /* Check parameters */
    if (!cpioPath || !src || !dest)
        goto done;

    /* Load the CPIO file */
    if (LoadFile(cpioPath, 0, (unsigned char**)&cpioData, &cpioSize) != 0)
        goto done;

    /* Get the file */
    if (CPIOGetFile(cpioData, cpioSize, src, &fileData, &fileSize) != 0)
        goto done;

    /* Write the output file */
    if (PutFile(dest, fileData, fileSize) != 0)
        goto done;

    rc = 0;

done:

    if (cpioData)
        Free(cpioData);

    if (fileData)
        Free(fileData);

    return rc;
}

int CPIODumpFileByPath(
    const char* cpioPath)
{
    int rc = 0;
    void* cpioData = NULL;
    size_t cpioSize;

    /* Check parameters */
    if (!cpioPath)
        goto done;

    /* Load file */
    if (LoadFile(cpioPath, 0, (unsigned char**)&cpioData, &cpioSize) != 0)
        goto done;

    /* Dump the file */
    if (CPIODumpFile(cpioData, cpioSize) != 0)
        goto done;

done:

    if (cpioData)
        Free(cpioData);

    return rc;
}

int CPIOCheckFileByPath(
    const char* cpioPath,
    const char* path)
{
    int rc = -1;
    void* cpioData = NULL;
    size_t cpioSize;

    /* Check parameters */
    if (!cpioPath || !path)
        goto done;

    /* Load file */
    if (LoadFile(cpioPath, 0, (unsigned char**)&cpioData, &cpioSize) != 0)
        goto done;

    /* Check the file */
    if (CPIOCheckFile(cpioData, cpioSize, path) != 0)
        goto done;

    rc = 0;

done:

    if (cpioData)
        Free(cpioData);

    return rc;
}

int CPIOAddFileByPath(
    const char* cpioInPath,
    const char* cpioOutPath,
    const char* newPath,
    const char* filePath,
    unsigned int mode)
{
    int rc = -1;
    void* cpioDataIn = NULL;
    size_t cpioSizeIn = 0;
    void* fileData = NULL;
    size_t fileSize = 0;
    void* cpioDataOut = NULL;
    UINTN cpioSizeOut = 0;

    if (!cpioInPath || !cpioOutPath || !newPath)
        goto done;

    if (LoadFile(cpioInPath, 0, (unsigned char**)&cpioDataIn, &cpioSizeIn) != 0)
        goto done;
    

    /* If not a directory, then load it */
    if (!(mode & CPIO_MODE_IFDIR))
    {
        if (LoadFile(filePath, 0, (unsigned char**)&fileData, &fileSize) != 0)
            goto done;
    }

    if (CPIOAddFile(
        cpioDataIn,
        cpioSizeIn,
        newPath,
        fileData,
        fileSize,
        mode,
        &cpioDataOut,
        &cpioSizeOut) != 0)
    {
        goto done;
    }

    if (PutFile(cpioOutPath, cpioDataOut, cpioSizeOut) != 0)
    {
        goto done;
    }

    rc = 0;

done:

    if (cpioDataIn)
        Free(cpioDataIn);

    if (cpioDataOut)
        Free(cpioDataOut);

    if (fileData)
        Free(fileData);

    return rc;
}

int CPIORemoveFileByPath(
    const char* cpioInPath,
    const char* cpioOutPath,
    const char* path)
{
    int rc = -1;
    void* cpioDataIn = NULL;
    size_t cpioSizeIn = 0;
    void* cpioDataOut = NULL;
    UINTN cpioSizeOut = 0;

    if (!cpioInPath || !cpioOutPath || !path)
        goto done;

    if (LoadFile(cpioInPath, 0, (unsigned char**)&cpioDataIn, &cpioSizeIn) != 0)
        goto done;

    if (CPIORemoveFile(
        cpioDataIn,
        cpioSizeIn,
        path,
        &cpioDataOut,
        &cpioSizeOut) != 0)
    {
        goto done;
    }

    if (PutFile(cpioOutPath, cpioDataOut, cpioSizeOut) != 0)
        goto done;

    rc = 0;

done:

    if (cpioDataIn)
        Free(cpioDataIn);

    if (cpioDataOut)
        Free(cpioDataOut);

    return rc;
}

int CPIOSplitFileByPath(
    const char *cpioPath)
{
    int rc = -1;
    char* cpioData = NULL;
    size_t cpioSize = 0;
    const void* subfilesData[CPIO_MAX_SUBFILES];
    UINTN subfilesSize[CPIO_MAX_SUBFILES];
    UINTN numSubfiles = 0;
    UINTN i;

    if (!cpioPath)
        goto done;

    if (LoadFile(cpioPath, 0, (unsigned char**)&cpioData, &cpioSize) != 0)
        goto done;

    if (CPIOSplitFile(
        cpioData, 
        cpioSize, 
        subfilesData, 
        subfilesSize, 
        &numSubfiles) != 0)
    {
        goto done;
    }

    for (i = 0; i < numSubfiles; i++)
    {
        U64tostrBuf buf;
        char path[128];

        if (Strlcpy(path, cpioPath, sizeof(path)) >= sizeof(path))
            goto done;

        if (Strlcat(path, ".", sizeof(path)) >= sizeof(path))
            goto done;

        if (Strlcat(path, U64tostr(&buf, i + 1), sizeof(path)) >= sizeof(path))
            goto done;

        if (PutFile(path, subfilesData[i], subfilesSize[i]) != 0)
            goto done;
    }

    rc = 0;

done:

    if (cpioData)
        Free(cpioData);

    return rc;
}

int CPIOMergeFilesByPath(
    const char* cpioOut,
    const char* const files[],
    size_t numFiles)
{
    UINTN i;
    int rc = -1;

    if (!files || !cpioOut)
        goto done;

    for (i = 0; i < numFiles; i++)
    {
        char* dataIn = NULL;
        size_t sizeIn = 0;

        if (LoadFile(files[i], 0, (unsigned char**)&dataIn, &sizeIn) != 0)
        {
            Free(dataIn);
            goto done;
        }

        if (AppendFile(cpioOut, dataIn, sizeIn) != 0)
        {
            Free(dataIn);
            goto done;
        }
        Free(dataIn);
    }
    rc = 0;

done:
    return rc;
}

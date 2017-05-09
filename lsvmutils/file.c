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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "config.h"
#include "file.h"

BOOLEAN IsDir(
    const char* path)
{
    struct stat st;

    if (!path)
        return FALSE;

    if (stat(path, &st) != 0)
        return FALSE;

    if (!(st.st_mode & S_IFDIR))
        return FALSE;

    return TRUE;
}

int LoadFile(
    const char* path, 
    size_t extraBytes,
    unsigned char** dataOut,
    size_t* outSize)
{
    FILE* is;
    size_t capacity = 4096;
    size_t size = 0;
    unsigned char* data;

    /* Clear output parameters */
    *dataOut = NULL;
    *outSize = 0;

    /* Open the file */
    if (!(is = fopen(path, "rb")))
        return -1;

    /* Allocate memory to hold contents of file */
    if (!(data = (unsigned char*)malloc(capacity + extraBytes)))
    {
        fclose(is);
        return -1;
    }

    /* Read file into memory */
    for (;;)
    {
        char buf[4096];
        size_t n;

        n = fread(buf, 1, sizeof(buf), is);

        if (n <= 0)
            break;

        if (size + n > capacity)
        {
            void* tmp;

            capacity *= 2;

            if (size + n > capacity)
                capacity += n;

            if (!(tmp = (unsigned char*)realloc(data, capacity + extraBytes)))
            {
                fclose(is);
                free(data);
            }

            data = tmp;
        }

        memcpy(data + size, buf, n);
        size += n;
    }

    /* Close the file */
    fclose(is);

    /* Set output parameters */
    *dataOut = data;
    *outSize = size;

    /* Success! */
    return 0;
}

int PutFile(
    const char* path,
    const void* data,
    size_t size)
{
    int rc = -1;
    FILE* os;

    if (!(os = fopen(path, "wb")))
        goto done;

    if (fwrite(data, 1, size, os) != size)
        goto done;

    rc = 0;

done:

    if (os)
        fclose(os);

    return rc;
}

int AppendFile(
    const char* path,
    const void* data,
    size_t size)
{
    int rc = 0;
    FILE* os;

    if (!(os = fopen(path, "ab")))
    {
        rc = -1;
        goto done;
    }

    if (fwrite(data, 1, size, os) != size)
    {
        rc = -1;
        goto done;
    }

done:

    if (os)
        fclose(os);

    return rc;
}

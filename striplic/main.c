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
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>

static int LoadFile(
    const char* path,
    char** dataOut,
    size_t* sizeOut)
{
    int rc = -1;
    FILE* is = NULL;
    char* data = NULL;
    struct stat st;

    /* Reject null parameters */
    if (!path || !dataOut || !sizeOut)
        goto done;

    /* Open the file */
    if (!(is = fopen(path, "rb")))
        goto done;

    /* Get the size of the file */
    if (stat(path, &st) != 0)
        goto done;

    /* Allocate buffer */
    if (!(data = (char*)malloc(st.st_size)))
        goto done;

    /* Read the file into memory */
    if (fread(data, 1, st.st_size, is) != st.st_size)
        goto done;

    rc = 0;

done:

    if (is)
        fclose(is);

    if (rc == 0)
    {
        *dataOut = data;
        *sizeOut = st.st_size;
    }
    else
        free(data);

    return rc;
}

static int PutFile(
    const char* path,
    const char* data,
    size_t size)
{
    int rc = -1;
    FILE* os = NULL;

    /* Reject null parameters */
    if (!path || !data)
        goto done;

    /* Open the file */
    if (!(os = fopen(path, "wb")))
        goto done;

    /* Write the file */
    if (fwrite(data, 1, size, os) != size)
        goto done;

    rc = 0;

done:

    if (os)
        fclose(os);

    return rc;
}

static int StripLicense(const char* path)
{
    int rc = -1;
    char* data = NULL;
    char* p;
    char* start;
    char* end;
    size_t size;

    if (!path)
        goto done;

    /* Load the file */
    if (LoadFile(path, &data, &size) != 0)
        goto done;

    /* Skip leading spaces */
    for (p = data; isspace(*p); p++)
        p++;

    /* If file does not start with comment, then return success */
    if (!(p[0] == '/' && p[1] == '*'))
    {
        rc = 0;
        goto done;
    }

    /* Find 'start' and 'end' of leading comment */
    {
        start = p;
        p += 2;

        while (*p && !(p[0] == '*' && p[1] == '/'))
            p++;

        /* If end of file, then fail */
        if (*p == '\0')
            goto done;

        p += 2;

        /* Skip traling whitespace */

        while (isspace(*p))
            p++;

        end = p;
    }

    /* See if leading comment contains "MIT License" string */
    {
        char save = *end;
        *end = '\0';

        if (!(strstr(start, "MIT License")))
        {
            rc = 0;
            goto done;
        }

        *end = save;
    }

    /* Write the new file */
    if (PutFile(path, end, (data + size) - end) != 0)
        goto done;

    rc = 0;

done:

    if (data)
        free(data);

    return rc;
}

int main(int argc, const char* argv[])
{
    const char* arg0 = argv[0];
    int i;

    for (i = 1; i < argc; i++)
    {
        printf("Stripping %s\n", argv[i]);

        if (StripLicense(argv[i]) != 0)
        {
            fprintf(stderr, "%s: failed to strip license: %s\n", arg0, argv[i]);
            exit(1);
        }
    }

    return 0;
}

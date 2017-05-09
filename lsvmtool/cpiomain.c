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
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <lsvmutils/alloc.h>
#include <lsvmutils/file.h>
#include <string.h>
#include "cpio.h"

static int _str_to_mask(const char *str, unsigned int *mask)
{
    int i;
    int len = 3;

    if (!mask || !str)
        return -1;
    
    *mask = 0;
    for (i = 0; i < len; i++)
    {
        if (str[i] == '\0')
            return -1;
        if (str[i] < '0' || str[i] > '7')
            return -1;
        *mask <<= 3;
        *mask |= (unsigned int) (str[i] - '0');
    }

    if (str[len] != '\0')
        return -1; 
    return 0;
}

static int _new(int argc, const char* argv[])
{
    int status = 1;
    const char* cpioPath;
    void* cpioData = NULL;
    UINTN cpioSize = 0;

    /* Check usage */
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s %s CPIOPATH\n", argv[0], argv[1]);
        goto done;
    }

    /* Collect arguments */
    cpioPath = argv[2];

    /* Create new CPIO archive */
    if (CPIONew(&cpioData, &cpioSize) != 0)
    {
        fprintf(stderr, "%s %s: failed to create new CPIO archive\n", 
            argv[0], argv[1]);
        goto done;
    }

    /* Create new file */
    if (PutFile(cpioPath, cpioData, cpioSize) != 0)
    {
        fprintf(stderr, "%s %s: failed to create file: %s\n", 
            argv[0], argv[1], cpioPath);
        goto done;
    }

    /* Success! */
    status = 0;

done:

    if (cpioData)
        Free(cpioData);

    return status;
}

static int _add(int argc, const char* argv[])
{
    int status = 0;
    const char* cpioPath;
    const char* path;
    const char* filename;
    unsigned int mode;

    /* Check usage */
    if (argc != 5 && argc != 6)
    {
        fprintf(stderr, "Usage: %s %s CPIOPATH PATH FILENAME [PERMISSIONS]\n", 
            argv[0], argv[1]);
        status = 1;
        goto done;
    }

    /* Collect arguments */
    cpioPath = argv[2];
    path = argv[3];
    filename = argv[4];

    /* Get the mode of the file */
    {
        struct stat st;

        if (stat(filename, &st) != 0)
        {
            fprintf(stderr, "%s %s: stat failed: %s\n", argv[0], argv[1],
                filename);
            status = 1;
            goto done;
        }

        mode = st.st_mode;
    }

    /* Change mode using cmd line permissions. */
    if (argc == 6)
    {
        unsigned int mask = 0;
        const char *str = argv[5];

        if (_str_to_mask(str, &mask))
        {
            fprintf(stderr, "%s %s: Permissions should be 3 digit octal like UNIX.\n",
                argv[0], str);
            status = 1;
            goto done;
        }

        /* Last nine digits is the permissions. */
        mode = (mode & ~0x1FF) | mask;
    }

    if (CPIOAddFileByPath(
        cpioPath,
        cpioPath,
        path,
        filename,
        mode) != 0)
    {
        fprintf(stderr, "%s %s: failed to add file: %s\n", argv[0], argv[1],
            path);
        status = 1;
        goto done;
    }

done:

    return status;
}

static int _mkdir(int argc, const char* argv[])
{
    int status = 0;
    const char* cpioPath;
    const char* path;
    unsigned int mode = CPIO_MODE_IFDIR | 0755;

    /* Check usage */
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s %s CPIOPATH PATH\n", argv[0], argv[1]);
        status = 1;
        goto done;
    }

    /* Collect arguments */
    cpioPath = argv[2];
    path = argv[3];

    /* Create the directrory */
    if (CPIOAddFileByPath(
        cpioPath,
        cpioPath,
        path,
        NULL,
        mode) != 0)
    {
        fprintf(stderr, "%s %s: failed to add file: %s\n", argv[0], argv[1],
            path);
        status = 1;
        goto done;
    }

done:

    return status;
}

static int _remove(int argc, const char* argv[])
{
    int status = 0;
    const char* cpioPath;
    const char* path;

    /* Check usage */
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s %s CPIOPATH PATH\n", argv[0], argv[1]);
        status = 1;
        goto done;
    }

    /* Collect arguments */
    cpioPath = argv[2];
    path = argv[3];

    if (CPIORemoveFileByPath(
        cpioPath,
        cpioPath,
        path) != 0)
    {
        fprintf(stderr, "%s: %s: failed to remove file: %s\n", 
            argv[0], argv[1], path);
        exit(1);
    }

done:

    return status;
}

static int _dump(int argc, const char* argv[])
{
    int status = 0;
    const char* cpioPath;

    /* Check usage */
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s %s CPIOPATH\n", argv[0], argv[1]);
        status = 1;
        goto done;
    }

    /* Collect arguments */
    cpioPath = argv[2];

    if (CPIODumpFileByPath(cpioPath) != 0)
    {
        fprintf(stderr, "%s: %s: failed to dump file: %s\n", 
            argv[0], argv[1], cpioPath);
        exit(1);
    }

done:

    return status;
}

static int _check(int argc, const char* argv[])
{
    int status = 0;
    const char* cpioPath;
    const char* path;

    /* Check usage */
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s %s CPIOPATH PATH\n", argv[0], argv[1]);
        status = 1;
        goto done;
    }

    /* Collect arguments */
    cpioPath = argv[2];
    path = argv[3];
        
    status = CPIOCheckFileByPath(cpioPath, path);
    if (status == -1)
    {
        fprintf(stderr, "%s: %s: failed to check file: %s %s\n", 
            argv[0], argv[1], cpioPath, path);
        exit(1);
    }

done:
    return status;
}

static int _split(int argc, const char *argv[])
{
    int status = 0;
    const char* cpioPath;

    /* Check usage */
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s %s CPIOPATH\n", argv[0], argv[1]);
        status = 1;
        goto done;
    }

    /* Collect arguments */
    cpioPath = argv[2];

    if (CPIOSplitFileByPath(cpioPath) != 0)
    {
        fprintf(stderr, "%s: %s: failed to split file: %s\n",
            argv[0], argv[1], cpioPath);
        exit(1);
    }

done:
    return status;
}

static int _merge(int argc, const char *argv[])
{
    int status = 0;
    const char* cpioPath;

    /* Check usage */
    if (argc < 4)
    {
        fprintf(stderr, "Usage: %s %s CPIOPATH MERGEPATH1 [MERGEPATH2] ...\n", argv[0], argv[1]);
        status = 1;
        goto done;
    }

    /* Collect arguments */
    cpioPath = argv[2];

    if (CPIOMergeFilesByPath(cpioPath, argv + 3, argc - 3) != 0)
    {
        fprintf(stderr, "%s: %s: failed to merge file: %s\n",
            argv[0], argv[1], cpioPath);
        exit(1);
    }

done:
    return status;
}

static int _find(int argc, const char *argv[])
{
    int status = 1;
    const char* cpioPath;
    const char* baseName;
    void* cpioData = NULL;
    size_t cpioSize;
    StrArr arr = STRARR_INITIALIZER;
    UINTN i;

    /* Check usage */
    if (argc < 4)
    {
        fprintf(stderr, "Usage: %s %s CPIOPATH BASENAME\n", argv[0], argv[1]);
        goto done;
    }

    /* Collect arguments */
    cpioPath = argv[2];
    baseName = argv[3];

    /* Load the CPIO file */
    if (LoadFile(cpioPath, 0, (unsigned char**)&cpioData, &cpioSize) != 0)
        goto done;

    /* Load all paths with this basename */
    if (CPIOFindFilesByBaseName(
        cpioData,
        cpioSize,
        baseName,
        &arr) != 0)
    {
        goto done;
    }

    for (i = 0; i < arr.size; i++)
        printf("%s\n", arr.data[i]);

    status = 1;

done:

    if (cpioData)
        Free(cpioData);

    StrArrRelease(&arr);

    return status;
}

static int _get(int argc, const char *argv[])
{
    int status = 0;
    const char* cpioPath;
    const char* destFile;
    const char* srcFile;

    /* Check usage */
    if (argc != 5)
    {
        fprintf(stderr, "Usage: %s %s CPIOPATH SRCFILE DESTFILE\n", argv[0], argv[1]);
        status = 1;
        goto done;
    }

    /* Collect arguments */
    cpioPath = argv[2];
    srcFile = argv[3];
    destFile = argv[4];

    if (CPIOGetFileByPath(cpioPath, srcFile, destFile) != 0)
    {
        fprintf(stderr, "%s: %s: failed to get file\n", argv[0], argv[1]);
        exit(1);
    }

done:
    return status;
}

typedef int (*CommandCallback)(int argc, const char *argv[]);

typedef struct _Command
{
    const char* name;
    const char* help;
    CommandCallback callback;
}
Command;

static Command _commands[] =
{
    {
        "new", 
        "Create a new (empty) CPIO archive",
        _new,
    },
    {
        "get", 
        "Get a file from the CPIO archive",
        _get,
    },
    {
        "add", 
        "Add a file to the CPIO archive",
        _add,
    },
    {
        "mkdir", 
        "Create a new directory in the CPIO archive",
        _mkdir,
    },
    {
        "remove", 
        "Remove a file from the CPIO archive",
        _remove,
    },
    {
        "dump", 
        "Dump information about the CPIO archive",
        _dump,
    },
    {
        "check", 
        "Check the sanity of the CPIO archive",
        _check,
    },
    {
        "split", 
        "Split cancatenated CPIO archives",
        _split,
    },
    {
        "merge", 
        "Merge multiple CPIO archives into one",
        _merge,
    },
    {
        "find", 
        "Find a file or directory within the CPIO archive",
        _find,
    },
};

static size_t _ncommands = sizeof(_commands) / sizeof(_commands[0]);

int cpio_main(int argc, const char* argv[])
{
    int i;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s SUBCOMMAND [ARGS]\n", argv[0]);
        fprintf(stderr, "Where SUBCOMMAND is:\n");

        for (i = 0; i < _ncommands; i++)
        {
            fprintf(stderr, "    %-*s - %s\n", 
                6,
                _commands[i].name,
                _commands[i].help);
        }

        fprintf(stderr, "\n");
        exit(1);
    }

    /* Find and execute the command given by argv[1] */
    for (i = 0; i < _ncommands; i++)
    {
        if (strcmp(argv[1], _commands[i].name) == 0)
        {
            exit((*_commands[i].callback)(argc, argv));
        }
    }

    fprintf(stderr, "%s: unknown command: '%s'\n", argv[0], argv[1]);

    return 0;
}

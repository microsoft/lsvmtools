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
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <lsvmutils/vfat.h>
#include <lsvmutils/getopt.h>
#include <lsvmutils/sha.h>
#include <lsvmutils/utils.h>
#include <lsvmutils/file.h>
#include <lsvmutils/alloc.h>
#include <lsvmutils/memblkdev.h>
#include <lsvmutils/strings.h>
#include <lsvmutils/print.h>
#include <lsvmutils/chksum.h>
#include <lsvmutils/gpt.h>
#include <lsvmutils/byteorder.h>

#if defined(__linux__)
# include <unistd.h>
#endif

static int _dump_command(
    VFAT* vfat,
    int argc, 
    const char* argv[])
{
    int status = 1;

    /* Check arguments */
    if (argc != 1)
    {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        goto done;
    }

    VFATDump(vfat);

    status = 0;

done:

    return status;
}

static int _stat_command(
    VFAT* vfat,
    int argc, 
    const char* argv[])
{
    int status = 1;
    const char* vfatpath;
    VFATDirectoryEntry entry;

    /* Check arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s VFATPATH\n", argv[0]);
        goto done;
    }

    /* Collect arguments */
    vfatpath = argv[1];

    /* Get file from VFAT file system */
    if (VFATStatFile(vfat, vfatpath, &entry) != 0)
    {
        fprintf(stderr, "%s: failed to stat file: %s\n", argv[0], vfatpath);
        goto done;
    }

    /* Dump the directory entry */
    VFATDumpDirectoryEntry(&entry);

    status = 0;

done:

    return status;
}

static int _get_command(
    VFAT* vfat,
    int argc, 
    const char* argv[])
{
    int status = 1;
    const char* vfatpath;
    const char* localpath;
    void* data = NULL;
    UINTN size;

    /* Check arguments */
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s VFATPATH LOCALPATH\n", argv[0]);
        goto done;
    }

    /* Collect arguments */
    vfatpath = argv[1];
    localpath = argv[2];

    /* Get file from VFAT file system */
    if (VFATGetFile(vfat, vfatpath, &data, &size) != 0)
    {
        fprintf(stderr, "%s: failed to get file: %s\n", argv[0], vfatpath);
        goto done;
    }

    /* Write file to disk */
    if (PutFile(localpath, data, size) != 0)
    {
        fprintf(stderr, "%s: failed to write file: %s\n", argv[0], localpath);
        goto done;
    }

    status = 0;

done:

    if (data)
        Free(data);

    return status;
}

static int _put_command(
    VFAT* vfat,
    int argc, 
    const char* argv[])
{
    int status = 1;
    const char* vfatpath;
    const char* localpath;
    UINT8* data = NULL;
    size_t size;

    /* Check arguments */
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s VFATPATH LOCALPATH\n", argv[0]);
        goto done;
    }

    /* Collect arguments */
    vfatpath = argv[1];
    localpath = argv[2];

    /* Load local file into memory */
    if (LoadFile(localpath, 0, &data, &size) != 0)
    {
        fprintf(stderr, "%s: failed to read file: %s\n", argv[0], localpath);
        goto done;
    }

    /* Get file from VFAT file system */
    if (VFATPutFile(vfat, vfatpath, data, size) != 0)
    {
        fprintf(stderr, "%s: failed to put file: %s\n", argv[0], vfatpath);
        goto done;
    }

    status = 0;

done:

    if (data)
        Free(data);

    return status;
}

static int _mkdir_command(
    VFAT* vfat,
    int argc, 
    const char* argv[])
{
    int status = 1;
    const char* vfatpath;

    /* Check arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s VFATPATH\n", argv[0]);
        goto done;
    }

    /* Collect arguments */
    vfatpath = argv[1];

    /* Create a directory on the VFAT file system */
    if (VFATMkdir(vfat, vfatpath) != 0)
    {
        fprintf(stderr, "%s: failed to create directrory\n", argv[0]);
        goto done;
    }

    status = 0;

done:

    return status;
}

static int _dir_command(
    VFAT* vfat,
    int argc, 
    const char* argv[])
{
    int status = 1;
    const char* vfatpath;
    StrArr paths = STRARR_INITIALIZER;
    UINTN i;

    /* Check arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s VFATPATH\n", argv[0]);
        goto done;
    }

    /* Collect arguments */
    vfatpath = argv[1];

    /* Get file from VFAT file system */
    if (VFATDir(vfat, vfatpath, &paths) != 0)
    {
        fprintf(stderr, "%s: failed to list directory\n", argv[0]);
        goto done;
    }

    status = 0;

    for (i = 0; i < paths.size; i++)
        printf("%s\n", paths.data[i]);

done:

    return status;
}

#include "../lsvmload/efivfatfs.c"

static int _test_command(
    VFAT* vfat__,
    int argc, 
    const char* argv[])
{
    int status = 1;
    Blkdev* memdev = NULL;
    VFAT* vfat = NULL;

    /* Check arguments */
    if (argc != 1)
    {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        goto done;
    }

    /* Create a memory block device to be used by VFAT */
    if (!(memdev = BlkdevFromMemory(efivfat, efivfat_size)))
    {
        fprintf(stderr, "%s: BlkdevFromMemory() failed\n", argv[0]);
        goto done;
    }

    /* Initialize the VFAT object */
    if (VFATInit(memdev, &vfat) != 0)
    {
        fprintf(stderr, "%s: VFATInit() failed\n", argv[0]);
        goto done;
    }

    /* Create the "EFI" directory */
    if (VFATMkdir(vfat, "/EFI") != 0)
    {
        fprintf(stderr, "%s: VFATMkdir() failed: /EFI\n", argv[0]);
        goto done;
    }

    /* Create the "UBUNTU" directory */
    if (VFATMkdir(vfat, "/EFI/UBUNTU") != 0)
    {
        fprintf(stderr, "%s: VFATMkdir() failed: /EFI/UBUNTU\n", argv[0]);
        goto done;
    }

    /* Get file from VFAT file system */
    {
        StrArr paths = STRARR_INITIALIZER;
        UINTN i;

        if (VFATDir(vfat, "/EFI", &paths) != 0)
        {
            fprintf(stderr, "%s: failed to list directory\n", argv[0]);
            goto done;
        }

        printf("paths{%u}\n", (int)paths.size);

        for (i = 0; i < paths.size; i++)
            printf("file{%s}\n", paths.data[i]);
    }
    
    const char data[] = "#HELLO\n";
    UINTN size = sizeof(data);

    /* Create the "GRUB.CFG" directory */
    if (VFATPutFile(vfat, "/EFI/UBUNTU/GRUB.CFG", data, size) != 0)
    {
        fprintf(stderr, "%s: VFATPutFile() failed\n", argv[0]);
        goto done;
    }

done:

    return status;
}

typedef int (*CommandCallback)(
    VFAT* vfat,
    int argc, 
    const char* argv[]);

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
        "dump", 
        "Dump the EXT2 file system",
        _dump_command,
    },
    {
        "stat", 
        "Stat this VFAT file",
        _stat_command,
    },
    {
        "get", 
        "Get file from VFAT file system",
        _get_command,
    },
    {
        "put", 
        "Put file into VFAT file system",
        _put_command,
    },
    {
        "mkdir", 
        "Create a new directory on the VFAT file system",
        _mkdir_command,
    },
    {
        "dir", 
        "Perform a directory listing",
        _dir_command,
    },
    {
        "test", 
        "Peform testing",
        _test_command,
    },
};

static UINTN _ncommands = sizeof(_commands) / sizeof(_commands[0]);

int vfat_main(
    int argc,
    const char* argv[])
{
    int status = 1;
    const char* vfatfs = NULL;
    Blkdev* dev = NULL;
    VFAT* vfat = NULL;
    UINTN i;
    UINTN offset = 0;

    /* Get the --vfat option (if any) */
    GetOpt(&argc, argv, "--vfatfs", &vfatfs);

    /* Get the -f option (if any) */
    GetOpt(&argc, argv, "-f", &vfatfs);

    /* If no --vfatfs option, fallback on EXT2FS environment variable */
    if (!vfatfs)
    {
        const char* dupenv;

        if (!(dupenv = Dupenv("VFATFS")))
        {
            fprintf(stderr, "%s: either specify --vfatfs option or "
                "define EXT2FS environment variable\n", argv[0]);
            return 1;
        }

        vfatfs = dupenv;
    }

    /* Print usage */
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
        return 1;
    }

    /* If raw disk device (e.g., /dev/sda), find the start offset. */
    if (strlen(vfatfs) >= 1 && !isdigit(vfatfs[strlen(vfatfs)-1]))
    {
        GPT gpt;

        /* Read the GPT */
        if (LoadGPT(vfatfs, &gpt) != 0)
        {
            fprintf(stderr, "%s: failed to load GPT: %s\n", argv[0], argv[1]);
            goto done;
        }

        /* Find the first VFAT */
        for (i = 0; i < GPT_MAX_ENTRIES; i++)
        {
            const GPTEntry* ent = &gpt.entries[i];

           /* EFI GUID is always: C12A7328-F81F-11D2-BA4B-00A0C93EC93B. */
            UINT64 g1 = 0x11d2f81fc12a7328;
            UINT64 g2 = 0x3bc93ec9a0004bba;   

            if (IS_BIG_ENDIAN)
            {
                g1 = 0xc12a7328f81f11d2;
                g2 = ByteSwapU64(g2);
            }    


            if (ent->typeGUID1 == 0)
                break;

            if (g1 == ent->typeGUID1 && g2 == ent->typeGUID2)
            {
                offset = ent->startingLBA * BLKDEV_BLKSIZE;
                break;
            }
        }

        if (offset == 0)
        {
            fprintf(stderr, "%s: failed to resolve partition\n", argv[0]);
            return 1;
        }
    }

    /* Open the block device file */
    if (!(dev = BlkdevOpen(vfatfs, BLKDEV_ACCESS_RDWR, offset)))
    {
        fprintf(stderr, "%s: failed to open: %s\n", argv[0], vfatfs);
        status = 1;
        goto done;
    }

    /* Create the VFAT object */
    if (VFATInit(dev, &vfat) != 0)
    {
        fprintf(stderr, "%s: VFAInit() failed\n", argv[0]);
        goto done;
    }

    /* Find and execute the command given by argv[1] */
    for (i = 0; i < _ncommands; i++)
    {
        if (strcmp(argv[1], _commands[i].name) == 0)
            return((*_commands[i].callback)(vfat, argc-1, argv+1));
    }

    fprintf(stderr, "%s: unknown command: '%s'\n", argv[0], argv[1]);
    status = 1;

done:

    if (vfat)
        VFATRelease(vfat);

    return status;
}

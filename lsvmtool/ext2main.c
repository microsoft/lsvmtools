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
#include <lsvmutils/ext2.h>
#include <lsvmutils/utils.h>
#include <lsvmutils/buf.h>
#include <lsvmutils/getopt.h>
#include <lsvmutils/strings.h>
#include <lsvmutils/luks.h>
#include <lsvmutils/luksblkdev.h>
#include <lsvmutils/cacheblkdev.h>
#include <lsvmutils/file.h>
#include <lsvmutils/guid.h>

#if defined(__linux__)
# include <unistd.h>
#endif

int WriteWholeFile(
    const char* path,
    const void* data,
    size_t size)
{
    int rc = -1;
    FILE* os;

    if (!(os = Fopen(path, "wb")))
    {
        goto done;
    }

    if (fwrite(data, 1, size, os) != size)
    {
        goto done;
    }

    rc = 0;

done:

    if (os)
        fclose(os);

    return rc;
}

static int _dump_command(
    EXT2* ext2,
    int argc, 
    const char* argv[])
{
    int status = 0;
    EXT2Err err;

    /* Check arguments */
    if (argc != 1)
    {
        fprintf(stderr, "Usage: %s PATH\n", argv[0]);
        status = 1;
        goto done;
    }

    /* Print the file system */
    if (EXT2_IFERR(err = EXT2Dump(ext2)))
    {
        fprintf(stderr, "%s: dump failed: %s\n", argv[0], EXT2ErrStr(err));
        status = 1;
        goto done;
    }

done:
    return status;
}

static int _check_command(
    EXT2* ext2,
    int argc, 
    const char* argv[])
{
    int status = 0;
    EXT2Err err;

    /* Check arguments */
    if (argc != 1)
    {
        fprintf(stderr, "Usage: %s PATH\n", argv[0]);
        status = 1;
        goto done;
    }

    /* Print the file system */
    if (EXT2_IFERR(err = EXT2Check(ext2)))
    {
        fprintf(stderr, "%s: check failed: %s\n", argv[0], EXT2ErrStr(err));
        status = 1;
        goto done;
    }

done:
    return status;
}

static int _ls_command(
    EXT2* ext2,
    int argc, 
    const char* argv[])
{
    int status = 0;
    EXT2_DIR* dir = NULL;
    EXT2DirEnt* ent;

    /* Check arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s PATH\n", argv[0]);
        status = 1;
        goto done;
    }

    /* Open the directory */
    if (!(dir = EXT2OpenDir(ext2, argv[1])))
    {
        fprintf(stderr, "%s: failed to open directory\n", argv[0]);
        status = 1;
        goto done;
    }

    /* For each entry */
    while ((ent = EXT2ReadDir(dir)))
    {
        printf("%s\n", ent->d_name);
    }

done:

    if (dir)
        EXT2CloseDir(dir);

    return status;
}

static int _lsr_command(
    EXT2* ext2,
    int argc, 
    const char* argv[])
{
    int status = 0;
    StrArr paths = STRARR_INITIALIZER;
    int i;

    /* Check arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s PATH\n", argv[0]);
        status = 1;
        goto done;
    }

    if (EXT2Lsr(ext2, argv[1], &paths) != 0)
    {
        fprintf(stderr, "%s: lsr failed\n", argv[0]);
        status = 1;
        goto done;
    }

    for (i = 0; i < paths.size; i++)
        printf("%s\n", (char*)paths.data[i]);

    StrArrRelease(&paths);

done:

    return status;
}

static int _get_command(
    EXT2* ext2,
    int argc, 
    const char* argv[])
{
    int status = 0;
    EXT2Inode inode;
    void* data = NULL;
    UINT32 size = 0;
    const char* arg0;
    const char* ext2_path;
    const char* path;
    EXT2Err err;

    /* Check arguments */
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s EXT2PATH PATH\n", argv[0]);
        status = 1;
        goto done;
    }

    /* Collect arguments */
    arg0 = argv[0];
    ext2_path = argv[1];
    path = argv[2];

    /* Convert the path into an inode */
    if (EXT2_IFERR(err = EXT2PathToInode(ext2, ext2_path, NULL, &inode)))
    {
        fprintf(stderr, "%s: failed to resolve path: %s\n", arg0,
            EXT2ErrStr(err));
        status = 1;
        goto done;
    }

    /* Load the file into memory */
    if (EXT2_IFERR(err = EXT2LoadFileFromInode(ext2, &inode, &data, &size)))
    {
        fprintf(stderr, "%s: failed load file: %s\n", arg0, EXT2ErrStr(err));
        status = 1;
        goto done;
    }

    /* Write to basename */
    if (WriteWholeFile(path, data, size) != 0)
    {
        fprintf(stderr, "%s: failed to write file: %s\n", arg0, argv[2]);
        status = 1;
        goto done;
    }

done:

    if (data)
        free(data);

    return status;
}

static int _put_command(
    EXT2* ext2,
    int argc, 
    const char* argv[])
{
    int status = 0;
    void* data = NULL;
    UINT32 size = 0;
    EXT2Err err;

    /* Check arguments */
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s PATH EXT2PATH\n", argv[0]);
        status = 1;
        goto done;
    }

    /* Load file into memory */
    if (EXT2_IFERR(err = EXT2LoadFile(argv[1], &data, &size)))
    {
        fprintf(stderr, "%s: failed to load file: %s: %s\n", argv[0], 
            argv[1], EXT2ErrStr(err));
        status = 1;
        goto done;
    }

    /* Copy the file */
    if (EXT2_IFERR(err = EXT2Put(
        ext2, 
        data, 
        size, 
        argv[2], 
        EXT2_FILE_MODE_RW0_R00_R00)))
    {
        fprintf(stderr, "%s: put failed: %s: %s\n", argv[0], 
            EXT2ErrStr(err), argv[1]);
        status = 1;
        goto done;
    }

done:

    if (data)
        free(data);

    return status;
}

static int _putget_command(
    EXT2* ext2,
    int argc, 
    const char* argv[])
{
    int status = 1;
    void* data = NULL;
    UINT32 size = 0;
    void* data2 = NULL;
    UINT32 size2 = 0;
    EXT2Err err;
    const char* path;
    const char* ext2_path;
    const char* outfile;
    EXT2Inode inode;

    /* Check arguments */
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s PATH EXT2PATH OUTFILE\n", argv[0]);
        goto done;
    }

    /* Collect arguments */
    path = argv[1];
    ext2_path = argv[2];
    outfile = argv[3];

    /* Load file into memory */
    if (EXT2_IFERR(err = EXT2LoadFile(path, &data, &size)))
    {
        fprintf(stderr, "%s: failed to load file: %s: %s\n", argv[0], 
            path, EXT2ErrStr(err));
        goto done;
    }

    /* Remove the file (it may or may not exist) */
    EXT2Rm(ext2, ext2_path);

    /* Create the file */
    if (EXT2_IFERR(err = EXT2Put(
        ext2, 
        data, 
        size, 
        ext2_path,
        EXT2_FILE_MODE_RW0_R00_R00)))
    {
        fprintf(stderr, "%s: put failed: %s: %s\n", argv[0], 
            EXT2ErrStr(err), path);
        goto done;
    }

    /* Convert the path into an inode */
    if (EXT2_IFERR(err = EXT2PathToInode(ext2, ext2_path, NULL, &inode)))
    {
        fprintf(stderr, "%s: failed to resolve path: %s\n", argv[0],
            EXT2ErrStr(err));
        goto done;
    }

    /* Load the file into memory */
    if (EXT2_IFERR(err = EXT2LoadFileFromInode(ext2, &inode, &data2, &size2)))
    {
        fprintf(stderr, "%s: failed load file: %s\n", argv[0], EXT2ErrStr(err));
        goto done;
    }

    /* Write the file back */
    if (WriteWholeFile(outfile, data2, size2) != 0)
    {
        fprintf(stderr, "%s: failed to write file: %s\n", argv[0], outfile);
        goto done;
    }

    status = 0;

done:

    if (data)
        free(data);

    if (data2)
        free(data2);

    return status;
}

static int _rm_command(
    EXT2* ext2,
    int argc, 
    const char* argv[])
{
    int status = 0;
    EXT2Err err;

    /* Check arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s EXT2PATH\n", argv[0]);
        status = 1;
        goto done;
    }

    /* Remove the file */
    if (EXT2_IFERR(err = EXT2Rm(ext2, argv[1])))
    {
        fprintf(stderr, "%s: remove failed: %s\n", argv[0],
            EXT2ErrStr(err));
        status = 1;
        goto done;
    }

done:

    return status;
}

static int _inode_command(
    EXT2* ext2,
    int argc, 
    const char* argv[])
{
    int status = 0;
    EXT2Inode inode;
    EXT2Err err;

    /* Check arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s PATH\n", argv[0]);
        status = 1;
        goto done;
    }

    /* Convert the path into an inode */
    if (EXT2_IFERR(err = EXT2PathToInode(ext2, argv[1], NULL, &inode)))
    {
        fprintf(stderr, "%s: failed to resolve path: %s\n", argv[0],
            EXT2ErrStr(err));
        status = 1;
        goto done;
    }

    /* Dump the inode */
    EXT2DumpInode(ext2, &inode);

done:

    return status;
}

static int _update_command(
    EXT2* ext2,
    int argc, 
    const char* argv[])
{
    int status = 0;
    void* data = NULL;
    UINT32 size = 0;
    EXT2Err err;

    /* Check arguments */
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s PATH EXT2PATH\n", argv[0]);
        status = 1;
        goto done;
    }

    /* Load file into memory */
    if (EXT2_IFERR(err = EXT2LoadFile(argv[1], &data, &size)))
    {
        fprintf(stderr, "%s: failed to load file: %s\n", argv[0],
            EXT2ErrStr(err));
        status = 1;
        goto done;
    }

    /* Update the file */
    if (EXT2_IFERR(err = EXT2Update(ext2, data, size, argv[2])))
    {
        fprintf(stderr, "%s: update failed: %s\n", argv[0], EXT2ErrStr(err));
        status = 1;
        goto done;
    }

done:

    if (data)
        free(data);

    return status;
}

static int _trunc_command(
    EXT2* ext2,
    int argc, 
    const char* argv[])
{
    int status = 0;
    EXT2Err err;

    /* Check arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s PATH\n", argv[0]);
        status = 1;
        goto done;
    }

    /* Remove the file */
    if (EXT2_IFERR(err = EXT2Trunc(ext2, argv[1])))
    {
        fprintf(stderr, "%s: truncate failed: %s\n", argv[0],
            EXT2ErrStr(err));
        status = 1;
        goto done;
    }

done:

    return status;
}

static int _mkdir_command(
    EXT2* ext2,
    int argc, 
    const char* argv[])
{
    int status = 0;
    EXT2Err err;

    /* Check arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s EXT2PATH\n", argv[0]);
        status = 1;
        goto done;
    }

    /* Create the directory */
    if (EXT2_IFERR(err = EXT2MkDir(ext2, argv[1], EXT2_DIR_MODE_RWX_R0X_R0X)))
    {
        fprintf(stderr, "%s: mkdir failed: %s\n", argv[0], EXT2ErrStr(err));
        status = 1;
        goto done;
    }

done:

    return status;
}

static int _hashdir_command(
    EXT2* ext2,
    int argc, 
    const char* argv[])
{
    int status = 0;
    EXT2Err err;
    SHA1Hash sha1;
    SHA256Hash sha256;

    /* Check arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s DIRNAME\n", argv[0]);
        status = 1;
        goto done;
    }

    /* Hash the contents of this directory */
    if (EXT2_IFERR(err = EXT2HashDir(
        ext2, 
        argv[1], 
        &sha1,
        &sha256)))
    {
        fprintf(stderr, "%s: EXT2HashDir() failed\n", argv[0]);
        status = 1;
        goto done;
    }

    {
        SHA1Str sha1str = SHA1ToStr(&sha1);
        printf("%s\n", sha1str.buf);
        SHA256Str sha256str = SHA256ToStr(&sha256);
        printf("%s\n", sha256str.buf);
    }

done:

    return status;
}

static int _buftest_command(
    EXT2* ext2,
    int argc, 
    const char* argv[])
{
    int status = 0;
    Buf buf = BUF_INITIALIZER;
    unsigned int i;
    const unsigned int N = 100000;

    /* Check arguments */
    if (argc != 1)
    {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        status = 1;
        goto done;
    }

    for (i = 0; i < N; i++)
    {
        if (BufAppend(&buf, &i, sizeof(i)) != 0)
        {
            fprintf(stderr, "%s: BufAppend() failed\n", argv[0]);
            exit(1);
        }
    }

    for (i = 0; i < N; i++)
    {
        unsigned int x = ((const unsigned int*)(buf.data))[i];
        if (x != i)
        {
            fprintf(stderr, "%s: data check failed\n", argv[0]);
            exit(1);
        }
    }

    BufRelease(&buf);

done:

    return status;
}

static int _fread_command(
    EXT2* ext2,
    int argc, 
    const char* argv[])
{
    int status = 0;
    EXT2File* file = NULL;
    UINT8 buffer[7];
    INTN n;
    Buf buf = BUF_INITIALIZER;

    /* Check arguments */
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s EXT2FILENAME LOCALFILENAME\n", argv[0]);
        status = 1;
        goto done;
    }

    /* Open the input file */
    if (!(file = EXT2OpenFile(ext2, argv[1], EXT2FILE_RDONLY)))
    {
        fprintf(stderr, "%s: failed to open: %s\n", argv[0], argv[1]);
        status = 1;
        goto done;
    }

    /* Read the file */
    while ((n = EXT2ReadFile(file, buffer, sizeof(buffer))) > 0)
    {
        BufAppend(&buf, buffer, n);
    }

    /* Write the file */
    if (PutFile(argv[2], buf.data, buf.size) != 0)
    {
        fprintf(stderr, "%s: failed to write file: %s", argv[0], argv[1]);
        status = 1;
        goto done;
    }

done:

    if (file)
        EXT2CloseFile(file);

    return status;
}

static int _whose_command(
    EXT2* ext2,
    int argc, 
    const char* argv[])
{
    int status = 0;
    unsigned long blkno;
    char path[EXT2_PATH_MAX];
    BOOLEAN found = FALSE;

    /* Check arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s BLKNO\n", argv[0]);
        status = 1;
        goto done;
    }

    /* Convert the BLKNO argument to integer */
    {
        char* end;
        blkno = strtoul(argv[1], &end, 10);

        if (!end || *end != '\0')
        {
            fprintf(stderr, "%s: bad argument: %s\n", argv[0], argv[1]);
            status = 1;
            goto done;
        }
    }

    /* Find the owner of this block */
    if (EXT2WhoseBlock(ext2, blkno, &found, path) != EXT2_ERR_NONE)
    {
        fprintf(stderr, "%s: EXT2WhoseBlock() failed\n", argv[0]);
        status = 1;
        goto done;
    }

    if (!found)
        printf("not found\n");
    else
        printf("%s\n", path);

done:

    return status;
}

static int _blocks_command(
    EXT2* ext2,
    int argc, 
    const char* argv[])
{
    int status = 0;
    BufU32 blknos = BUF_U32_INITIALIZER;
    UINTN i;

    /* Check arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s PATH\n", argv[0]);
        status = 1;
        goto done;
    }

    /* Find the owner of this block */
    if (EXT2GetBlockNumbers(ext2, argv[1], &blknos) != EXT2_ERR_NONE)
    {
        fprintf(stderr, "%s: EXT2WhoseBlock() failed\n", argv[0]);
        status = 1;
        goto done;
    }

    /* Print the block numbers */
    for (i = 0; i < blknos.size; i++)
    {
        printf("%u\n", blknos.data[i]);
    }

    printf("\n");

done:

    return status;
}

static int _uuid_command(
    EXT2* ext2,
    int argc, 
    const char* argv[])
{
    int status = 1;
    EFI_GUID guid;
    char buf[GUID_STRING_SIZE];

    /* Check arguments */
    if (argc != 1)
    {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        goto done;
    }

    MakeGUIDFromBytes(&guid, ext2->sb.s_uuid);
    FormatGUID(buf, &guid);
    printf("%s\n", buf);

    status = 0;

done:

    return status;
}

typedef int (*CommandCallback)(
    EXT2* ext2,
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
        "check", 
        "Check the EXT2 file system",
        _check_command,
    },
    {
        "ls", 
        "List directory contents",
        _ls_command,
    },
    {
        "lsr", 
        "Recursively list directory contents",
        _lsr_command,
    },
    {
        "get", 
        "Get the given file",
        _get_command,
    },
    {
        "put", 
        "Put the given file",
        _put_command,
    },
    {
        "putget", 
        "Put the given file and then read it back",
        _putget_command,
    },
    {
        "rm", 
        "Remove the given file",
        _rm_command,
    },
    {
        "inode", 
        "Dump the contents of inode for the given file",
        _inode_command,
    },
    {
        "trunc", 
        "Truncate the given file",
        _trunc_command,
    },
    {
        "update", 
        "Update the given file with new contents",
        _update_command,
    },
    {
        "mkdir", 
        "Create a new directory",
        _mkdir_command,
    },
    {
        "hashdir", 
        "Compute the hash of the given directrory",
        _hashdir_command,
    },
    {
        "buftest", 
        "Test buffer allocation",
        _buftest_command,
    },
    {
        "fread", 
        "Test reading of a file using EXT2File objects",
        _fread_command,
    },
    {
        "whose", 
        "Determine which file owns a block",
        _whose_command,
    },
    {
        "blocks", 
        "Print out the blocks used by this file",
        _blocks_command,
    },
    {
        "uuid", 
        "Print out the UUID of this EXT2 file system",
        _uuid_command,
    },
};

static size_t _ncommands = sizeof(_commands) / sizeof(_commands[0]);

int ext2_main(
    int argc,
    const char* argv[])
{
    int status = 0;
    EXT2* ext2 = NULL;
    const char* ext2fs = NULL;
    char* dupenv = NULL;
    EXT2Err err;
    int i;      
    Blkdev* rawdev = NULL;
    Blkdev* cachedev = NULL;
    Blkdev* dev = NULL;
    const char* keyfile = NULL;
    const char* masterkeyfile = NULL;
    char* passphrase = NULL;
    size_t passphraseSize = 0;
    UINT8* masterkey = NULL;
    size_t masterkeySize;
    BOOLEAN cached = FALSE;
        
    /* Get the --keyfile option (if any) */
    GetOpt(&argc, argv, "--keyfile", &keyfile);

    /* Get the --masterkeyfile option (if any) */
    GetOpt(&argc, argv, "--masterkeyfile", &masterkeyfile);

    /* Get the --ext2fs option (if any) */
    GetOpt(&argc, argv, "--ext2fs", &ext2fs);
    GetOpt(&argc, argv, "-f", &ext2fs);

    /* Get the --passphrase option (if any) */
    {
        const char* arg = NULL;
        GetOpt(&argc, argv, "--passphrase", &arg);
        GetOpt(&argc, argv, "-p", &arg);
        passphrase = (char*)arg;
    }

    /* Get the --cached option */
    if (GetOpt(&argc, argv, "--cached", NULL) == 1)
        cached = TRUE;

    /* If no --ext2fs option, fallback on EXT2FS environment variable */
    if (!ext2fs)
    {
        if (!(dupenv = Dupenv("EXT2FS")))
        {
            fprintf(stderr, "%s: either specify --ext2fs option or "
                "define EXT2FS environment variable\n", argv[0]);
            exit(1);
        }

        ext2fs = dupenv;
    }

    /* Load the key file (if any) */
    if (!passphrase && keyfile)
    {
        if (LoadFile(keyfile, 1, (UINT8**)&passphrase, &passphraseSize) != 0)
        {
            fprintf(stderr, "%s: failed to read: %s\n", argv[0], keyfile);
            goto done;
        }

        passphrase[passphraseSize] = '\0';
    }

    /* Load the masterkey file (if any) */
    if (masterkeyfile)
    {
        if (LoadFile(masterkeyfile, 0, &masterkey, &masterkeySize) != 0)
        {
            fprintf(stderr, "%s: failed to read: %s\n", argv[0], masterkeyfile);
            goto done;
        }
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
        exit(1);
    }

    /* Open the block device file */
    if (!(rawdev = BlkdevOpen(ext2fs, BLKDEV_ACCESS_RDWR, 0)))
    {
        fprintf(stderr, "%s: failed to open: %s\n", argv[0], ext2fs);
        status = 1;
        goto done;
    }

    /* Inject cached device into stack */
    if (!(cachedev = NewCacheBlkdev(rawdev)))
    {
        fprintf(stderr, "%s: failed to create cahced block device", argv[0]);
        status = 1;
        goto done;
    }

    /* Enable caching if --cached option given */
    if (cached)
        cachedev->SetFlags(cachedev, BLKDEV_ENABLE_CACHING);

    /* Create ext2_file_t object from LUKS or raw device */
    if (IsRawLUKSDevice(cachedev))
    {
        Blkdev* luksdev;

        if (masterkey)
        {
            if (!(luksdev = LUKSBlkdevFromMasterkey(
                cachedev, 
                masterkey,
                masterkeySize)))
            {
                fprintf(stderr, "%s: cannot create LUKS block device\n", 
                    argv[0]);
                status = 1;
                goto done;
            }
        }
        else
        {
            if (!passphrase)
            {
#if defined(__linux__)
                if (!(passphrase = getpass("passphrase: ")))
                {
                    fprintf(stderr, "%s: failed to get passphrase\n", argv[0]);
                    status = 1;
                    goto done;
                }
#else /* !defined(__linux__) */
                fprintf(stderr, "%s: use --passphrase to provide passphrase",
                    argv[0]);
                exit(1);
#endif /* !defined(__linux__) */
            }

            if (!(luksdev = LUKSBlkdevFromPassphrase(cachedev, passphrase)))
            {
                fprintf(stderr, "%s: cannot create LUKS block device\n", 
                    argv[0]);
                status = 1;
                goto done;
            }
        }

        /* ext2file -> luksdev -> cachedev -> rawdev */
        dev = luksdev;
    }
    else
    {
        /* ext2file -> cachedev -> rawdev */
        dev = cachedev;
    }

    /* Open the file system (takes ownership of 'file') */
    if (EXT2_IFERR(err = EXT2New(dev, &ext2)))
    {
        fprintf(stderr, "%s: failed to open file system: %s\n", argv[0], 
            EXT2ErrStr(err));
        status = 1;
        goto done;
    }

    /* Find and execute the command given by argv[1] */
    for (i = 0; i < _ncommands; i++)
    {
        if (strcmp(argv[1], _commands[i].name) == 0)
        {
            exit((*_commands[i].callback)(ext2, argc-1, argv+1));
        }
    }

    fprintf(stderr, "%s: unknown command: '%s'\n", argv[0], argv[1]);
    status = 1;

done:

    if (masterkey)
        free(masterkey);

    if (passphrase)
        free(passphrase);

    if (ext2)
        EXT2Delete(ext2);

    if (dupenv)
        free(dupenv);
        
    return status;
}

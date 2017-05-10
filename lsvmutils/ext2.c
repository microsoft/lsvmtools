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
#include "ext2.h"

#if !defined(BUILD_EFI)
# include <stdio.h>
# include <stdlib.h>
# include <assert.h>
# include <string.h>
#endif /* defined(BUILD_EFI) */

#include <time.h>
#include "utils.h"
#include "buf.h"
#include "strings.h"
#include "strarr.h"
#include "alloc.h"
#include "print.h"

#if 1
# define EXT2_DECLARE_ERR(ERR) EXT2Err ERR = EXT2_ERR_FAILED
#else
# define EXT2_DECLARE_ERR(ERR) EXT2Err ERR; /* uninitialized */
#endif

/*
**==============================================================================
**
** tracing:
**
**==============================================================================
*/

#define TRACE printf("TRACE: %s(%u)\n", __FILE__, __LINE__)

#if 0
#define TRACE_GOTOS
#endif

#if defined(BUILD_EFI)
# define GOTO_PRINT \
    Print(L"GOTO: %a(%d): %s()\n", __FILE__, __LINE__, __FUNCTION__);
#else
# define GOTO_PRINT \
    printf("GOTO: %s(%d): %s()\n", __FILE__, __LINE__, __FUNCTION__);
#endif

#if defined(TRACE_GOTOS)
# define GOTO(LABEL) \
    do \
    { \
        GOTO_PRINT; \
        goto LABEL; \
    } \
    while (0)
#else
# define GOTO(LABEL) goto LABEL
#endif

/*
**==============================================================================
**
** local definitions:
**
**==============================================================================
*/

static __inline UINT32 _NextMult(UINT32 x, UINT32 m)
{
    return (x + m - 1) / m * m;
}

static __inline UINT32 _Min(UINT32 x, UINT32 y)
{
    return x < y ? x : y;
}

# if !defined(BUILD_EFI)
static void _HexDump(
    const UINT8* data,
    UINT32 size,
    BOOLEAN printables)
{
    UINT32 i;

    printf("%u bytes\n", size);

    for (i = 0; i < size; i++)
    {
        unsigned char c = data[i];

        if (printables && (c >= ' ' && c < '~'))
            printf("'%c", c);
        else
            printf("%02X", c);

        if ((i + 1) % 16)
        {
            printf(" ");
        }
        else
        {
            printf("\n");
        }
    }

    printf("\n");
}
# endif /* defined(BUILD_EFI) */

#if defined(_WIN32)
typedef unsigned long ssize_t;
#endif

static ssize_t _Read(
    Blkdev* dev,
    size_t offset,
    void* data,
    size_t size)
{
    UINT32 blkno;
    UINT32 i;
    UINT32 rem;
    UINT8* ptr;

    if (!dev || !data)
        return -1;

    blkno = offset / BLKDEV_BLKSIZE;

    for (i = blkno, rem = size, ptr = (UINT8*)data; rem; i++)
    {
        UINT8 blk[BLKDEV_BLKSIZE];
        UINT32 off; /* offset into this block */
        UINT32 len; /* bytes to read from this block */

        if (dev->Get(dev, i, blk) != 0)
            return -1;

        /* If first block */
        if (i == blkno)
            off = offset % BLKDEV_BLKSIZE;
        else
            off = 0;

        len = BLKDEV_BLKSIZE - off;

        if (len > rem)
            len = rem;

        Memcpy(ptr, &blk[off], len);
        rem -= len;
        ptr += len;
    }

    return size;
}

static ssize_t _Write(
    Blkdev* dev,
    size_t offset,
    const void* data,
    size_t size)
{
    UINT32 blkno;
    UINT32 i;
    UINT32 rem;
    UINT8* ptr;

    if (!dev || !data)
        return -1;

    blkno = offset / BLKDEV_BLKSIZE;

    for (i = blkno, rem = size, ptr = (UINT8*)data; rem; i++)
    {
        UINT8 blk[BLKDEV_BLKSIZE];
        UINT32 off; /* offset into this block */
        UINT32 len; /* bytes to write from this block */

        /* Fetch the block */
        if (dev->Get(dev, i, blk) != 0)
            return -1;

        /* If first block */
        if (i == blkno)
            off = offset % BLKDEV_BLKSIZE;
        else
            off = 0;

        len = BLKDEV_BLKSIZE - off;

        if (len > rem)
            len = rem;

        Memcpy(&blk[off], ptr, len);
        rem -= len;
        ptr += len;

        /* Rewrite the block */
        if (dev->Put(dev, i, blk) != 0)
            return -1;
    }

    return size;
}

# if !defined(BUILD_EFI)
static void _DumpBlockNumbers(
    const UINT32* data,
    UINT32 size)
{
    UINT32 i;

    printf("%u blocks\n", size);

    for (i = 0; i < size; i++)
    {
        printf("%08X", data[i]);

        if ((i + 1) % 8)
        {
            printf(" ");
        }
        else
        {
            printf("\n");
        }
    }

    printf("\n");
}
# endif /* defined(BUILD_EFI) */

# if !defined(BUILD_EFI)
static void _ASCII_Dump(
    const UINT8* data,
    UINT32 size)
{
    UINT32 i;

    printf("=== ASCII dump:\n");

    for (i = 0; i < size; i++)
    {
        unsigned char c = data[i];

        if (c >= ' ' && c <= '~')
            printf("%c", c);
        else
            printf(".");

        if (i + 1 != size && !((i + 1) % 80))
            printf("\n");
    }

    printf("\n");
}
# endif /* defined(BUILD_EFI) */

# if !defined(BUILD_EFI)
static BOOLEAN _zero_filled(
    const UINT8* data,
    UINT32 size)
{
    UINT32 i;

    for (i = 0; i < size; i++)
    {
        if (data[i])
            return 0;
    }

    return 1;
}
# endif /* defined(BUILD_EFI) */

static UINT32 _CountBits(
    UINT8 byte)
{
    UINT8 i;
    UINT8 n = 0;

    for (i = 0; i < 8; i++)
    {
        if (byte & (1 << i))
            n++;
    }

    return n;
}

static UINT32 _CountBitsN(
    const UINT8* data,
    UINT32 size)
{
    UINT32 i;
    UINT32 n = 0;

    for (i = 0; i < size; i++)
    {
        n += _CountBits(data[i]);
    }

    return n;
}

static __inline BOOLEAN _TestBit(
    const UINT8* data,
    UINT32 size,
    UINT32 index)
{
    UINT32 byte = index / 8;
    UINT32 bit = index % 8;

    if (byte >= size)
        return 0;

    return ((UINT32)(data[byte]) & (1 << bit)) ? 1 : 0;
}

static __inline void _SetBit(
    UINT8* data,
    UINT32 size,
    UINT32 index)
{
    UINT32 byte = index / 8;
    UINT32 bit = index % 8;

    if (byte >= size)
        return;

    data[byte] |= (1 << bit);
}

static __inline void _ClearBit(
    UINT8* data,
    UINT32 size,
    UINT32 index)
{
    UINT32 byte = index / 8;
    UINT32 bit = index % 8;

    if (byte >= size)
        return;

    data[byte] &= ~(1 << bit);
}

# if !defined(BUILD_EFI)
static void _dump_bitmap(
    const EXT2Block* block)
{
    if (_zero_filled(block->data, block->size))
    {
        printf("...\n\n");
    }
    else
    {
        _HexDump(block->data, block->size, 0);
    }
}
# endif /* defined(BUILD_EFI) */

static BOOLEAN _IsBigEndian()
{
    union
    {
        unsigned short x;
        unsigned char bytes[2];
    }
    u;
    u.x = 0xABCD;
    return u.bytes[0] == 0xAB ? 1 : 0;
}

# if !defined(BUILD_EFI)
EXT2Err EXT2LoadFile(
    const char* path, 
    void** data,
    UINT32* size)
{
    EXT2_DECLARE_ERR(err);
    FILE* is = NULL;
    Buf buf = BUF_INITIALIZER;

    /* Clear output parameters */
    *data = NULL;
    *size = 0;

    /* Open the file */
    if (!(is = Fopen(path, "rb")))
    {
        err = EXT2_ERR_OPEN_FAILED;
        GOTO(done);
    }

    /* Read file into memory */
    for (;;)
    {
        char tmp[4096];
        size_t n;

        n = fread(tmp, 1, sizeof(tmp), is);

        if (n <= 0)
            break;

        if (BufAppend(&buf, tmp, n) != 0)
        {
            GOTO(done);
        }
    }

    *data = buf.data;
    *size = buf.size;

    err = EXT2_ERR_NONE;

done:

    if (is)
        fclose(is);

    if (EXT2_IFERR(err))
        BufRelease(&buf);

    return err;
}
# endif /* !defined(BUILD_EFI) */ 

/*
**==============================================================================
**
** errors:
**
**==============================================================================
*/

#if defined(EXT2_USE_TYPESAFE_ERRORS)
# define EXT2_DEFINE_ERR(TAG, NUM) const EXT2Err TAG = { NUM };

EXT2_DEFINE_ERR(EXT2_ERR_NONE, 0);
EXT2_DEFINE_ERR(EXT2_ERR_FAILED, 1);
EXT2_DEFINE_ERR(EXT2_ERR_INVALID_PARAMETER, 2);
EXT2_DEFINE_ERR(EXT2_ERR_FILE_NOT_FOUND, 3);
EXT2_DEFINE_ERR(EXT2_ERR_BAD_MAGIC, 4);
EXT2_DEFINE_ERR(EXT2_ERR_UNSUPPORTED, 5);
EXT2_DEFINE_ERR(EXT2_ERR_OUT_OF_MEMORY, 6);
EXT2_DEFINE_ERR(EXT2_ERR_FAILED_TO_READ_SUPERBLOCK, 7);
EXT2_DEFINE_ERR(EXT2_ERR_FAILED_TO_READ_GROUPS, 8);
EXT2_DEFINE_ERR(EXT2_ERR_FAILED_TO_READ_INODE, 9);
EXT2_DEFINE_ERR(EXT2_ERR_UNSUPPORTED_REVISION, 10);
EXT2_DEFINE_ERR(EXT2_ERR_OPEN_FAILED, 11);
EXT2_DEFINE_ERR(EXT2_ERR_BUFFER_OVERFLOW, 12);
EXT2_DEFINE_ERR(EXT2_ERR_SEEK_FAILED, 13);
EXT2_DEFINE_ERR(EXT2_ERR_READ_FAILED, 14);
EXT2_DEFINE_ERR(EXT2_ERR_WRITE_FAILED, 15);
EXT2_DEFINE_ERR(EXT2_ERR_UNEXPECTED, 16);
EXT2_DEFINE_ERR(EXT2_ERR_SANITY_CHECK_FAILED, 17);
EXT2_DEFINE_ERR(EXT2_ERR_BAD_BLKNO, 18);
EXT2_DEFINE_ERR(EXT2_ERR_BAD_INO, 19);
EXT2_DEFINE_ERR(EXT2_ERR_BAD_GRPNO, 18);
EXT2_DEFINE_ERR(EXT2_ERR_BAD_MULTIPLE, 19);
EXT2_DEFINE_ERR(EXT2_ERR_EXTRANEOUS_DATA, 20);
EXT2_DEFINE_ERR(EXT2_ERR_BAD_SIZE, 21);
EXT2_DEFINE_ERR(EXT2_ERR_PATH_TOO_LONG, 22);

#endif /* define(EXT2_USE_TYPESAFE_ERRORS) */

const char* EXT2ErrStr(
    EXT2Err err)
{
    if (EXT2_ERRNO(err) == EXT2_ERRNO(EXT2_ERR_NONE))
        return "ok";

    if (EXT2_ERRNO(err) == EXT2_ERRNO(EXT2_ERR_FAILED))
        return "failed";

    if (EXT2_ERRNO(err) == EXT2_ERRNO(EXT2_ERR_INVALID_PARAMETER))
        return "invalid parameter";

    if (EXT2_ERRNO(err) == EXT2_ERRNO(EXT2_ERR_FILE_NOT_FOUND))
        return "file not found";

    if (EXT2_ERRNO(err) == EXT2_ERRNO(EXT2_ERR_BAD_MAGIC))
        return "bad magic";

    if (EXT2_ERRNO(err) == EXT2_ERRNO(EXT2_ERR_UNSUPPORTED))
        return "unsupported";

    if (EXT2_ERRNO(err) == EXT2_ERRNO(EXT2_ERR_OUT_OF_MEMORY))
        return "out of memory";

    if (EXT2_ERRNO(err) == EXT2_ERRNO(EXT2_ERR_FAILED_TO_READ_SUPERBLOCK))
        return "failed to read superblock";

    if (EXT2_ERRNO(err) == EXT2_ERRNO(EXT2_ERR_FAILED_TO_READ_GROUPS))
        return "failed to read groups";

    if (EXT2_ERRNO(err) == EXT2_ERRNO(EXT2_ERR_FAILED_TO_READ_INODE))
        return "failed to read inode";

    if (EXT2_ERRNO(err) == EXT2_ERRNO(EXT2_ERR_UNSUPPORTED_REVISION))
        return "unsupported revision";

    if (EXT2_ERRNO(err) == EXT2_ERRNO(EXT2_ERR_OPEN_FAILED))
        return "open failed";

    if (EXT2_ERRNO(err) == EXT2_ERRNO(EXT2_ERR_BUFFER_OVERFLOW))
        return "buffer overflow";

    if (EXT2_ERRNO(err) == EXT2_ERRNO(EXT2_ERR_SEEK_FAILED))
        return "buffer overflow";

    if (EXT2_ERRNO(err) == EXT2_ERRNO(EXT2_ERR_READ_FAILED))
        return "read failed";

    if (EXT2_ERRNO(err) == EXT2_ERRNO(EXT2_ERR_WRITE_FAILED))
        return "write failed";

    if (EXT2_ERRNO(err) == EXT2_ERRNO(EXT2_ERR_UNEXPECTED))
        return "unexpected";

    if (EXT2_ERRNO(err) == EXT2_ERRNO(EXT2_ERR_SANITY_CHECK_FAILED))
        return "sanity check failed";

    if (EXT2_ERRNO(err) == EXT2_ERRNO(EXT2_ERR_BAD_BLKNO))
        return "bad block number";

    if (EXT2_ERRNO(err) == EXT2_ERRNO(EXT2_ERR_BAD_INO))
        return "bad inode number";

    if (EXT2_ERRNO(err) == EXT2_ERRNO(EXT2_ERR_BAD_GRPNO))
        return "bad group number";

    if (EXT2_ERRNO(err) == EXT2_ERRNO(EXT2_ERR_BAD_MULTIPLE))
        return "bad multiple";

    if (EXT2_ERRNO(err) == EXT2_ERRNO(EXT2_ERR_EXTRANEOUS_DATA))
        return "extraneous data";

    if (EXT2_ERRNO(err) == EXT2_ERRNO(EXT2_ERR_BAD_SIZE))
        return "bad size";

    if (EXT2_ERRNO(err) == EXT2_ERRNO(EXT2_ERR_PATH_TOO_LONG))
        return "path too long";

    return "unknown";
}

/*
**==============================================================================
**
** blocks:
**
**==============================================================================
*/

/* Byte offset of this block (block 0 is the null block) */
static UINTN BlockOffset(UINT32 blkno, UINT32 block_size)
{
    return blkno * block_size;
}

static UINT32 MakeBlkno(
    const EXT2* ext2,
    UINT32 grpno,
    UINT32 lblkno)
{
    const UINTN first = ext2->sb.s_first_data_block;
    return (grpno * ext2->sb.s_blocks_per_group) + (lblkno + first);
}

static UINT32 _BloknoToGrpno(
    const EXT2* ext2,
    UINT32 blkno)
{
    const UINTN first = ext2->sb.s_first_data_block;

    if (first && blkno == 0)
        return 0;

    return (blkno - first) / ext2->sb.s_blocks_per_group;
}

static UINT32 _BlknoToLblkno(
    const EXT2* ext2,
    UINT32 blkno)
{
    const UINTN first = ext2->sb.s_first_data_block;

    if (first && blkno == 0)
        return 0;

    return (blkno - first) % ext2->sb.s_blocks_per_group;
}

static EXT2Err _ReadBlocks(
    const EXT2* ext2,
    UINT32 blkno,
    UINT32 nblks,
    Buf* buf)
{
    EXT2_DECLARE_ERR(err);
    UINT32 bytes;

    /* Check for null parameters */
    if (!EXT2Valid(ext2) || !buf)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    /* Expand the allocation to hold the new data */
    {
        bytes = nblks * ext2->block_size;

        if (BufReserve(buf, buf->size + bytes) != 0)
            GOTO(done);
    }

    /* Read the blocks */
    if (_Read(
        ext2->dev,
        BlockOffset(blkno, ext2->block_size),
        (unsigned char*)buf->data + buf->size,
        bytes) != bytes)
    {
        err = EXT2_ERR_READ_FAILED;
        GOTO(done);
    }

    buf->size += bytes;

    err = EXT2_ERR_NONE;

done:

    return err;
}

EXT2Err EXT2ReadBlock(
    const EXT2* ext2,
    UINT32 blkno,
    EXT2Block* block)
{
    EXT2_DECLARE_ERR(err);

    /* Check for null parameters */
    if (!EXT2Valid(ext2) || !block)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    Memset(block, 0xAA, sizeof(EXT2Block));

    /* Is block size too big for buffer? */
    if (ext2->block_size > sizeof(block->data))
    {
        err = EXT2_ERR_BUFFER_OVERFLOW;
        GOTO(done);
    }

    /* Set the size of the block */
    block->size = ext2->block_size;

    /* Read the block */
    if (_Read(
        ext2->dev, 
        BlockOffset(blkno, ext2->block_size),
        block->data, 
        block->size) != block->size)
    {
        err = EXT2_ERR_READ_FAILED;
        GOTO(done);
    }

    err = EXT2_ERR_NONE;

done:

    return err;
}

EXT2Err EXT2WriteBlock(
    const EXT2* ext2,
    UINT32 blkno,
    const EXT2Block* block)
{
    EXT2_DECLARE_ERR(err);

    /* Check for null parameters */
    if (!EXT2Valid(ext2) || !block)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    /* Is block size too big for buffer? */
    if (block->size > ext2->block_size)
    {
        err = EXT2_ERR_BUFFER_OVERFLOW;
        GOTO(done);
    }

    /* Write the block */
    if (_Write(
        ext2->dev, 
        BlockOffset(blkno, ext2->block_size),
        block->data, 
        block->size) != block->size)
    {
        err = EXT2_ERR_WRITE_FAILED;
        GOTO(done);
    }

    err = EXT2_ERR_NONE;

done:

    return err;
}

static EXT2Err _AppendDirectBlockNumbers(
    const UINT32* blocks,
    UINT32 num_blocks,
    BufU32 *buf)
{
    EXT2_DECLARE_ERR(err);
    UINT32 i;
    UINT32 n = 0;

    /* Determine size of blocks array */
    for (i = 0; i < num_blocks && blocks[i]; i++)
        n++;

    if (n == 0)
    {
        err = EXT2_ERR_NONE;
        goto done;
    }

    /* Append the blocks to the 'data' array */
    if (EXT2_IFERR(err = BufU32Append(buf, blocks, n)))
        GOTO(done);

    err = EXT2_ERR_NONE;

done:
    return err;
}

static EXT2Err _AppendIndirectBlockNumbers(
    const EXT2* ext2,
    const UINT32* blocks,
    UINT32 num_blocks,
    BOOLEAN include_block_blocks,
    BufU32 *buf)
{
    EXT2_DECLARE_ERR(err);
    EXT2Block block;
    UINT32 i;

    if (include_block_blocks)
    {
        if (EXT2_IFERR(err = _AppendDirectBlockNumbers(
            blocks, 
            num_blocks, 
            buf)))
        {
            GOTO(done);
        }
    }

    /* Handle the direct blocks */
    for (i = 0; i < num_blocks && blocks[i]; i++)
    {
        UINT32 block_no = blocks[i];

        /* Read the next block */
        if (EXT2_IFERR(err = EXT2ReadBlock(ext2, block_no, &block)))
        {
            GOTO(done);
        }

        if (EXT2_IFERR(err = _AppendDirectBlockNumbers(
            (const UINT32*)block.data,
            block.size / sizeof(UINT32),
            buf)))
        {
            GOTO(done);
        }
    }

    err = EXT2_ERR_NONE;

done:
    return err;
}

static EXT2Err _AppendDoubleIndirectBlockNumbers(
    const EXT2* ext2,
    const UINT32* blocks,
    UINT32 num_blocks,
    BOOLEAN include_block_blocks,
    BufU32 *buf)
{
    EXT2_DECLARE_ERR(err);
    EXT2Block block;
    UINT32 i;

    if (include_block_blocks)
    {
        if (EXT2_IFERR(err = _AppendDirectBlockNumbers(
            blocks, 
            num_blocks, 
            buf)))
        {
            GOTO(done);
        }
    }

    /* Handle the direct blocks */
    for (i = 0; i < num_blocks && blocks[i]; i++)
    {
        UINT32 block_no = blocks[i];

        /* Read the next block */
        if (EXT2_IFERR(err = EXT2ReadBlock(ext2, block_no, &block)))
        {
            GOTO(done);
        }

        if (EXT2_IFERR(err = _AppendIndirectBlockNumbers(
            ext2, 
            (const UINT32*)block.data,
            block.size / sizeof(UINT32),
            include_block_blocks,
            buf)))
        {
            GOTO(done);
        }
    }

    err = EXT2_ERR_NONE;

done:
    return err;
}

static EXT2Err _LoadBlockNumbersFromInode(
    const EXT2* ext2,
    const EXT2Inode* inode,
    BOOLEAN include_block_blocks,
    BufU32 *buf)
{
    EXT2_DECLARE_ERR(err);

    /* Check parameters */
    if (!EXT2Valid(ext2) || !inode || !buf)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    /* Handle the direct blocks */
    if (EXT2_IFERR(err = _AppendDirectBlockNumbers(
        inode->i_block,
        EXT2_SINGLE_INDIRECT_BLOCK,
        buf)))
    {
        GOTO(done);
    }

    /* Handle single-indirect blocks */
    if (inode->i_block[EXT2_SINGLE_INDIRECT_BLOCK])
    {
        EXT2Block block;
        UINT32 block_no = inode->i_block[EXT2_SINGLE_INDIRECT_BLOCK];

        /* Read the next block */
        if (EXT2_IFERR(err = EXT2ReadBlock(ext2, block_no, &block)))
        {
            GOTO(done);
        }

        if (include_block_blocks)
        {
            if (EXT2_IFERR(err = BufU32Append(buf, &block_no, 1)))
            {
                GOTO(done);
            }
        }

        /* Append the block numbers from this block */
        if (EXT2_IFERR(err = _AppendDirectBlockNumbers(
            (const UINT32*)block.data,
            block.size / sizeof(UINT32),
            buf)))
        {
            GOTO(done);
        }
    }

    /* Handle double-indirect blocks */
    if (inode->i_block[EXT2_DOUBLE_INDIRECT_BLOCK])
    {
        EXT2Block block;
        UINT32 block_no = inode->i_block[EXT2_DOUBLE_INDIRECT_BLOCK];

        /* Read the next block */
        if (EXT2_IFERR(err = EXT2ReadBlock(ext2, block_no, &block)))
        {
            GOTO(done);
        }

        if (include_block_blocks)
        {
            if (EXT2_IFERR(err = BufU32Append(buf, &block_no, 1)))
                GOTO(done);
        }

        if (EXT2_IFERR(err = _AppendIndirectBlockNumbers(
            ext2, 
            (const UINT32*)block.data,
            block.size / sizeof(UINT32),
            include_block_blocks,
            buf)))
        {
            GOTO(done);
        }
    }

    /* Handle triple-indirect blocks */
    if (inode->i_block[EXT2_TRIPLE_INDIRECT_BLOCK])
    {
        EXT2Block block;
        UINT32 block_no = inode->i_block[EXT2_TRIPLE_INDIRECT_BLOCK];

        /* Read the next block */
        if (EXT2_IFERR(err = EXT2ReadBlock(ext2, block_no, &block)))
        {
            GOTO(done);
        }

        if (include_block_blocks)
        {
            if (EXT2_IFERR(err = BufU32Append(buf, &block_no, 1)))
                GOTO(done);
        }

        if (EXT2_IFERR(err = _AppendDoubleIndirectBlockNumbers(
            ext2, 
            (const UINT32*)block.data,
            block.size / sizeof(UINT32),
            include_block_blocks,
            buf)))
        {
            GOTO(done);
        }
    }

/* This is broken */
#if 0
    /* Check size expectations */
    if (!include_block_blocks)
    {
        UINT32 expected_size;

        expected_size = inode->i_size / ext2->block_size;

        if (inode->i_size % ext2->block_size)
            expected_size++;

        if (buf->size != expected_size)
        {
            err = EXT2_ERR_SANITY_CHECK_FAILED;
            GOTO(done);
        }
    }
#endif

    err = EXT2_ERR_NONE;

done:

    return err;
}

static EXT2Err _WriteGroup(
    const EXT2* ext2,
    UINT32 grpno);

static EXT2Err _WriteSuperBlock(
    const EXT2* ext2);

static EXT2Err _CheckBlockNumber(
    EXT2* ext2,
    UINT32 blkno,
    UINT32 grpno,
    UINT32 lblkno)
{
    EXT2_DECLARE_ERR(err);

    /* Check parameters */
    if (!ext2)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    /* Sanity check */
    if (MakeBlkno(ext2, grpno, lblkno) != blkno)
    {
        err = EXT2_ERR_BAD_BLKNO;
        GOTO(done);
    }

    /* See if 'grpno' is out of range */
    if (grpno > ext2->group_count)
    {
        err = EXT2_ERR_BAD_GRPNO;
        GOTO(done);
    }

    err = EXT2_ERR_NONE;

done:
    return err;
}

static EXT2Err _WriteGroubWithBitmap(
    EXT2 *ext2,
    UINT32 grpno,
    EXT2Block *bitmap)
{
    EXT2_DECLARE_ERR(err);

    /* Check parameters */
    if (!ext2 || !bitmap)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    /* Write the group */
    if (EXT2_IFERR(err = _WriteGroup(ext2, grpno)))
    {
        GOTO(done);
    }

    /* Write the bitmap */
    if (EXT2_IFERR(err = EXT2WriteBlockBitmap(ext2, grpno, bitmap)))
    {
        GOTO(done);
    }
   
    err = EXT2_ERR_NONE;

done:
    return err;
}

# if !defined(BUILD_EFI)
static int _CompareUint32(
    const void* p1,
    const void* p2)
{
    UINT32 v1 = *(UINT32*)p1;
    UINT32 v2 = *(UINT32*)p2;
    if (v1 < v2)
        return -1;
    else if (v1 > v2)
        return 1;
    return 0;
}
# endif /* !defined(BUILD_EFI) */

# if defined(BUILD_EFI)
static void _Sort(
    UINT32 *blks,
    UINT32 nblks)
{
    UINTN i;
    UINTN j;
    UINTN n;

    n = nblks - 1;

    for (i = 0; i < nblks - 1; i++)
    {
        BOOLEAN swapped = FALSE;

        for (j = 0; j < n; j++)
        {
            if (blks[j] > blks[j+1])
            {
                UINT32 tmp = blks[j];
                blks[j] = blks[j+1];
                blks[j+1] = tmp;
                swapped = TRUE;
            }
        }

        if (!swapped)
            break;

        n--;
    }
}
# endif /* defined(BUILD_EFI) */

static EXT2Err _PutBlocks(
    EXT2* ext2,
    const UINT32* blknos,
    UINT32 nblknos) 
{
    EXT2_DECLARE_ERR(err);
    UINT32 i;
    UINT32 *temp = NULL;

    /* Sort the block numbers, so we can just iterate through the group list
       and make changes there. So we do I/O operations = to the group list 
       rather than the number of blocks.
       NOTE: Hash table should be faster, but not a big deal for small files 
       (initrd). 
    */
    temp = (UINT32*)Malloc(nblknos * sizeof(UINT32));
    if (temp == NULL)
    {
        err = EXT2_ERR_OUT_OF_MEMORY;
        GOTO(done);
    }
    for (i = 0; i < nblknos; i++)
    {
        temp[i] = blknos[i];
    }

#if defined(BUILD_EFI)
    _Sort(temp, nblknos);
#else /* defined(BUILD_EFI) */
    qsort(temp, nblknos, sizeof(UINT32), _CompareUint32);
#endif /* !defined(BUILD_EFI) */

    /* Loop through the groups. */
    EXT2Block bitmap;
    UINT32 prevgrpno = 0;
    for (i = 0; i < nblknos; i++)
    {
        UINT32 grpno = _BloknoToGrpno(ext2, temp[i]);
        UINT32 lblkno = _BlknoToLblkno(ext2, temp[i]);

        if (EXT2_IFERR(err = _CheckBlockNumber(ext2, temp[i], grpno, lblkno)))
        {
            GOTO(done);
        }

        if (i == 0)
        {
            if (EXT2_IFERR(err = EXT2ReadBlockBitmap(ext2, grpno, &bitmap)))
            {
                GOTO(done);
            }
        }
        else if (prevgrpno != grpno)
        {
            /* Advanced to next group so write old bitmap and read new one. */
            if (EXT2_IFERR(err = _WriteGroubWithBitmap(ext2, prevgrpno, &bitmap)))
            {
                GOTO(done);
            }
            if (EXT2_IFERR(err = EXT2ReadBlockBitmap(ext2, grpno, &bitmap)))
            {
                GOTO(done);
            }
        }

        /* Sanity check */
        if (!_TestBit(bitmap.data, bitmap.size, lblkno))
        {
            err = EXT2_ERR_SANITY_CHECK_FAILED;
            GOTO(done);
        }

        /* Update in memory structs. */
        _ClearBit(bitmap.data, bitmap.size, lblkno);
        ext2->sb.s_free_blocks_count++;
        ext2->groups[grpno].bg_free_blocks_count++;
        prevgrpno = grpno;
        
        /* Always write final block. */
        if (i + 1 == nblknos)
        {
            if (EXT2_IFERR(err = _WriteGroubWithBitmap(ext2, grpno, &bitmap)))
            {
                GOTO(done);
            }
        }
    }

    /* Update super block. */
    if (EXT2_IFERR(err = _WriteSuperBlock(ext2)))
    {
        GOTO(done);
    }

    err = EXT2_ERR_NONE;

done:
    Free(temp);
    return err;
}

static EXT2Err _GetBlock(
    EXT2* ext2,
    UINT32* blkno)
{
    EXT2_DECLARE_ERR(err);
    EXT2Block bitmap;
    UINT32 grpno;

    /* Check parameters */
    if (!ext2 || !blkno)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    /* Clear any block number */
    *blkno = 0;

    /* Use brute force search for a free block */
    for (grpno = 0; grpno < ext2->group_count; grpno++)
    {
        UINT32 lblkno;

        /* Read the bitmap */
        
        if (EXT2_IFERR(err = EXT2ReadBlockBitmap(ext2, grpno, &bitmap)))
        {
            GOTO(done);
        }

        /* Scan the bitmap, looking for free bit */
        for (lblkno = 0; lblkno < bitmap.size * 8; lblkno++)
        {
            if (!_TestBit(bitmap.data, bitmap.size, lblkno))
            {
                _SetBit(bitmap.data, bitmap.size, lblkno);
                *blkno = MakeBlkno(ext2, grpno, lblkno);
                break;
            }
        }

        if (*blkno)
            break;
    }

    /* If no free blocks found */
    if (!*blkno)
    {
        GOTO(done);
    }

    /* Write the superblock */
    {
        ext2->sb.s_free_blocks_count--;

        if (EXT2_IFERR(err = _WriteSuperBlock(ext2)))
        {
            GOTO(done);
        }
    }

    /* Write the group */
    {
        ext2->groups[grpno].bg_free_blocks_count--;

        if (EXT2_IFERR(err = _WriteGroup(ext2, grpno)))
        {
            GOTO(done);
        }
    }

    /* Write the bitmap */
    if (EXT2_IFERR(err = EXT2WriteBlockBitmap(ext2, grpno, &bitmap)))
    {
        GOTO(done);
    }

    err = EXT2_ERR_NONE;

done:
    return err;
}

/*
**==============================================================================
**
** super block:
**
**==============================================================================
*/

# if !defined(BUILD_EFI)
void EXT2DumpSuperBlock(const EXT2SuperBlock* sb)
{
    printf("=== EXT2SuperBlock:\n");
    printf("s_inodes_count=%u\n", sb->s_inodes_count);
    printf("s_blocks_count=%u\n", sb->s_blocks_count);
    printf("s_r_blocks_count=%u\n", sb->s_r_blocks_count);
    printf("s_free_blocks_count=%u\n", sb->s_free_blocks_count);
    printf("s_free_inodes_count=%u\n", sb->s_free_inodes_count);
    printf("s_first_data_block=%u\n", sb->s_first_data_block);
    printf("s_log_block_size=%u\n", sb->s_log_block_size);
    printf("s_log_frag_size=%u\n", sb->s_log_frag_size);
    printf("s_blocks_per_group=%u\n", sb->s_blocks_per_group);
    printf("s_frags_per_group=%u\n", sb->s_frags_per_group);
    printf("s_inodes_per_group=%u\n", sb->s_inodes_per_group);
    printf("s_mtime=%u\n", sb->s_mtime);
    printf("s_wtime=%u\n", sb->s_wtime);
    printf("s_mnt_count=%u\n", sb->s_mnt_count);
    printf("s_max_mnt_count=%u\n", sb->s_max_mnt_count);
    printf("s_magic=%X\n", sb->s_magic);
    printf("s_state=%u\n", sb->s_state);
    printf("s_errors=%u\n", sb->s_errors);
    printf("s_minor_rev_level=%u\n", sb->s_minor_rev_level);
    printf("s_lastcheck=%u\n", sb->s_lastcheck);
    printf("s_checkinterval=%u\n", sb->s_checkinterval);
    printf("s_creator_os=%u\n", sb->s_creator_os);
    printf("s_rev_level=%u\n", sb->s_rev_level);
    printf("s_def_resuid=%u\n", sb->s_def_resuid);
    printf("s_def_resgid=%u\n", sb->s_def_resgid);
    printf("s_first_ino=%u\n", sb->s_first_ino);
    printf("s_inode_size=%u\n", sb->s_inode_size);
    printf("s_block_group_nr=%u\n", sb->s_block_group_nr);
    printf("s_feature_compat=%u\n", sb->s_feature_compat);
    printf("s_feature_incompat=%u\n", sb->s_feature_incompat);
    printf("s_feature_ro_compat=%u\n", sb->s_feature_ro_compat);
    printf("s_uuid="
        "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n", 
        sb->s_uuid[0], sb->s_uuid[1], sb->s_uuid[2], sb->s_uuid[3], 
        sb->s_uuid[4], sb->s_uuid[5], sb->s_uuid[6], sb->s_uuid[7], 
        sb->s_uuid[8], sb->s_uuid[9], sb->s_uuid[10], sb->s_uuid[11],
        sb->s_uuid[12], sb->s_uuid[13], sb->s_uuid[14], sb->s_uuid[15]);
    printf("s_volume_name=%s\n", sb->s_volume_name);
    printf("s_last_mounted=%s\n", sb->s_last_mounted);
    printf("s_algo_bitmap=%u\n", sb->s_algo_bitmap);
    printf("s_prealloc_blocks=%u\n", sb->s_prealloc_blocks);
    printf("s_prealloc_dir_blocks=%u\n", sb->s_prealloc_dir_blocks);
    printf("s_journal_uuid="
        "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n", 
        sb->s_journal_uuid[0], sb->s_journal_uuid[1], sb->s_journal_uuid[2],
        sb->s_journal_uuid[3], sb->s_journal_uuid[4], sb->s_journal_uuid[5],
        sb->s_journal_uuid[6], sb->s_journal_uuid[7], sb->s_journal_uuid[8],
        sb->s_journal_uuid[9], sb->s_journal_uuid[10], sb->s_journal_uuid[11],
        sb->s_journal_uuid[12], sb->s_journal_uuid[13], sb->s_journal_uuid[14],
        sb->s_journal_uuid[15]);
    printf("s_journal_inum=%u\n", sb->s_journal_inum);
    printf("s_journal_dev=%u\n", sb->s_journal_dev);
    printf("s_last_orphan=%u\n", sb->s_last_orphan);
    printf("s_hash_seed={%02X,%02X,%02X,%02X}\n", 
        sb->s_hash_seed[0], sb->s_hash_seed[1], 
        sb->s_hash_seed[2], sb->s_hash_seed[3]);
    printf("s_def_hash_version=%u\n", sb->s_def_hash_version);
    printf("s_default_mount_options=%u\n", sb->s_default_mount_options);
    printf("s_first_meta_bg=%u\n", sb->s_first_meta_bg);
    printf("\n");
}
# endif /* !defined(BUILD_EFI) */

static EXT2Err _ReadSuperBlock(
    Blkdev* dev, 
    EXT2SuperBlock* sb)
{
    EXT2_DECLARE_ERR(err);

    /* Read the superblock */
    if (_Read(
        dev,
        EXT2_BASE_OFFSET,
        sb,
        sizeof(EXT2SuperBlock)) != sizeof(EXT2SuperBlock))
    {
        GOTO(done);
    }

    err = EXT2_ERR_NONE;

done:
    return err;
}

static EXT2Err _WriteSuperBlock(const EXT2* ext2)
{
    EXT2_DECLARE_ERR(err);

    /* Read the superblock */
    if (_Write(
        ext2->dev, 
        EXT2_BASE_OFFSET,
        &ext2->sb, 
        sizeof(EXT2SuperBlock)) != sizeof(EXT2SuperBlock))
    {
        GOTO(done);
    }

    err = EXT2_ERR_NONE;

done:
    return err;
}

/*
**==============================================================================
**
** groups:
**
**==============================================================================
*/

# if !defined(BUILD_EFI)
static void _DumpGroupDesc(const EXT2GroupDesc* gd)
{
    printf("=== EXT2GroupDesc\n");
    printf("bg_block_bitmap=%u\n", gd->bg_block_bitmap);
    printf("bg_inode_bitmap=%u\n", gd->bg_inode_bitmap);
    printf("bg_inode_table=%u\n", gd->bg_inode_table);
    printf("bg_free_blocks_count=%u\n", gd->bg_free_blocks_count);
    printf("bg_free_inodes_count=%u\n", gd->bg_free_inodes_count);
    printf("bg_used_dirs_count=%u\n", gd->bg_used_dirs_count);
    printf("\n");
}
# endif /* !defined(BUILD_EFI) */

# if !defined(BUILD_EFI)
static void _DumpGroupDescs(
    const EXT2GroupDesc* groups,
    UINT32 group_count)
{
    const EXT2GroupDesc* p = groups;
    const EXT2GroupDesc* end = groups + group_count;

    while (p != end)
    {
        _DumpGroupDesc(p);
        p++;
    }
}
# endif /* !defined(BUILD_EFI) */

static EXT2GroupDesc* _ReadGroups(
    const EXT2* ext2)
{
    EXT2_DECLARE_ERR(err);
    EXT2GroupDesc* groups = NULL;
    UINT32 groups_size = 0;
    UINT32 blkno;

    /* Check the file system argument */
    if (!EXT2Valid(ext2))
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    /* Allocate the groups list */
    {
        groups_size = ext2->group_count * sizeof(EXT2GroupDesc);

        if (!(groups = (EXT2GroupDesc*)Malloc(groups_size)))
        {
            GOTO(done);
        }

        Memset(groups, 0xAA, groups_size);
    }

    /* Determine the block where group table starts */
    if (ext2->block_size == 1024)
        blkno = 2;
    else
        blkno = 1;

    /* Read the block */
    if (_Read(
        ext2->dev, 
        BlockOffset(blkno, ext2->block_size),
        groups, 
        groups_size) != groups_size)
    {
        GOTO(done);
    }

    err = EXT2_ERR_NONE;

done:

    if (EXT2_IFERR(err))
    {
        if (groups)
        {
            Free(groups);
            groups = NULL;
        }
    }

    return groups;
}

static EXT2Err _WriteGroup(
    const EXT2* ext2,
    UINT32 grpno)
{
    EXT2_DECLARE_ERR(err);
    UINT32 blkno;

    /* Check the file system argument */
    if (!EXT2Valid(ext2))
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    if (ext2->block_size == 1024)
        blkno = 2;
    else
        blkno = 1;

    /* Read the block */
    if (_Write(
        ext2->dev, 
        BlockOffset(blkno,ext2->block_size) + 
            (grpno * sizeof(EXT2GroupDesc)),
        &ext2->groups[grpno], 
        sizeof(EXT2GroupDesc)) != sizeof(EXT2GroupDesc))
    {
        err = EXT2_ERR_WRITE_FAILED;
        GOTO(done);
    }

    err = EXT2_ERR_NONE;

done:

    return err;
}

/*
**==============================================================================
**
** inodes:
**
**==============================================================================
*/

static UINT32 _MakeIno(
    const EXT2* ext2,
    UINT32 grpno,
    UINT32 lino)
{
    return (grpno * ext2->sb.s_inodes_per_group) + (lino + 1);
}

static UINT32 _InoToGrpno(
    const EXT2* ext2,
    EXT2Ino ino)
{
    if (ino == 0)
        return 0;

    return (ino-1) / ext2->sb.s_inodes_per_group;
}

static UINT32 _InoToLino(
    const EXT2* ext2,
    EXT2Ino ino)
{
    if (ino == 0)
        return 0;

    return (ino-1) % ext2->sb.s_inodes_per_group;
}

# if !defined(BUILD_EFI)
void EXT2DumpInode(
    const EXT2* ext2,
    const EXT2Inode* inode)
{
    UINT32 i;
    UINT32 n;
    (void)_HexDump;
    (void)_ASCII_Dump;

    printf("=== EXT2Inode\n");
    printf("i_mode=%u (%X)\n", inode->i_mode, inode->i_mode);
    printf("i_uid=%u\n", inode->i_uid);
    printf("i_size=%u\n", inode->i_size);
    printf("i_atime=%u\n", inode->i_atime);
    printf("i_ctime=%u\n", inode->i_ctime);
    printf("i_mtime=%u\n", inode->i_mtime);
    printf("i_dtime=%u\n", inode->i_dtime);
    printf("i_gid=%u\n", inode->i_gid);
    printf("i_links_count=%u\n", inode->i_links_count);
    printf("i_blocks=%u\n", inode->i_blocks);
    printf("i_flags=%u\n", inode->i_flags);
    printf("i_osd1=%u\n", inode->i_osd1);

    {
        printf("i_block[]={");
        n = sizeof(inode->i_block) / sizeof(inode->i_block[0]);

        for (i = 0; i < n; i++)
        {
            printf("%X", inode->i_block[i]);

            if (i + 1 != n)
                printf(", ");
        }

        printf("}\n");
    }

    printf("i_generation=%u\n", inode->i_generation);
    printf("i_file_acl=%u\n", inode->i_file_acl);
    printf("i_dir_acl=%u\n", inode->i_dir_acl);
    printf("i_faddr=%u\n", inode->i_faddr);

    {
        printf("i_osd2[]={");
        n = sizeof(inode->i_osd2) / sizeof(inode->i_osd2[0]);

        for (i = 0; i < n; i++)
        {
            printf("%u", inode->i_osd2[i]);

            if (i + 1 != n)
                printf(", ");
        }

        printf("}\n");
    }

    printf("\n");

    if (inode->i_block[0])
    {
        EXT2Block block;

        if (!EXT2_IFERR(EXT2ReadBlock(ext2, inode->i_block[0], &block)))
        {
            _ASCII_Dump(block.data, block.size);
        }
    }
}
# endif /* !defined(BUILD_EFI) */

static EXT2Err _WriteBlockBitmap(
    const EXT2* ext2,
    UINT32 group_index,
    const EXT2Block* block)
{
    EXT2_DECLARE_ERR(err);
    UINT32 bitmap_size_bytes;

    if (!EXT2Valid(ext2) || !block)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    bitmap_size_bytes = ext2->sb.s_inodes_per_group / 8;

    if (block->size != bitmap_size_bytes)
    {
        err = EXT2_ERR_BAD_SIZE;
        GOTO(done);
    }

    if (group_index > ext2->group_count)
    {
        err = EXT2_ERR_BAD_GRPNO;
        GOTO(done);
    }

    if (EXT2_IFERR(err = EXT2WriteBlock(
        ext2, 
        ext2->groups[group_index].bg_inode_bitmap, 
        block)))
    {
        GOTO(done);
    }

    err = EXT2_ERR_NONE;

done:
    return err;
}

static EXT2Err _GetIno(
    EXT2* ext2,
    EXT2Ino* ino)
{
    EXT2_DECLARE_ERR(err);
    EXT2Block bitmap;
    UINT32 grpno;

    /* Check parameters */
    if (!ext2 || !ino)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    /* Clear the node number */
    *ino = 0;

    /* Use brute force search for a free inode number */
    for (grpno = 0; grpno < ext2->group_count; grpno++)
    {
        UINT32 lino;

        /* Read the bitmap */
        if (EXT2_IFERR(err = EXT2readReadInodeBitmap(ext2, grpno, &bitmap)))
        {
            GOTO(done);
        }

        /* Scan the bitmap, looking for free bit */
        for (lino = 0; lino < bitmap.size * 8; lino++)
        {
            if (!_TestBit(bitmap.data, bitmap.size, lino))
            {
                _SetBit(bitmap.data, bitmap.size, lino);
                *ino = _MakeIno(ext2, grpno, lino);
                break;
            }
        }

        if (*ino)
            break;
    }

    /* If no free inode numbers */
    if (!*ino)
    {
        GOTO(done);
    }

    /* Write the superblock */
    {
        ext2->sb.s_free_inodes_count--;

        if (EXT2_IFERR(err = _WriteSuperBlock(ext2)))
        {
            GOTO(done);
        }
    }

    /* Write the group */
    {
        ext2->groups[grpno].bg_free_inodes_count--;

        if (EXT2_IFERR(err = _WriteGroup(ext2, grpno)))
        {
            GOTO(done);
        }
    }

    /* Write the bitmap */
    if (EXT2_IFERR(err = _WriteBlockBitmap(ext2, grpno, &bitmap)))
    {
        GOTO(done);
    }

    err = EXT2_ERR_NONE;

done:
    return err;
}

EXT2Err EXT2ReadInode(
    const EXT2* ext2,
    EXT2Ino ino,
    EXT2Inode* inode)
{
    EXT2_DECLARE_ERR(err);
    UINT32 lino = _InoToLino(ext2, ino);
    UINT32 grpno = _InoToGrpno(ext2, ino);
    const EXT2GroupDesc* group = &ext2->groups[grpno];
    UINT32 inode_size = ext2->sb.s_inode_size;
    UINTN offset;

    if (ino == 0)
    {
        err = EXT2_ERR_BAD_INO;
        GOTO(done);
    }

#if !defined(BUILD_EFI)
    /* Check the reverse mapping */
    {
        EXT2Ino tmp;
        tmp = _MakeIno(ext2, grpno, lino);
        assert(tmp == ino);
    }
#endif /* !defined(BUILD_EFI) */

    offset = BlockOffset(group->bg_inode_table, ext2->block_size) +
        lino * inode_size;

    /* Read the inode */
    if (_Read(
        ext2->dev, 
        offset,
        inode, 
        inode_size) != inode_size)
    {
        GOTO(done);
    }

    err = EXT2_ERR_NONE;

done:
    return err;
}

static EXT2Err _WriteInode(
    const EXT2* ext2,
    EXT2Ino ino,
    const EXT2Inode* inode)
{
    EXT2_DECLARE_ERR(err);
    UINT32 lino = _InoToLino(ext2, ino);
    UINT32 grpno = _InoToGrpno(ext2, ino);
    const EXT2GroupDesc* group = &ext2->groups[grpno];
    UINT32 inode_size = ext2->sb.s_inode_size;
    UINTN offset;

#if !defined(BUILD_EFI)
    /* Check the reverse mapping */
    {
        EXT2Ino tmp;
        tmp = _MakeIno(ext2, grpno, lino);
        assert(tmp == ino);
    }
#endif /* !defined(BUILD_EFI) */

    offset = BlockOffset(group->bg_inode_table, ext2->block_size) + 
        lino * inode_size;

    /* Read the inode */
    if (_Write(
        ext2->dev, 
        offset,
        inode, 
        inode_size) != inode_size)
    {
        GOTO(done);
    }

    err = EXT2_ERR_NONE;

done:
    return err;
}

EXT2Err EXT2PathToIno(
    const EXT2* ext2,
    const char* path,
    EXT2Ino* ino)
{
    EXT2_DECLARE_ERR(err);
    char buf[EXT2_PATH_MAX];
    const char* elements[32];
    const UINT8 NELEMENTS = sizeof(elements) / sizeof(elements[0]);
    UINT8 nelements = 0;
    char* p;
    char* save;
    UINT8 i;
    EXT2Ino current_ino = 0;

    /* Check for null parameters */
    if (!ext2 || !path || !ino)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    /* Initialize to null inode for now */
    *ino = 0;

    /* Check path length */
    if (Strlen(path) >= EXT2_PATH_MAX)
    {
        err = EXT2_ERR_PATH_TOO_LONG;
        GOTO(done);
    }

    /* Copy path */
    Strlcpy(buf, path, sizeof(buf));

    /* Be sure path begins with "/" */
    if (path[0] != '/')
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    elements[nelements++] = "/";

    /* Split the path into components */
    for (p = Strtok(buf, "/", &save); p; p = Strtok(NULL, "/", &save))
    {
        if (nelements == NELEMENTS)
        {
            err = EXT2_ERR_BUFFER_OVERFLOW;
            GOTO(done);
        }

        elements[nelements++] = p;
    }

    /* Load each inode along the path until we find it */
    for (i = 0; i < nelements; i++)
    {
        if (Strcmp(elements[i], "/") == 0)
        {
            current_ino = EXT2_ROOT_INO;
        }
        else
        {
            EXT2DirEnt* entries = NULL;
            UINT32 nentries = 0;
            UINT32 j;

            if (EXT2_IFERR(err = EXT2ListDirInode(
                ext2, 
                current_ino, 
                &entries, 
                &nentries)))
            {
                GOTO(done);
            }

            current_ino = 0;

            for (j = 0; j < nentries; j++)
            {
                const EXT2DirEnt* ent = &entries[j];

                if (i + 1 == nelements || ent->d_type == EXT2_DT_DIR)
                {
                    if (Strcmp(ent->d_name, elements[i]) == 0)
                    {
                        current_ino = ent->d_ino;
                        break;
                    }
                }
            }

            if (entries)
                Free(entries);

            if (!current_ino)
            {
                /* Not found case */
                err = EXT2_ERR_FILE_NOT_FOUND;
                goto done;
            }
        }
    }

    *ino = current_ino;

    err = EXT2_ERR_NONE;

done:
    return err;
}

EXT2Err EXT2PathToInode(
    const EXT2* ext2,
    const char* path,
    EXT2Ino* ino,
    EXT2Inode* inode)
{
    EXT2_DECLARE_ERR(err);
    EXT2Ino tmp_ino;

    /* Check parameters */
    if (!ext2 || !path || !inode)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    /* Find the ino for this path */
    if (EXT2_IFERR(err = EXT2PathToIno(ext2, path, &tmp_ino)))
    {
        /* Not found case */
        goto done;
    }

    /* Read the inode into memory */
    if (EXT2_IFERR(err = EXT2ReadInode(ext2, tmp_ino, inode)))
    {
        GOTO(done);
    }

    if (ino)
        *ino = tmp_ino;

    err = EXT2_ERR_NONE;

done:
    return err;
}

/*
**==============================================================================
**
** bitmaps:
**
**==============================================================================
*/

EXT2Err EXT2ReadBlockBitmap(
    const EXT2* ext2,
    UINT32 group_index,
    EXT2Block* block)
{
    EXT2_DECLARE_ERR(err);
    UINT32 bitmap_size_bytes;

    if (!EXT2Valid(ext2) || !block)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    Memset(block, 0, sizeof(EXT2Block));

    bitmap_size_bytes = ext2->sb.s_blocks_per_group / 8;

    if (group_index > ext2->group_count)
    {
        err = EXT2_ERR_BAD_GRPNO;
        GOTO(done);
    }

    if (EXT2_IFERR(err = EXT2ReadBlock(
        ext2, 
        ext2->groups[group_index].bg_block_bitmap, 
        block)))
    {
        GOTO(done);
    }

    if (block->size > bitmap_size_bytes)
        block->size = bitmap_size_bytes;

    err = EXT2_ERR_NONE;

done:
    return err;
}

EXT2Err EXT2WriteBlockBitmap(
    const EXT2* ext2,
    UINT32 group_index,
    const EXT2Block* block)
{
    EXT2_DECLARE_ERR(err);
    UINT32 bitmap_size_bytes;

    if (!EXT2Valid(ext2) || !block)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    bitmap_size_bytes = ext2->sb.s_blocks_per_group / 8;

    if (block->size != bitmap_size_bytes)
    {
        err = EXT2_ERR_BAD_SIZE;
        GOTO(done);
    }

    if (group_index > ext2->group_count)
    {
        err = EXT2_ERR_BAD_GRPNO;
        GOTO(done);
    }

    if (EXT2_IFERR(err = EXT2WriteBlock(
        ext2, 
        ext2->groups[group_index].bg_block_bitmap, 
        block)))
    {
        GOTO(done);
    }

    err = EXT2_ERR_NONE;

done:
    return err;
}

EXT2Err EXT2readReadInodeBitmap(
    const EXT2* ext2,
    UINT32 group_index,
    EXT2Block* block)
{
    EXT2_DECLARE_ERR(err);
    UINT32 bitmap_size_bytes;

    if (!EXT2Valid(ext2) || !block)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    Memset(block, 0, sizeof(EXT2Block));

    bitmap_size_bytes = ext2->sb.s_inodes_per_group / 8;

    if (group_index > ext2->group_count)
    {
        err = EXT2_ERR_BAD_GRPNO;
        GOTO(done);
    }

    if (EXT2_IFERR(err = EXT2ReadBlock(
        ext2, 
        ext2->groups[group_index].bg_inode_bitmap, 
        block)))
    {
        GOTO(done);
    }

    if (block->size > bitmap_size_bytes)
        block->size = bitmap_size_bytes;

    err = EXT2_ERR_NONE;

done:
    return err;
}

/*
**==============================================================================
**
** directory:
**
**==============================================================================
*/

# if !defined(BUILD_EFI)
static void _DumpDirectoryEntry(
    const EXT2DirEntry* dirent)
{
    printf("=== EXT2DirEntry:\n");
    printf("inode=%u\n", dirent->inode);
    printf("rec_len=%u\n", dirent->rec_len);
    printf("name_len=%u\n", dirent->name_len);
    printf("file_type=%u\n", dirent->file_type);
    printf("name={%.*s}\n", dirent->name_len, dirent->name);
}
# endif /* !defined(BUILD_EFI) */

static EXT2Err _CountDirectoryEntries(
    const EXT2* ext2,
    const void* data,
    UINT32 size,
    UINT32* count)
{
    EXT2_DECLARE_ERR(err);
    const UINT8* p = (UINT8*)data;
    const UINT8* end = (UINT8*)data + size;

    /* Check parameters */
    if (!EXT2Valid(ext2) || !data || !size || !count)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    /* Initilize the count */
    *count = 0;

    /* Must be divisiable by block size */
    if ((end - p) % ext2->block_size)
    {
        err = EXT2_ERR_BAD_MULTIPLE;
        GOTO(done);
    }

    while (p < end)
    {
        const EXT2DirEntry* ent = (const EXT2DirEntry*)p;

        if (ent->name_len)
        {
            (*count)++;
        }
        
        p += ent->rec_len;
    }

    if (p != end)
    {
        err = EXT2_ERR_EXTRANEOUS_DATA;
        GOTO(done);
    }

    err = EXT2_ERR_NONE;

done:
    return err;
}

struct _EXT2_DIR
{
    void *data;
    UINT32 size;
    const void *next;
    EXT2DirEnt ent;
};

EXT2_DIR *EXT2OpenDir(
    const EXT2* ext2,
    const char *path)
{
    EXT2_DIR *dir = NULL;
    EXT2Ino ino;

    /* Check parameters */
    if (!ext2 || !path)
    {
        GOTO(done);
    }

    /* Find inode number for this directory */
    if (EXT2_IFERR(EXT2PathToIno(ext2, path, &ino)))
        GOTO(done);

    /* Open directory from this inode */
    if (!(dir = EXT2OpenDirIno(ext2, ino)))
        GOTO(done);

done:
    return dir;
}

EXT2_DIR *EXT2OpenDirIno(
    const EXT2* ext2,
    EXT2Ino ino)
{
    EXT2_DIR *dir = NULL;
    EXT2Inode inode;

    /* Check parameters */
    if (!ext2 || !ino)
    {
        GOTO(done);
    }

    /* Read the inode into memory */
    if (EXT2_IFERR(EXT2ReadInode(ext2, ino, &inode)))
    {
        GOTO(done);
    }

    /* Fail if not a directory */
    if (!(inode.i_mode & EXT2_S_IFDIR))
    {
        GOTO(done);
    }

    /* Allocate directory object */
    if (!(dir = (EXT2_DIR*)Calloc(1, sizeof(EXT2_DIR))))
    {
        GOTO(done);
    }

    /* Load the blocks for this inode into memory */
    if (EXT2_IFERR(EXT2LoadFileFromInode(
        ext2, 
        &inode, 
        &dir->data, 
        &dir->size)))
    {
        Free(dir);
        dir = NULL;
        GOTO(done);
    }

    /* Set pointer to current directory */
    dir->next = dir->data;

done:
    return dir;
}

EXT2DirEnt *EXT2ReadDir(
    EXT2_DIR *dir)
{
    EXT2DirEnt *ent = NULL;

    if (!dir || !dir->data || !dir->next)
        goto done;

    /* Find the next entry (possibly skipping padding entries) */
    {
        const void* end = (void*) ((char*) dir->data + dir->size);

        while (!ent && dir->next < end)
        {
            const EXT2DirEntry* de = 
                (EXT2DirEntry*)dir->next;

            if (de->rec_len == 0)
                break;

            if (de->name_len > 0)
            {
                /* Found! */

                /* Set EXT2DirEnt.d_ino */
                dir->ent.d_ino = de->inode;

                /* Set EXT2DirEnt.d_off (not used) */
                dir->ent.d_off = 0;

                /* Set EXT2DirEnt.d_reclen (not used) */
                dir->ent.d_reclen = sizeof(EXT2DirEnt);

                /* Set EXT2DirEnt.type */
                switch (de->file_type)
                {
                    case EXT2_FT_UNKNOWN:
                        dir->ent.d_type = EXT2_DT_UNKNOWN;
                        break;
                    case EXT2_FT_REG_FILE:
                        dir->ent.d_type = EXT2_DT_REG;
                        break;
                    case EXT2_FT_DIR:
                        dir->ent.d_type = EXT2_DT_DIR;
                        break;
                    case EXT2_FT_CHRDEV:
                        dir->ent.d_type = EXT2_DT_CHR;
                        break;
                    case EXT2_FT_BLKDEV:
                        dir->ent.d_type = EXT2_DT_BLK;
                        break;
                    case EXT2_FT_FIFO:
                        dir->ent.d_type = EXT2_DT_FIFO;
                        break;
                    case EXT2_FT_SOCK:
                        dir->ent.d_type = EXT2_DT_SOCK;
                        break;
                    case EXT2_FT_SYMLINK:
                        dir->ent.d_type = EXT2_DT_LNK;
                        break;
                    default:
                        dir->ent.d_type = EXT2_DT_UNKNOWN;
                        break;
                }

                /* Set EXT2DirEnt.d_name */
                dir->ent.d_name[0] = '\0';

                Strncat(
                    dir->ent.d_name, 
                    sizeof(dir->ent.d_name),
                    de->name, 
                    _Min(EXT2_PATH_MAX-1, de->name_len));

                /* Success! */
                ent = &dir->ent;
            }

            /* Position to the next entry (for next call to readdir) */
                dir->next = (void*)((char*)dir->next + de->rec_len); 
        }
    }

done:
    return ent;
}

EXT2Err EXT2CloseDir(
    EXT2_DIR* dir)
{
    EXT2_DECLARE_ERR(err);

    if (!dir)
    {
        GOTO(done);
    }

    Free(dir->data);
    Free(dir);

    err = EXT2_ERR_NONE;

done:
    return err;
}

EXT2Err EXT2ListDirInode(
    const EXT2* ext2,
    EXT2Ino ino,
    EXT2DirEnt** entries,
    UINT32* nentries)
{
    EXT2_DECLARE_ERR(err);
    EXT2_DIR* dir = NULL;
    EXT2DirEnt* ent;
    Buf buf = BUF_INITIALIZER;

    /* Check parameters */
    if (!ext2 || !ino || !entries || !nentries)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    *entries = NULL;
    *nentries = 0;

    /* Open the directory */
    if (!(dir = EXT2OpenDirIno(ext2, ino)))
    {
        GOTO(done);
    }

    /* Add entries to array */
    while ((ent = EXT2ReadDir(dir)))
    {
        /* Append to buffer */
        if (EXT2_IFERR(err = BufAppend(
            &buf,
            ent,
            sizeof(EXT2DirEnt))))
        {
            GOTO(done);
        }
    }

    (*entries) = (EXT2DirEnt*)buf.data;
    (*nentries) = buf.size / sizeof(EXT2DirEnt);

    err = EXT2_ERR_NONE;

done:

    if (EXT2_IFERR(err))
        BufRelease(&buf);

    /* Close the directory */
    if (dir)
        EXT2CloseDir(dir);

    return err;
}

/*
**==============================================================================
**
** files:
**
**==============================================================================
*/

static EXT2Err _SplitFullPath(
    const char* path,
    char dirname[EXT2_PATH_MAX],
    char basename[EXT2_PATH_MAX])
{
    char* slash;

    /* Reject paths that are not absolute */
    if (path[0] != '/')
        return EXT2_ERR_FAILED;

    /* Handle root directory up front */
    if (Strcmp(path, "/") == 0)
    {
        Strlcpy(dirname, "/", EXT2_PATH_MAX);
        Strlcpy(basename, "/", EXT2_PATH_MAX);
        return EXT2_ERR_NONE;
    }

    /* This cannot fail (prechecked) */
    if (!(slash = Strrchr(path, '/')))
        return EXT2_ERR_FAILED;

    /* If path ends with '/' character */
    if (!slash[1])
        return EXT2_ERR_FAILED;

    /* Split the path */
    {
        if (slash == path)
        {
            Strlcpy(dirname, "/", EXT2_PATH_MAX);
        }
        else
        {
            UINTN index = slash - path;
            Strlcpy(dirname, path, EXT2_PATH_MAX);

            if (index < EXT2_PATH_MAX)
                dirname[index] = '\0';
            else
                dirname[EXT2_PATH_MAX-1] = '\0';
        }

        Strlcpy(basename, slash + 1, EXT2_PATH_MAX);
    }

    return EXT2_ERR_NONE;
}

EXT2Err EXT2LoadFileFromInode(
    const EXT2* ext2,
    const EXT2Inode* inode,
    void** data,
    UINT32* size)
{
    EXT2_DECLARE_ERR(err);
    BufU32 blknos = BUF_U32_INITIALIZER;
    Buf buf = BUF_INITIALIZER;
    UINT32 i;

    /* Check parameters */
    if (!EXT2Valid(ext2) || !inode || !data || !size)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    /* Initialize the output */
    *data = NULL;
    *size = 0;

    /* Form a list of block-numbers for this file */
    if (EXT2_IFERR(err = _LoadBlockNumbersFromInode(
        ext2, 
        inode,
        0, /* include_block_blocks */
        &blknos)))
    {
        GOTO(done);
    }

    /* Read and append each block */
    for (i = 0; i < blknos.size; )
    {
        UINT32 nblks = 1;
        UINT32 j;
        EXT2Block block;

        /* Count the number of consecutive blocks: nblks */
        for (j = i + 1; j < blknos.size; j++)
        {
            if (blknos.data[j] != blknos.data[j-1] + 1)
                break;

            nblks++;
        }

        if (nblks == 1)
        {
            /* Read the next block */
            if (EXT2_IFERR(err = EXT2ReadBlock(ext2, blknos.data[i], &block)))
            {
                GOTO(done);
            }

            /* Append block to end of buffer */
            if (EXT2_IFERR(err = BufAppend(&buf, block.data, block.size)))
                GOTO(done);
        }
        else
        {
            if (EXT2_IFERR(_ReadBlocks(ext2, blknos.data[i], nblks, &buf)))
            {
                GOTO(done);
            }
        }

        i += nblks;
    }

    *data = buf.data;
    *size = inode->i_size; /* data may be smaller than block multiple */

    err = EXT2_ERR_NONE;

done:

    if (EXT2_IFERR(err))
        BufRelease(&buf);

    BufU32Release(&blknos);

    return err;
}

EXT2Err EXT2LoadFileFromPath(
    const EXT2* ext2,
    const char* path,
    void** data,
    UINT32* size)
{
    EXT2_DECLARE_ERR(err);
    EXT2Inode inode;

    if (EXT2PathToInode(ext2, path, NULL, &inode) != EXT2_ERR_NONE)
        goto done;

    if (EXT2LoadFileFromInode(ext2, &inode, data, size) != EXT2_ERR_NONE)
        goto done;

    err = EXT2_ERR_NONE;

done:
    return err;
}

/*
**==============================================================================
**
** EXT2
**
**==============================================================================
*/

EXT2Err EXT2New(
    Blkdev* dev,
    EXT2** ext2_out)
{
    EXT2_DECLARE_ERR(err);
    EXT2* ext2 = NULL;

    /* Check parameters */
    if (!dev || !ext2_out)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    /* Initialize output parameters */
    *ext2_out = NULL;

    /* Bit endian is not supported */
    if (_IsBigEndian())
    {
        err = EXT2_ERR_UNSUPPORTED;
        GOTO(done);
    }

    /* Allocate the file system object */
    if (!(ext2 = (EXT2*)Calloc(1, sizeof(EXT2))))
    {
        err = EXT2_ERR_OUT_OF_MEMORY;
        GOTO(done);
    }

    /* Set the file object */
    ext2->dev = dev;

    /* Read the superblock */
    if (EXT2_IFERR(err = _ReadSuperBlock(ext2->dev, &ext2->sb)))
    {
        err = EXT2_ERR_FAILED_TO_READ_SUPERBLOCK;
        GOTO(done);
    }

    /* Check the superblock magic number */
    if (ext2->sb.s_magic != EXT2_S_MAGIC)
    {
        err = EXT2_ERR_BAD_MAGIC;
        GOTO(done);
    }

    /* Reject revision 0 file systems */
    if (ext2->sb.s_rev_level == EXT2_GOOD_OLD_REV)
    {
        /* Revision 0 not supported */
        err = EXT2_ERR_UNSUPPORTED_REVISION;
        GOTO(done);
    }

    /* Accept revision 1 file systems */
    if (ext2->sb.s_rev_level < EXT2_DYNAMIC_REV)
    {
        /* Revision 1 and up supported */
        err = EXT2_ERR_UNSUPPORTED_REVISION;
        GOTO(done);
    }

    /* Check inode size */
    if (ext2->sb.s_inode_size > sizeof(EXT2Inode))
    {
        err = EXT2_ERR_UNEXPECTED;
        GOTO(done);
    }

    /* Calcualte the block size in bytes */
    ext2->block_size = 1024 << ext2->sb.s_log_block_size;

    /* Calculate the number of block groups */
    ext2->group_count = 
        1 + (ext2->sb.s_blocks_count-1) / ext2->sb.s_blocks_per_group;

    /* Get the groups list */
    if (!(ext2->groups = _ReadGroups(ext2)))
    {
        err = EXT2_ERR_FAILED_TO_READ_GROUPS;
        GOTO(done);
    }

    /* Read the root inode */
    if (EXT2_IFERR(err = EXT2ReadInode(ext2, EXT2_ROOT_INO, &ext2->root_inode)))
    {
        err = EXT2_ERR_FAILED_TO_READ_INODE;
        GOTO(done);
    }

    err = EXT2_ERR_NONE;

done:

    if (EXT2_IFERR(err))
    {
        /* Caller must dispose of this! */
        ext2->dev = NULL;
        EXT2Delete(ext2);
    }
    else
        *ext2_out = ext2;

    return err;
}

void EXT2Delete(EXT2* ext2)
{
    if (ext2)
    {
        if (ext2->dev)
            ext2->dev->Close(ext2->dev);

        if (ext2->groups)
            Free(ext2->groups);

        Free(ext2);
    }
}

# if !defined(BUILD_EFI)
EXT2Err EXT2Dump(
    const EXT2* ext2)
{
    EXT2_DECLARE_ERR(err);
    UINT32 grpno;

    /* Print the superblock */
    EXT2DumpSuperBlock(&ext2->sb);

    printf("block_size=%u\n", ext2->block_size);
    printf("group_count=%u\n", ext2->group_count);

    /* Print the groups */
    _DumpGroupDescs(ext2->groups, ext2->group_count);

    /* Print out the bitmaps for the data blocks */
    {
        for (grpno = 0; grpno < ext2->group_count; grpno++)
        {
            EXT2Block bitmap;

            if (EXT2_IFERR(err = EXT2ReadBlockBitmap(ext2, grpno, &bitmap)))
            {
                GOTO(done);
            }

            printf("=== block bitmap:\n");
            _dump_bitmap(&bitmap);
        }
    }

    /* Print the inode bitmaps */
    for (grpno = 0; grpno < ext2->group_count; grpno++)
    {
        EXT2Block bitmap;

        if (EXT2_IFERR(err = EXT2readReadInodeBitmap(ext2, grpno, &bitmap)))
        {
            GOTO(done);
        }

        printf("=== inode bitmap:\n");
        _dump_bitmap(&bitmap);
    }

    /* dump the inodes */
    {
        UINT32 nbits = 0;
        UINT32 mbits = 0;

        /* Print the inode tables */
        for (grpno = 0; grpno < ext2->group_count; grpno++)
        {
            EXT2Block bitmap;
            UINT32 lino;

            /* Get inode bitmap for this group */
            if (EXT2_IFERR(err = EXT2readReadInodeBitmap(ext2, grpno, &bitmap)))
            {
                GOTO(done);
            }

            nbits += _CountBitsN(bitmap.data, bitmap.size);

            /* For each bit set in the bit map */
            for (lino = 0; lino < ext2->sb.s_inodes_per_group; lino++)
            {
                EXT2Inode inode;
                EXT2Ino ino;

                if (!_TestBit(bitmap.data, bitmap.size, lino))
                    continue;

                mbits++;

                if ((lino+1) < EXT2_FIRST_INO && (lino+1) != EXT2_ROOT_INO)
                    continue;

                ino = _MakeIno(ext2, grpno, lino);

                if (EXT2_IFERR(err = EXT2ReadInode(ext2, ino, &inode)))
                {
                    GOTO(done);
                }

                printf("INODE{%u}\n", ino);
                EXT2DumpInode(ext2, &inode);
            }
        }

        printf("nbits{%u}\n", nbits);
        printf("mbits{%u}\n", mbits);
    }

    /* dump the root inode */
    EXT2DumpInode(ext2, &ext2->root_inode);

    err = EXT2_ERR_NONE;

done:
    return err;
}
#endif /* !defined(BUILD_EFI) */

EXT2Err EXT2Check(
    const EXT2* ext2)
{
    EXT2_DECLARE_ERR(err);

    /* Check the block bitmaps */
    {
        UINT32 i;
        UINT32 n = 0;
        UINT32 nused = 0;
        UINT32 nfree = 0;

        for (i = 0; i < ext2->group_count; i++)
        {
            EXT2Block bitmap;

            nfree += ext2->groups[i].bg_free_blocks_count;

            if (EXT2_IFERR(err = EXT2ReadBlockBitmap(ext2, i, &bitmap)))
            {
                GOTO(done);
            }

            nused += _CountBitsN(bitmap.data, bitmap.size);
            n += bitmap.size * 8;
        }

        if (ext2->sb.s_free_blocks_count != nfree)
        {
#if !defined(BUILD_EFI)
            printf("s_free_blocks_count{%u}, nfree{%u}\n",
                ext2->sb.s_free_blocks_count, nfree);
#endif /* !defined(BUILD_EFI) */
            GOTO(done);
        }

        if (ext2->sb.s_free_blocks_count != n - nused)
        {
            GOTO(done);
        }
    }

    /* Check the inode bitmaps */
    {
        UINT32 i;
        UINT32 n = 0;
        UINT32 nused = 0;
        UINT32 nfree = 0;

        /* Check the bitmaps for the inodes */
        for (i = 0; i < ext2->group_count; i++)
        {
            EXT2Block bitmap;

            nfree += ext2->groups[i].bg_free_inodes_count;

            if (EXT2_IFERR(err = EXT2readReadInodeBitmap(ext2, i, &bitmap)))
            {
                GOTO(done);
            }

            nused += _CountBitsN(bitmap.data, bitmap.size);
            n += bitmap.size * 8;
        }

        if (ext2->sb.s_free_inodes_count != n - nused)
        {
            GOTO(done);
        }

        if (ext2->sb.s_free_inodes_count != nfree)
        {
            GOTO(done);
        }
    }

    /* Check the inodes */
    {
        UINT32 grpno;
        UINT32 nbits = 0;
        UINT32 mbits = 0;

        /* Check the inode tables */
        for (grpno = 0; grpno < ext2->group_count; grpno++)
        {
            EXT2Block bitmap;
            UINT32 lino;

            /* Get inode bitmap for this group */
            if (EXT2_IFERR(err = EXT2readReadInodeBitmap(ext2, grpno, &bitmap)))
            {
                GOTO(done);
            }

            nbits += _CountBitsN(bitmap.data, bitmap.size);

            /* For each bit set in the bit map */
            for (lino = 0; lino < ext2->sb.s_inodes_per_group; lino++)
            {
                EXT2Inode inode;
                EXT2Ino ino;

                if (!_TestBit(bitmap.data, bitmap.size, lino))
                    continue;

                mbits++;

                if ((lino+1) < EXT2_FIRST_INO && (lino+1) != EXT2_ROOT_INO)
                    continue;

                ino = _MakeIno(ext2, grpno, lino);

                if (EXT2_IFERR(err = EXT2ReadInode(ext2, ino, &inode)))
                {
                    GOTO(done);
                }

                /* Mode can never be zero */
                if (inode.i_mode == 0)
                {
                    GOTO(done);
                }

                /* If file is not zero size but no blocks, then fail */
                if (inode.i_size && !inode.i_block[0])
                {
                    GOTO(done);
                }
            }
        }

        /* The number of bits in bitmap must match number of active inodes */
        if (nbits != mbits)
        {
            GOTO(done);
        }
    }

    err = EXT2_ERR_NONE;

done:
    return err;
}

EXT2Err EXT2Trunc(
    EXT2* ext2,
    const char* path)
{
    EXT2_DECLARE_ERR(err);
    char dirname[EXT2_PATH_MAX];
    char basename[EXT2_PATH_MAX];
    EXT2Ino dir_ino;
    EXT2Inode dir_inode;
    EXT2Ino file_ino;
    EXT2Inode file_inode;
    BufU32 blknos = BUF_U32_INITIALIZER;
#if !defined(BUILD_EFI)
    (void)_DumpBlockNumbers;
#endif /* !defined(BUILD_EFI) */

    /* Check parameters */
    if (!ext2 || !path)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    /* Split the path */
    if (EXT2_IFERR(err = _SplitFullPath(path, dirname, basename)))
    {
        GOTO(done);
    }

    /* Find the inode of the dirname */
    if (EXT2_IFERR(err = EXT2PathToInode(ext2, dirname, &dir_ino, &dir_inode)))
    {
        GOTO(done);
    }

    /* Find the inode of the basename */
    if (EXT2_IFERR(err = EXT2PathToInode(
        ext2, path, &file_ino, &file_inode)))
    {
        goto done;
    }

    /* Form a list of block-numbers for this file */
    if (EXT2_IFERR(err = _LoadBlockNumbersFromInode(
        ext2, 
        &file_inode, 
        1, /* include_block_blocks */
        &blknos)))
    {
        GOTO(done);
    }

    /* If this file is a directory, then fail */
    if (file_inode.i_mode & EXT2_S_IFDIR)
    {
        GOTO(done);
    }

    /* Return the blocks to the free list */
    {
        if (EXT2_IFERR(err = _PutBlocks(ext2, blknos.data, blknos.size)))
            GOTO(done);
    }

    /* Rewrite the inode */
    {
        file_inode.i_size = 0;
        Memset(file_inode.i_block, 0, sizeof(file_inode.i_block));

        if (EXT2_IFERR(err = _WriteInode(ext2, file_ino, &file_inode)))
        {
            GOTO(done);
        }
    }

    /* Update the super block */
    if (EXT2_IFERR(err = _WriteSuperBlock(ext2)))
    {
        GOTO(done);
    }

    err = EXT2_ERR_NONE;

done:

    BufU32Release(&blknos);

    return err;
}

static EXT2Err _WriteSingleDirectBlockNumbers(
    EXT2* ext2,
    const UINT32* blknos,
    UINT32 nblknos,
    UINT32* blkno)
{
    EXT2_DECLARE_ERR(err);
    EXT2Block block;

    /* Check parameters */
    if (!EXT2Valid(ext2) || !blknos || !nblknos)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    /* If no room in a single block for the block numbers */
    if (nblknos > ext2->block_size / sizeof(UINT32))
    {
        err = EXT2_ERR_BUFFER_OVERFLOW;
        GOTO(done);
    }

    /* Assign an available block */
    if (EXT2_IFERR(err = _GetBlock(ext2, blkno)))
    {
        GOTO(done);
    }

    /* Copy block numbers into block */
    Memset(&block, 0, sizeof(EXT2Block));
    Memcpy(block.data, blknos, nblknos * sizeof(UINT32));
    block.size = ext2->block_size;

    /* Write the block */
    if (EXT2_IFERR(err = EXT2WriteBlock(ext2, *blkno, &block)))
    {
        GOTO(done);
    }

    err = EXT2_ERR_NONE;

done:
    return err;
}

static EXT2Err _WriteIndirectBlockNumbers(
    EXT2* ext2,
    UINT32 indirection, /* level of indirection: 2=double, 3=triple */
    const UINT32* blknos,
    UINT32 nblknos,
    UINT32* blkno)
{
    EXT2_DECLARE_ERR(err);
    EXT2Block block;
    UINT32 blknos_per_block = ext2->block_size / sizeof(UINT32);

    /* Check parameters */
    if (!EXT2Valid(ext2) || !blknos || !nblknos || !blkno)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    /* Check indirection level */
    if (indirection != 2 && indirection != 3)
    {
        err = EXT2_ERR_UNEXPECTED;
        GOTO(done);
    }

    /* If double indirection */
    if (indirection == 2)
    {
        /* If too many indirect blocks */
        if (nblknos > blknos_per_block * blknos_per_block)
        {
            err = EXT2_ERR_BUFFER_OVERFLOW;
            GOTO(done);
        }
    }

    /* Assign an available block */
    if (EXT2_IFERR(err = _GetBlock(ext2, blkno)))
    {
        GOTO(done);
    }

    /* Allocate indirect block (to hold block numbers) */
    Memset(&block, 0, sizeof(EXT2Block));
    block.size = ext2->block_size;

    /* Write each of the indirect blocks */
    {
        const UINT32* p = blknos;
        UINT32 r = nblknos;
        UINT32 i;

        /* For each block */
        for (i = 0; r > 0; i++)
        {
            UINT32 n;

            /* If double indirection */
            if (indirection == 2)
            {
                n = _Min(r, blknos_per_block);

                if (EXT2_IFERR(err = _WriteSingleDirectBlockNumbers(
                    ext2,
                    p,
                    n,
                    (UINT32*)block.data + i)))
                {
                    GOTO(done);
                }
            }
            else
            {
                n = _Min(r, blknos_per_block * blknos_per_block);

                /* Write the block numbers for this block */
                if (EXT2_IFERR(err = _WriteIndirectBlockNumbers(
                    ext2,
                    2, /* double indirection */
                    p,
                    n,
                    (UINT32*)block.data + i)))
                {
                    GOTO(done);
                }
            }

            p += n;
            r -= n;
        }

        /* Check that the blocks were exhausted */
        if (r != 0)
        {
            err = EXT2_ERR_EXTRANEOUS_DATA;
            GOTO(done);
        }
    }

    /* Write the indirect block */
    if (EXT2_IFERR(err = EXT2WriteBlock(ext2, *blkno, &block)))
    {
        GOTO(done);
    }

    err = EXT2_ERR_NONE;

done:
    return err;
}

static EXT2Err _WriteData(
    EXT2* ext2,
    const void* data,
    UINT32 size,
    BufU32* blknos)
{
    EXT2_DECLARE_ERR(err);
    UINT32 blksize = ext2->block_size;
    UINT32 nblks = (size + blksize - 1) / blksize;
    UINT32 rem = size % blksize;
    UINT32 i = 0;
    UINT32 grpno;

    for (grpno = 0; grpno < ext2->group_count && i < nblks; grpno++)
    {
        EXT2Block bitmap;
        EXT2Block block;
        UINT32 blkno;
        UINT32 lblkno;
        int changed = 0;
    
        /* Read the bitmap */
        if (EXT2_IFERR(err = EXT2ReadBlockBitmap(ext2, grpno, &bitmap)))
        {
            GOTO(done);
        }

        /* Scan the bitmap, looking for free bit */
        for (lblkno = 0; lblkno < bitmap.size * 8 && i < nblks; lblkno++)
        {
            if (!_TestBit(bitmap.data, bitmap.size, lblkno))
            {
                _SetBit(bitmap.data, bitmap.size, lblkno);
                blkno = MakeBlkno(ext2, grpno, lblkno);

                /* Append this to the array of blocks */
                if (EXT2_IFERR(err = BufU32Append(blknos, &blkno, 1)))
                    GOTO(done);

                /* If this is the final block */
                if (i + 1 == nblks && rem)
                {
                    Memcpy(block.data, (char*)data + (i * blksize), rem);
                    block.size = rem;
                }
                else
                {
                    Memcpy(block.data, (char*)data + (i * blksize), blksize);
                    block.size = blksize;
                }

                /* Write the block */
                if (EXT2_IFERR(err = EXT2WriteBlock(ext2, blkno, &block)))
                {
                    GOTO(done);
                }

                /* Update the in memory structs. */
                ext2->sb.s_free_blocks_count--;
                ext2->groups[grpno].bg_free_blocks_count--;
                i++;
                changed = 1;
            }
        }

        if (changed == 1)
        {
            /* Write group and bitmap. */
            if (EXT2_IFERR(err = _WriteGroup(ext2, grpno)))
            {
                GOTO(done);
            }

            if (EXT2_IFERR(err = EXT2WriteBlockBitmap(ext2, grpno, &bitmap)))
            {
                GOTO(done);
            }
        }
    }

    /* Ran out of disk space. */
    if (i != nblks)
    {
        GOTO(done);
    }

    /* Write super block data. */
    if (EXT2_IFERR(err = _WriteSuperBlock(ext2)))
    {
        GOTO(done);
    }

    err = EXT2_ERR_NONE;

done:
    return err;
}

static EXT2Err _UpdateInodeBlockPointers(
    EXT2* ext2,
    EXT2Ino ino,
    EXT2Inode* inode,
    UINT32 size,
    const UINT32* blknos,
    UINT32 nblknos)
{
    EXT2_DECLARE_ERR(err);
    UINT32 i;
    const UINT32* p = blknos;
    UINT32 r = nblknos;
    UINT32 blknos_per_block = ext2->block_size / sizeof(UINT32);

    /* Update the inode size */
    inode->i_size = size;

    /* Update the direct inode blocks */
    if (r)
    {
        UINT32 n = _Min(r, EXT2_SINGLE_INDIRECT_BLOCK);

        for (i = 0; i < n; i++)
        {
            inode->i_block[i] = p[i];
        }

        p += n;
        r -= n;
    }

    /* Write the indirect block numbers */
    if (r)
    {
        UINT32 n = _Min(r, blknos_per_block);

        if (EXT2_IFERR(err = _WriteSingleDirectBlockNumbers(
            ext2,
            p,
            n,
            &inode->i_block[EXT2_SINGLE_INDIRECT_BLOCK])))
        {
            GOTO(done);
        }

        p += n;
        r -= n;
    }

    /* Write the double indirect block numbers */
    if (r)
    {
        UINT32 n = _Min(r, blknos_per_block * blknos_per_block);

        if (EXT2_IFERR(err = _WriteIndirectBlockNumbers(
            ext2,
            2, /* double indirection */
            p,
            n,
            &inode->i_block[EXT2_DOUBLE_INDIRECT_BLOCK])))
        {
            GOTO(done);
        }

        p += n;
        r -= n;
    }

    /* Write the triple indirect block numbers */
    if (r)
    {
        UINT32 n = r;

        if (EXT2_IFERR(err = _WriteIndirectBlockNumbers(
            ext2,
            3, /* triple indirection */
            p,
            n,
            &inode->i_block[EXT2_TRIPLE_INDIRECT_BLOCK])))
        {
            GOTO(done);
        }

        p += n;
        r -= n;
    }

    /* Note: triple indirect block numbers not handled! */
    if (r > 0)
    {
        GOTO(done);
    }

    /* Rewrite the inode */
    if (EXT2_IFERR(err = _WriteInode(ext2, ino, inode)))
    {
        GOTO(done);
    }

    err = EXT2_ERR_NONE;

done:
    return err;
}

static EXT2Err _UpdateInodeDataBlocks(
    EXT2* ext2,
    EXT2Ino ino,
    EXT2Inode* inode,
    const void* data,
    UINT32 size,
    BOOLEAN is_dir)
{
    EXT2_DECLARE_ERR(err);
    BufU32 blknos = BUF_U32_INITIALIZER;
    UINT32 count = 0;
    void* tmp_data = NULL;

    if (EXT2_IFERR(err = _WriteData(ext2, data, size, &blknos)))
        GOTO(done);

    if (is_dir)
    {
        if (EXT2_IFERR(err = _CountDirectoryEntries(
            ext2, data, size, &count)))
        {
            GOTO(done);
        }
    }

    inode->i_osd1 = count + 1;

    if (EXT2_IFERR(err = _UpdateInodeBlockPointers(
        ext2,
        ino,
        inode,
        size,
        blknos.data,
        blknos.size)))
    {
        GOTO(done);
    }

    err = EXT2_ERR_NONE;

done:

    BufU32Release(&blknos);

    if (tmp_data)
        Free(tmp_data);

    return err;
}

EXT2Err EXT2Update(
    EXT2* ext2,
    const void* data,
    UINT32 size,
    const char* path)
{
    EXT2_DECLARE_ERR(err);
    EXT2Ino ino;
    EXT2Inode inode;
#if !defined(BUILD_EFI)
    (void)_DumpDirectoryEntry;
#endif /* !defined(BUILD_EFI) */

    /* Check parameters */
    if (!EXT2Valid(ext2) || !data || !path)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    /* Truncate the destination file */
    if (EXT2_IFERR(err = EXT2Trunc(ext2, path)))
    {
        GOTO(done);
    }

    /* Read inode for this file */
    if (EXT2_IFERR(err = EXT2PathToInode(ext2, path, &ino, &inode)))
    {
        GOTO(done);
    }

    /* Update the inode blocks */
    if (EXT2_IFERR(err = _UpdateInodeDataBlocks(
        ext2,
        ino,
        &inode,
        data,
        size,
        0))) /* !is_dir */
    {
        GOTO(done);
    }

    err = EXT2_ERR_NONE;

done:

    return err;
}

static EXT2Err _CheckDirectoryEntries(
    const EXT2* ext2,
    const void* data,
    UINT32 size)
{
    EXT2_DECLARE_ERR(err);
    const UINT8* p = (UINT8*)data;
    const UINT8* end = (UINT8*)data + size;

    /* Must be divisiable by block size */
    if ((end - p) % ext2->block_size)
    {
        GOTO(done);
    }

    while (p < end)
    {
        UINT32 n;
        const EXT2DirEntry* ent = (const EXT2DirEntry*)p;
        
        n = sizeof(EXT2DirEntry) - EXT2_PATH_MAX + ent->name_len;
        n = _NextMult(n, 4);

        if (n != ent->rec_len)
        {
            UINT32 offset = ((char*)p - (char*)data) % ext2->block_size;
            UINT32 rem = ext2->block_size - offset;

            if (rem != ent->rec_len)
            {
                GOTO(done);
            }
        }

        p += ent->rec_len;
    }

    if (p != end)
    {
        GOTO(done);
    }

    err = EXT2_ERR_NONE;

done:
    return err;
}

#if !defined(BUILD_EFI)
static void _DumpDirectoryEntries(
    const EXT2* ext2,
    const void* data,
    UINT32 size)
{
    const UINT8* p = (UINT8*)data;
    const UINT8* end = (UINT8*)data + size;

    while (p < end)
    {
        UINT32 n;
        const EXT2DirEntry* ent = (const EXT2DirEntry*)p;

        _DumpDirectoryEntry(ent);
        
        n = sizeof(EXT2DirEntry) - EXT2_PATH_MAX + ent->name_len;
        n = _NextMult(n, 4);

        if (n != ent->rec_len)
        {
            UINT32 gap = ent->rec_len - n;
            UINT32 offset = ((char*)p - (char*)data) % ext2->block_size;
            UINT32 rem = ext2->block_size - offset;

            printf("gap: %u\n", gap);
            printf("offset: %u\n", offset);
            printf("remaing: %u\n", rem);
        }

        p += ent->rec_len;
    }
}
#endif /* !defined(BUILD_EFI) */

static const EXT2DirEntry* _FindDirectoryEntry(
    const char* name,
    const void* data,
    UINT32 size)
{
    const UINT8* p = (UINT8*)data;
    const UINT8* end = (UINT8*)data + size;

    while (p < end)
    {
        const EXT2DirEntry* ent = (const EXT2DirEntry*)p;
        char tmp[EXT2_PATH_MAX];


        /* Create zero-terminated name */
        tmp[0] = '\0';
        Strncat(tmp, sizeof(tmp), ent->name, 
            _Min(ent->name_len, EXT2_PATH_MAX-1));

        if (Strcmp(tmp, name) == 0)
            return ent;

        p += ent->rec_len;
    }

    /* Not found */
    return NULL;
}

static EXT2Err _CountDirectoryEntriesIno(
    EXT2* ext2,
    EXT2Ino ino,
    UINT32* count)
{
    EXT2_DECLARE_ERR(err);
    EXT2_DIR* dir = NULL;
    EXT2DirEnt* ent;

    /* Check parameters */
    if (!EXT2Valid(ext2) || !ino || !count)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    /* Initialize count */
    *count = 0;

    /* Open directory */
    if (!(dir = EXT2OpenDirIno(ext2, ino)))
    {
        GOTO(done);
    }

    /* Count the entries (notes that "." and ".." will always be present). */
    while ((ent = EXT2ReadDir(dir)))
    {
        (*count)++;
    }

    err = EXT2_ERR_NONE;

done:

    if (dir)
        EXT2CloseDir(dir);

    return err;
}

EXT2Err EXT2Rm(
    EXT2* ext2,
    const char* path)
{
    EXT2_DECLARE_ERR(err);
    char dirname[EXT2_PATH_MAX];
    char filename[EXT2_PATH_MAX];
    void* blocks = NULL;
    UINT32 blocks_size = 0;
    void* new_blocks = NULL;
    UINT32 new_blocks_size = 0;
    BufU32 blknos = BUF_U32_INITIALIZER;
    EXT2Ino ino;
    EXT2Inode inode;
    const EXT2DirEntry* ent = NULL;
#if !defined(BUILD_EFI)
    (void)_DumpDirectoryEntries;
#endif /* !defined(BUILD_EFI) */

    /* Check parameters */
    if (!EXT2Valid(ext2) && !path)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    /* Truncate the file first */
    if (EXT2_IFERR(err = EXT2Trunc(ext2, path)))
    {
        goto done;
    }

    /* Split the path */
    if (EXT2_IFERR(err = _SplitFullPath(path, dirname, filename)))
    {
        GOTO(done);
    }

    /* Load the directory inode */
    if (EXT2_IFERR(err = EXT2PathToInode(ext2, dirname, &ino, &inode)))
    {
        GOTO(done);
    }

    /* Load the directory file */
    if (EXT2_IFERR(err = EXT2LoadFileFromInode(
        ext2, 
        &inode, 
        &blocks, 
        &blocks_size)))
    {
        GOTO(done);
    }

    /* Load the block numbers (including the block blocks) */
    if (EXT2_IFERR(err = _LoadBlockNumbersFromInode(
        ext2,
        &inode,
        1, /* include_block_blocks */
        &blknos)))
    {
        GOTO(done);
    }

    /* Find 'filename' within this directory */
    if (!(ent = _FindDirectoryEntry(filename, blocks, blocks_size)))
    {
        GOTO(done);
    }

    /* Allow removal of empty directories only */
    if (ent->file_type == EXT2_FT_DIR)
    {
        EXT2Ino dir_ino;
        EXT2Inode dir_inode;
        UINT32 count;

        /* Find the inode of the filename */
        if (EXT2_IFERR(err = EXT2PathToInode(
            ext2, filename, &dir_ino, &dir_inode)))
        {
            GOTO(done);
        }

        /* Disallow removal if directory is non empty */
        if (EXT2_IFERR(err = _CountDirectoryEntriesIno(ext2, dir_ino, &count)))
        {
            GOTO(done);
        }

        /* Expect just "." and ".." entries */
        if (count != 2)
        {
            GOTO(done);
        }
    }

    /* Convert from 'indexed' to 'linked list' directory format */
    {
        new_blocks_size = blocks_size;
        const char* src = (const char*)blocks;
        const char* src_end = (const char*)blocks + inode.i_size;
        char* dest = NULL;

        /* Allocate a buffer to hold 'linked list' directory */
        if (!(new_blocks = Calloc(new_blocks_size, 1)))
        {
            GOTO(done);
        }

        /* Set current and end pointers to new buffer */
        dest = (char*)new_blocks;

        /* Copy over directory entries (skipping removed entry) */
        {
            EXT2DirEntry* prev = NULL;

            while (src < src_end)
            {
                const EXT2DirEntry* curr_ent = 
                    (const EXT2DirEntry*)src;
                UINT32 rec_len;
                UINT32 offset;

                /* Skip the removed directory entry */
                if (curr_ent == ent || !ent->name)
                {
                    src += curr_ent->rec_len;
                    continue;
                }

                /* Compute size of the new directory entry */
                rec_len = sizeof(*curr_ent) - EXT2_PATH_MAX + curr_ent->name_len;
                rec_len = _NextMult(rec_len, 4);

                /* Compute byte offset into current block */
                offset = (dest - (char*)new_blocks) % ext2->block_size;

                /* If new entry would overflow the block */
                if (offset + rec_len > ext2->block_size)
                {
                    UINT32 rem = ext2->block_size - offset;

                    if (!prev)
                    {
                        GOTO(done);
                    }

                    /* Adjust previous entry to point to next block */
                    prev->rec_len += rem;
                    dest += rem;
                }

                /* Copy this entry into new buffer */
                {
                    EXT2DirEntry* new_ent = 
                        (EXT2DirEntry*)dest;
                    Memset(new_ent, 0, rec_len);
                    Memcpy(new_ent, curr_ent, 
                        sizeof(*curr_ent) + curr_ent->name_len);

                    new_ent->rec_len = rec_len;
                    prev = new_ent;
                    dest += rec_len;
                }

                src += curr_ent->rec_len;
            }

            /* Set final entry to point to end of the block */
            if (prev)
            {
                UINT32 offset;
                UINT32 rem;

                /* Compute byte offset into current block */
                offset = (dest - (char*)new_blocks) % ext2->block_size;

                /* Compute remaining bytes */
                rem = ext2->block_size - offset;

                /* Set record length of final entry to end of block */
                prev->rec_len += rem;

                /* Advance dest to block boundary */
                dest += rem;
            }

            /* Size down the new blocks size */
            new_blocks_size = (UINT32)(dest - (char*)new_blocks);

            if (EXT2_IFERR(err = _CheckDirectoryEntries(
                ext2, 
                new_blocks, 
                new_blocks_size)))
            {
                GOTO(done);
            }
        }
    }

    /* Count directory entries before and after */
    {
        UINT32 count;
        UINT32 new_count;

        if (EXT2_IFERR(err = _CountDirectoryEntries(
            ext2, 
            blocks, 
            blocks_size, 
            &count)))
        {
            GOTO(done);
        }

        if (EXT2_IFERR(err = _CountDirectoryEntries(
            ext2, 
            new_blocks, 
            new_blocks_size, 
            &new_count)))
        {
            GOTO(done);
        }
    }

    /* Return all directory blocks to the free list */
    if (EXT2_IFERR(err = _PutBlocks(ext2, blknos.data, blknos.size)))
    {
        GOTO(done);
    }

    /* Update the inode blocks */
    if (EXT2_IFERR(err = _UpdateInodeDataBlocks(
        ext2,
        ino,
        &inode,
        new_blocks,
        new_blocks_size,
        1))) /* is_dir */
    {
        GOTO(done);
    }

    err = EXT2_ERR_NONE;

done:

    if (blocks)
        Free(blocks);

    if (new_blocks)
        Free(new_blocks);

    BufU32Release(&blknos);

    return err;
}

static EXT2Err _CreateFileInode(
    EXT2* ext2, 
    const void* data, 
    UINT32 size, 
    UINT16 mode,
    UINT32* blknos, 
    UINT32 nblknos,
    UINT32* ino)
{
    EXT2_DECLARE_ERR(err);
    EXT2Inode inode;

    /* Check parameters */
    if (!EXT2Valid(ext2) || !data || !blknos)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    /* Initialize the inode */
    {
        /* Uses posix_time() for EFI */
        const UINT32 t = time(NULL);

        Memset(&inode, 0, sizeof(EXT2Inode));

        /* Set the mode of the new file */
        inode.i_mode = mode;

        /* Set the uid and gid to root */
        inode.i_uid = 0;
        inode.i_gid = 0;

        /* Set the size of this file */
        inode.i_size = size;

        /* Set the access, creation, and mtime to the same value */
        inode.i_atime = t;
        inode.i_ctime = t;
        inode.i_mtime = t;

        /* Linux-specific value */
        inode.i_osd1 = 1;

        /* The number of links is initially 1 */
        inode.i_links_count = 1;

        /* Set the number of 512 byte blocks */
        inode.i_blocks = (nblknos * ext2->block_size) / 512;
    }

    /* Assign an inode number */
    if (EXT2_IFERR(err = _GetIno(ext2, ino)))
    {
        GOTO(done);
    }

    /* Update the inode block pointers and write the inode to disk */
    if (EXT2_IFERR(err = _UpdateInodeBlockPointers(
        ext2,
        *ino,
        &inode,
        size,
        blknos,
        nblknos)))
    {
        GOTO(done);
    }

    err = EXT2_ERR_NONE;

done:
    return err;
}

typedef struct _EXT2DirectoryEntBuf
{
    EXT2DirEntry base;
        char buf[EXT2_PATH_MAX];
}
EXT2DirectoryEntBuf;

static EXT2Err _CreateDirInodeAndBlock(
    EXT2* ext2, 
    EXT2Ino parent_ino,
    UINT16 mode,
    EXT2Ino* ino)
{
    EXT2_DECLARE_ERR(err);
    EXT2Inode inode;
    UINT32 blkno;
    EXT2Block block;

    /* Check parameters */
    if (!EXT2Valid(ext2) || !mode || !parent_ino || !ino)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    /* Initialize the inode */
    {
        const UINT32 t = time(NULL);

        Memset(&inode, 0, sizeof(EXT2Inode));

        /* Set the mode of the new file */
        inode.i_mode = mode;

        /* Set the uid and gid to root */
        inode.i_uid = 0;
        inode.i_gid = 0;

        /* Set the size of this file */
        inode.i_size = ext2->block_size;

        /* Set the access, creation, and mtime to the same value */
        inode.i_atime = t;
        inode.i_ctime = t;
        inode.i_mtime = t;

        /* Linux-specific value */
        inode.i_osd1 = 1;

        /* The number of links is initially 2 */
        inode.i_links_count = 2;

        /* Set the number of 512 byte blocks */
        inode.i_blocks = ext2->block_size / 512;
    }

    /* Assign an inode number */
    if (EXT2_IFERR(err = _GetIno(ext2, ino)))
    {
        GOTO(done);
    }

    /* Assign a block number */
    if (EXT2_IFERR(err = _GetBlock(ext2, &blkno)))
    {
        GOTO(done);
    }

    /* Create a block to hold the two directory entries */
    {
        EXT2DirectoryEntBuf dot1;
        EXT2DirectoryEntBuf dot2;
        EXT2DirEntry* ent;

        /* The "." directory */
        Memset(&dot1, 0, sizeof(dot1));
        dot1.base.inode = *ino;
        dot1.base.name_len = 1;
        dot1.base.file_type = EXT2_DT_DIR;
        dot1.base.name[0] = '.';
        UINT16 d1len = sizeof(EXT2DirEntry) - EXT2_PATH_MAX + dot1.base.name_len;
        dot1.base.rec_len = _NextMult(d1len, 4);

        /* The ".." directory */
        Memset(&dot2, 0, sizeof(dot2));
        dot2.base.inode = parent_ino;
        dot2.base.name_len = 2;
        dot2.base.file_type = EXT2_DT_DIR;
        dot2.base.name[0] = '.';
        dot2.base.name[1] = '.';
        UINT16 d2len = sizeof(EXT2DirEntry) - EXT2_PATH_MAX + dot2.base.name_len;
        dot2.base.rec_len = _NextMult(d2len, 4);

        /* Initialize the directory entries block */
        Memset(&block, 0, sizeof(EXT2Block));
        Memcpy(block.data, &dot1, dot1.base.rec_len);
        Memcpy(block.data + dot1.base.rec_len, &dot2, dot2.base.rec_len);
        block.size = ext2->block_size;

        /* Adjust dot2.rec_len to point to end of block */
        ent = (EXT2DirEntry*)(block.data + dot1.base.rec_len);

        ent->rec_len += ext2->block_size - 
            (dot1.base.rec_len + dot2.base.rec_len);

        /* Write the block */
        if (EXT2_IFERR(err = EXT2WriteBlock(ext2, blkno, &block)))
        {
            GOTO(done);
        }
    }

    /* Update the inode block pointers and write the inode to disk */
    if (EXT2_IFERR(err = _UpdateInodeBlockPointers(
        ext2,
        *ino,
        &inode,
        ext2->block_size,
        &blkno,
        1)))
    {
        GOTO(done);
    }

    err = EXT2_ERR_NONE;

done:
    return err;
}

static EXT2Err _AppendDirectoryEntry(
    EXT2* ext2, 
    void* data,
    UINT32 size,
    char** current,
    EXT2DirEntry** prev,
    const EXT2DirEntry* ent)
{
    EXT2_DECLARE_ERR(err);
    UINT32 rec_len;
    UINT32 offset;

    /* ATTN: size not used (use to check for bounds violation) */
    (void)size;

    /* Compute size of the new directory entry */
    rec_len = sizeof(*ent) - EXT2_PATH_MAX + ent->name_len;
    rec_len = _NextMult(rec_len, 4);

    /* Compute byte offset into current block */
    offset = ((char*)(*current) - (char*)data) % ext2->block_size;

    /* If new entry would overflow the block */
    if (offset + rec_len > ext2->block_size)
    {
        UINT32 rem = ext2->block_size - offset;

        if (!(*prev))
        {
            GOTO(done);
        }

        /* Adjust previous entry to point to next block */
        (*prev)->rec_len += rem;
        (*current) += rem;
    }

    /* Copy this entry into new buffer */
    {
        EXT2DirEntry* tmp_ent;

        /* Set pointer to next new entry */
        tmp_ent = (EXT2DirEntry*)(*current);

        /* Copy over new entry */
        Memset(tmp_ent, 0, rec_len);
        Memcpy(tmp_ent, ent, sizeof(*ent) - EXT2_PATH_MAX + ent->name_len);
        tmp_ent->rec_len = rec_len;
        (*prev) = tmp_ent;
        (*current) += rec_len;
    }

    err = EXT2_ERR_NONE;

done:
    return err;
}

static EXT2Err _IndexDirectoryToLinkedListDirectory(
    EXT2* ext2,
    const char* rm_name, /* may be null */
    EXT2DirEntry* new_ent, /* may be null */
    const void* data,
    UINT32 size,
    void** new_data,
    UINT32* new_size)
{
    EXT2_DECLARE_ERR(err);
    const char* src = (const char*)data;
    const char* src_end = (const char*)data + size;
    char* dest = NULL;

    /* Check parameters */
    if (!EXT2Valid(ext2) || !data || !size || !new_data || !new_size)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    /* Linked list directory will be smaller than this */
    *new_size = size;

    /* Allocate a buffer to hold 'linked list' directory */
    if (!(*new_data = Calloc(*new_size, 1)))
    {
        GOTO(done);
    }

    /* Set current and end pointers to new buffer */
    dest = (char*)*new_data;

    /* Copy over directory entries (skipping removed entry) */
    {
        EXT2DirEntry* prev = NULL;

        while (src < src_end)
        {
            const EXT2DirEntry* ent;

            /* Set pointer to current entry */
            ent = (const EXT2DirEntry*)src;

            /* Skip blank directory entries */
            if (!ent->name)
            {
                src += ent->rec_len;
                continue;
            }

            /* Skip optional entry to be removed */
            if (rm_name && Strncmp(rm_name, ent->name, ent->name_len) == 0)
            {
                src += ent->rec_len;
                continue;
            }

            if (EXT2_IFERR(err = _AppendDirectoryEntry(
                ext2,
                *new_data,
                *new_size,
                &dest,
                &prev,
                ent)))
            {
                GOTO(done);
            }

            src += ent->rec_len;
        }

        if (new_ent)
        {
            if (EXT2_IFERR(err = _AppendDirectoryEntry(
                ext2,
                *new_data,
                *new_size,
                &dest,
                &prev,
                new_ent)))
            {
                GOTO(done);
            }
        }

        /* Set final entry to point to end of the block */
        if (prev)
        {
            UINT32 offset;
            UINT32 rem;

            /* Compute byte offset into current block */
            offset = (dest - (char*)*new_data) % ext2->block_size;

            /* Compute remaining bytes */
            rem = ext2->block_size - offset;

            /* Set record length of final entry to end of block */
            prev->rec_len += rem;

            /* Advance dest to block boundary */
            dest += rem;
        }

        /* Size down the new blocks size */
        *new_size = (UINT32)(dest - (char*)*new_data);

        /* Perform a sanity check on the new entries */
        if (EXT2_IFERR(err = _CheckDirectoryEntries(
            ext2, *new_data, *new_size)))
        {
            GOTO(done);
        }
    }

    err = EXT2_ERR_NONE;

done:
    return err;
}

static EXT2Err _AddDirectoryEntry(
    EXT2* ext2,
    EXT2Ino dir_ino,
    EXT2Inode* dir_inode,
    const char* filename,
    EXT2DirEntry* new_ent)
{
    EXT2_DECLARE_ERR(err);
    void* blocks = NULL;
    UINT32 blocks_size = 0;
    void* new_blocks = NULL;
    UINT32 new_blocks_size = 0;
    BufU32 blknos = BUF_U32_INITIALIZER;

    /* Load the directory file */
    if (EXT2_IFERR(err = EXT2LoadFileFromInode(
        ext2, 
        dir_inode, 
        &blocks, 
        &blocks_size)))
    {
        GOTO(done);
    }

    /* Load the block numbers (including the block blocks) */
    if (EXT2_IFERR(err = _LoadBlockNumbersFromInode(
        ext2,
        dir_inode,
        1, /* include_block_blocks */
        &blknos)))
    {
        GOTO(done);
    }

    /* Find 'filename' within this directory */
    if ((_FindDirectoryEntry(filename, blocks, blocks_size)))
    {
        err = EXT2_ERR_FILE_NOT_FOUND;
        GOTO(done);
    }

    /* Convert from 'indexed' to 'linked list' directory format */
    if (EXT2_IFERR(err = _IndexDirectoryToLinkedListDirectory(
        ext2,
        NULL, /* rm_name */
        new_ent, /* new_entry */
        blocks,
        dir_inode->i_size,
        &new_blocks,
        &new_blocks_size)))
    {
        GOTO(done);
    }

    /* Count directory entries before and after */
    {
        UINT32 count;
        UINT32 new_count;

        if (EXT2_IFERR(err = _CountDirectoryEntries(
            ext2, 
            blocks, 
            blocks_size, 
            &count)))
        {
            GOTO(done);
        }

        if (EXT2_IFERR(err = _CountDirectoryEntries(
            ext2, 
            new_blocks, 
            new_blocks_size, 
            &new_count)))
        {
            GOTO(done);
        }

        if (count + 1 != new_count)
        {
            GOTO(done);
        }
    }

    /* Return all directory blocks to the free list */
    if (EXT2_IFERR(err = _PutBlocks(ext2, blknos.data, blknos.size)))
    {
        GOTO(done);
    }

    /* Update the directory inode blocks */
    if (EXT2_IFERR(err = _UpdateInodeDataBlocks(
        ext2,
        dir_ino,
        dir_inode,
        new_blocks,
        new_blocks_size,
        1))) /* is_dir */
    {
        GOTO(done);
    }

    err = EXT2_ERR_NONE;

done:

    if (blocks)
        Free(blocks);

    if (new_blocks)
        Free(new_blocks);

    BufU32Release(&blknos);

    return err;
}

EXT2Err EXT2Put(
    EXT2* ext2,
    const void* data,
    UINT32 size,
    const char* path,
    UINT16 mode)
{
    EXT2_DECLARE_ERR(err);
    char dirname[EXT2_PATH_MAX];
    char filename[EXT2_PATH_MAX];
    EXT2Ino file_ino;
    EXT2Inode file_inode;
    EXT2Ino dir_ino;
    EXT2Inode dir_inode;
    BufU32 blknos = BUF_U32_INITIALIZER;
    EXT2DirectoryEntBuf ent_buf;
    EXT2DirEntry* ent = &ent_buf.base;

    /* Check parameters */
    if (!EXT2Valid(ext2) || !data || !size || !path)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    /* Reject attempts to copy directories */
    if (mode & EXT2_S_IFDIR)
    {
        GOTO(done);
    }

    /* Split the path */
    if (EXT2_IFERR(err = _SplitFullPath(path, dirname, filename)))
    {
        GOTO(done);
    }

    /* Read inode for the directory */
    if (EXT2_IFERR(err = EXT2PathToInode(ext2, dirname, &dir_ino, &dir_inode)))
    {
        GOTO(done);
    }

    /* Read inode for the file (if any) */
    if (!EXT2_IFERR(err = EXT2PathToInode(ext2, path, &file_ino, &file_inode)))
    {
        /* Disallow overwriting of directory */
        if (!(file_inode.i_mode & EXT2_S_IFDIR))
        {
            GOTO(done);
        }

        /* Delete the file if it exists */
        if (EXT2_IFERR(err = EXT2Rm(ext2, path)))
        {
            GOTO(done);
        }
    }

    /* Write the blocks of the file */
    if (EXT2_IFERR(err = _WriteData(ext2, data, size, &blknos)))
        GOTO(done);

    /* Create an inode for this new file */
    if (EXT2_IFERR(err = _CreateFileInode(
        ext2, 
        data, 
        size, 
        mode, 
        blknos.data, 
        blknos.size,
        &file_ino)))
    {
        GOTO(done);
    }

    /* Initialize the new directory entry */
    {
        /* ent->inode */
        ent->inode = file_ino;

        /* ent->name_len */
        ent->name_len = (UINT32)Strlen(filename);

        /* ent->file_type */
        ent->file_type = EXT2_FT_REG_FILE;

        /* ent->name[] */
        ent->name[0] = '\0';
        Strncat(ent->name, sizeof(ent->name), filename, EXT2_PATH_MAX-1);

        /* ent->rec_len */
        ent->rec_len = 
            _NextMult(sizeof(*ent)- EXT2_PATH_MAX + ent->name_len, 4);
    }

    /* Create new entry for this file in the directory inode */
    if (EXT2_IFERR(err = _AddDirectoryEntry(
        ext2, 
        dir_ino, 
        &dir_inode, 
        filename, 
        ent)))
    {
        GOTO(done);
    }

    err = EXT2_ERR_NONE;

done:

    BufU32Release(&blknos);

    return err;
}

EXT2Err EXT2MkDir(
    EXT2* ext2,
    const char* path,
    UINT16 mode)
{
    EXT2_DECLARE_ERR(err);
    char dirname[EXT2_PATH_MAX];
    char basename[EXT2_PATH_MAX];
    EXT2Ino dir_ino;
    EXT2Inode dir_inode;
    EXT2Ino base_ino;
    EXT2Inode base_inode;
    EXT2DirectoryEntBuf ent_buf;
    EXT2DirEntry* ent = &ent_buf.base;

    /* Check parameters */
    if (!EXT2Valid(ext2) || !path || !mode)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    /* Reject is not a directory */
    if (!(mode & EXT2_S_IFDIR))
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    /* Split the path */
    if (EXT2_IFERR(err = _SplitFullPath(path, dirname, basename)))
    {
        GOTO(done);
    }

    /* Read inode for 'dirname' */
    if (EXT2_IFERR(err = EXT2PathToInode(
        ext2, dirname, &dir_ino, &dir_inode)))
    {
        GOTO(done);
    }

    /* Fail if the directory already exists */
    if (!EXT2_IFERR(err = EXT2PathToInode(
        ext2, path, &base_ino, &base_inode)))
    {
        GOTO(done);
    }

    /* Create the directory inode and its one block */
    if (EXT2_IFERR(err = _CreateDirInodeAndBlock(
        ext2, 
        dir_ino,
        mode,
        &base_ino)))
    {
        GOTO(done);
    }

    /* Initialize the new directory entry */
    {
        /* ent->inode */
        ent->inode = base_ino;

        /* ent->name_len */
        ent->name_len = (UINT32)Strlen(basename);

        /* ent->file_type */
        ent->file_type = EXT2_FT_DIR;

        /* ent->name[] */
        ent->name[0] = '\0';
        Strncat(ent->name, sizeof(ent->name), basename, EXT2_PATH_MAX-1);

        /* ent->rec_len */
        ent->rec_len = 
            _NextMult(sizeof(*ent) - EXT2_PATH_MAX + ent->name_len, 4);
    }

    /* Include new child directory in parent directory's i_links_count */
    dir_inode.i_links_count++;

    /* Create new entry for this file in the directory inode */
    if (EXT2_IFERR(err = _AddDirectoryEntry(
        ext2, dir_ino, &dir_inode, basename, ent)))
    {
        GOTO(done);
    }

    err = EXT2_ERR_NONE;

done:
    return err;
}

EXT2Err EXT2Lsr(
    EXT2* ext2,
    const char* root,
    StrArr* paths)
{
    EXT2_DECLARE_ERR(err);
    EXT2_DIR* dir = NULL;
    EXT2DirEnt* ent;
    char path[EXT2_PATH_MAX];
    StrArr dirs = STRARR_INITIALIZER;

    /* Check parameters */
    if (!ext2 || !root || !paths)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    /* Open the directory */
    if (!(dir = EXT2OpenDir(ext2, root)))
    {
        GOTO(done);
    }

    /* For each entry */
    while ((ent = EXT2ReadDir(dir)))
    {
        if (Strcmp(ent->d_name, ".") == 0 || Strcmp(ent->d_name, "..") == 0)
            continue;

        Strlcpy(path, root, sizeof(path));

        if (Strcmp(root, "/") != 0)
            Strlcat(path, "/", sizeof(path));

        Strlcat(path, ent->d_name, sizeof(path));

        /* Append to paths[] array */
        if (StrArrAppend(paths, path) != 0)
        {
            err = EXT2_ERR_OUT_OF_MEMORY;
            GOTO(done);
        }
        
        /* Append to dirs[] array */
        if (ent->d_type & EXT2_DT_DIR)
        {
            if (StrArrAppend(&dirs, path) != 0)
            {
                err = EXT2_ERR_OUT_OF_MEMORY;
                GOTO(done);
            }
        }
    }

    /* Recurse into child directories */
    {
        UINTN i;

        for (i = 0; i < dirs.size; i++)
        {
            if (EXT2Lsr(ext2, dirs.data[i], paths) != EXT2_ERR_NONE)
            {
                GOTO(done);
            }
        }
    }

    err = EXT2_ERR_NONE;

done:

    /* Close the directory */
    if (dir)
        EXT2CloseDir(dir);

    StrArrRelease(&dirs);

    if (err != EXT2_ERR_NONE)
    {
        StrArrRelease(paths);
        Memset(paths, 0, sizeof(StrArr));
    }

    return err;
}

EXT2Err EXT2HashDir(
    EXT2* ext2,
    const char* root,
    SHA1Hash* sha1,
    SHA256Hash* sha256)
{
    EXT2_DECLARE_ERR(err);
    StrArr paths = STRARR_INITIALIZER;
    SHA1Context sha1Context;
    SHA256Context sha256Context;
    void* data = NULL;
    UINT32 size;

    /* Check for null parameters */
    if (!ext2 || !root || !sha1 || !sha256)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        GOTO(done);
    }

    /* Clear the hashes */

    if (sha1)
        SHA1Init(&sha1Context);

    if (sha256)
        SHA256Init(&sha256Context);

    /* Form an array of paths */
    if (EXT2_IFERR(err = EXT2Lsr(ext2, root, &paths)))
    {
        GOTO(done);
    }

    /* Sort the path names */
    StrArrSort(&paths);

    /* Hash the contents of all files in this directory */
    {
        UINTN i;

        for (i = 0; i < paths.size; i++)
        {
            EXT2Ino ino;
            EXT2Inode inode;

            if (EXT2_IFERR(err = EXT2PathToInode(
                ext2, 
                paths.data[i], 
                &ino, 
                &inode)))
            {
                GOTO(done);
            }

            /* Skip directories */
            if (inode.i_mode & EXT2_S_IFDIR)
                continue;

            /* Load the file into memory */
            if (EXT2_IFERR(err = EXT2LoadFileFromInode(
                ext2, 
                &inode, 
                &data, 
                &size)))
            {
                GOTO(done);
            }

            /* Update the SHA1 hash */
            if (sha1 && !SHA1Update(&sha1Context, data, size))
            {
                GOTO(done);
            }

            /* Update the SHA256 hash */
            if (sha256 && !SHA256Update(&sha256Context, data, size))
            {
                GOTO(done);
            }

            /* Release the file memory */
            Free(data);
            data = NULL;
        }
    }

    /* Finalize the SHA-1 hash */
    if (sha1)
        SHA1Final(&sha1Context, sha1);

    /* Finalize the SHA-256 hash */
    if (sha256)
        SHA256Final(&sha256Context, sha256);

    err = EXT2_ERR_NONE;

done:

    if (data)
        Free(data);

    StrArrRelease(&paths);

    return err;
}

/*
**==============================================================================
**
** EXT2File
**
**==============================================================================
*/

struct _EXT2File
{
    EXT2* ext2;
    EXT2Inode inode;
    BufU32 blknos;
    UINTN offset;
    BOOLEAN eof;
};

EXT2File* EXT2OpenFile(
    EXT2* ext2,
    const char* path,
    EXT2FileMode mode)
{
    EXT2File* file = NULL;
    EXT2Ino ino;
    EXT2Inode inode;
    BufU32 blknos = BUF_U32_INITIALIZER;

    /* Reject null parameters */
    if (!ext2 || !path)
        goto done;

    /* Find the inode for this file */
    if (EXT2PathToInode(ext2, path, &ino, &inode) != EXT2_ERR_NONE)
        goto done;

    /* Load the block numbers for this inode */
    if (_LoadBlockNumbersFromInode(
        ext2, 
        &inode,
        0, /* include_block_blocks */
        &blknos) != EXT2_ERR_NONE)
    {
        goto done;
    }

    /* Allocate and initialize the file object */
    {
        if (!(file = (EXT2File*)Calloc(1, sizeof(EXT2File))))
            goto done;

        file->ext2 = ext2;
        file->inode = inode;
        file->blknos = blknos;
        file->offset = 0;
    }

done:
    if (!file)
        BufU32Release(&blknos);

    return file;
}

INTN EXT2ReadFile(
    EXT2File* file,
    void* data,
    UINTN size)
{
    INTN nread = -1;
    UINT32 first;
    UINT32 i;
    UINTN remaining;
    UINT8* end = (UINT8*)data;

    /* Check parameters */
    if (!file || !file->ext2 || !data)
        goto done;

    /* The index of the first block to read: ext2->blknos[first] */
    first = file->offset / file->ext2->block_size;

    /* The number of bytes remaining to be read */
    remaining = size;

    /* Read the data block-by-block */
    for (i = first; i < file->blknos.size && remaining > 0 && !file->eof; i++)
    {
        EXT2Block block;
        UINT32 offset;

        if (EXT2ReadBlock(
            file->ext2, 
            file->blknos.data[i], 
            &block) != EXT2_ERR_NONE)
        {
            goto done;
        }

        /* The offset of the data within this block */
        offset = file->offset % file->ext2->block_size;

        /* Copy the minimum of these to the caller's buffer:
         *     remaining
         *     block-bytes-remaining
         *     file-bytes-remaining
         */
        {
            UINTN copyBytes;

            /* Bytes to copy to user buffer */
            copyBytes = remaining;

            /* Reduce copyBytes to bytes remaining in the block? */
            {
                UINTN blockBytesRemaining = file->ext2->block_size - offset;

                if (blockBytesRemaining < copyBytes)
                    copyBytes = blockBytesRemaining;
            }

            /* Reduce copyBytes to bytes remaining in the file? */
            {
                UINTN fileBytesRemaining = file->inode.i_size - file->offset;

                if (fileBytesRemaining < copyBytes)
                {
                    copyBytes = fileBytesRemaining;
                    file->eof = TRUE;
                }
            }

            /* Copy data to user buffer */
            Memcpy(end, block.data + offset, copyBytes);
            remaining -= copyBytes;
            end += copyBytes;
            file->offset += copyBytes;
        }
    }

    /* Calculate number of bytes read */
    nread = size - remaining;

done:
    return nread;
}

INTN EXT2WriteFile(
    EXT2File* file,
    const void* data,
    UINTN size)
{
    /* Unsupported */
    return -1;
}

int EXT2FlushFile(
    EXT2File* file)
{
    int rc = -1;

    /* Check parameters */
    if (!file || !file->ext2)
        goto done;

    /* No-op because EXT2 is always flushed */

    rc = 0;

done:
    return rc;
}

int EXT2CloseFile(
    EXT2File* file)
{
    int rc = -1;

    /* Check parameters */
    if (!file || !file->ext2)
        goto done;

    /* Release the block numbers buffer */
    BufU32Release(&file->blknos);

    /* Release the file object */
    Free(file);

    rc = 0;

done:
    return rc;
}

int EXT2SeekFile(
    EXT2File* file,
    INTN offset)
{
    int rc = -1;

    if (!file)
        goto done;

    if (offset > file->inode.i_size)
        goto done;

    file->offset = offset;

    rc = 0;

done:
    return rc;
}

INTN EXT2TellFile(
    EXT2File* file)
{
    return file ? file->offset : -1;
}

INTN EXT2SizeFile(
    EXT2File* file)
{
    return file ? file->inode.i_size : -1;
}

EXT2Err EXT2WhoseBlock(
    EXT2* ext2,
    UINT32 blkno,
    BOOLEAN* found,
    char path[EXT2_PATH_MAX])
{
    EXT2_DECLARE_ERR(err);
    StrArr paths = STRARR_INITIALIZER;
    UINTN i;

    /* Reject null parameters */
    if (!ext2 || !found|| !path)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        goto done;
    }

    /* Initialize output path parameter */
    *path = '\0';

    /* Set found to false initially */
    *found = FALSE;

    /* Form an array of paths */
    if (EXT2_IFERR(err = EXT2Lsr(ext2, "/", &paths)))
    {
        goto done;
    }

    /* Search blocks for each file located */
    for (i = 0; i < paths.size; i++)
    {
        EXT2Ino ino;
        EXT2Inode inode;
        BufU32 blknos = BUF_U32_INITIALIZER;
        UINTN j;

        /* Find the inode for this file */
        if (EXT2PathToInode(ext2, paths.data[i], &ino, &inode) != EXT2_ERR_NONE)
            goto done;

        /* Load the block numbers for this inode */
        if (EXT2_IFERR(err = _LoadBlockNumbersFromInode(
            ext2, 
            &inode,
            1, /* include_block_blocks */
            &blknos)))
        {
            goto done;
        }

        /* Search the block list for this file */
        for (j = 0; j < blknos.size; j++)
        {
            if (blkno == blknos.data[j])
            {
                *found = TRUE;
                Strlcpy(path, paths.data[i], EXT2_PATH_MAX);
                break;
            }
        }

        BufU32Release(&blknos);
    }

    err = EXT2_ERR_NONE;

done:

    StrArrRelease(&paths);

    return err;
}

EXT2Err EXT2GetBlockNumbers(
    EXT2* ext2,
    const char* path,
    BufU32* blknos)
{
    EXT2_DECLARE_ERR(err);
    EXT2Ino ino;
    EXT2Inode inode;

    /* Reject null parameters */
    if (!ext2 || !path || !blknos)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        goto done;
    }

    /* Find the inode for this file */
    if (EXT2PathToInode(ext2, path, &ino, &inode) != EXT2_ERR_NONE)
        goto done;

    /* Load the block numbers for this inode */
    if (EXT2_IFERR(err = _LoadBlockNumbersFromInode(
        ext2, 
        &inode,
        1, /* include_block_blocks */
        blknos)))
    {
        goto done;
    }

    err = EXT2_ERR_NONE;

done:

    return err;
}

UINTN EXT2BlknoToLBA(
    const EXT2* ext2,
    UINT32 blkno)
{
    return BlockOffset(blkno, ext2->block_size) / BLKDEV_BLKSIZE;
}

EXT2Err EXT2GetFirstBlkno(
    const EXT2* ext2,
    EXT2Ino ino,
    UINT32* blkno)
{
    EXT2_DECLARE_ERR(err);
    EXT2Inode inode;

    /* Initialize output parameter */
    if (blkno)
        *blkno = 0;

    /* Reject null parameters */
    if (!ext2 || !blkno)
    {
        err = EXT2_ERR_INVALID_PARAMETER;
        goto done;
    }

    /* Read the inode into memory */
    if (EXT2ReadInode(ext2, ino, &inode) != EXT2_ERR_NONE)
        goto done;

    /* Get the block number from the inode */
    *blkno = inode.i_block[0];

    err = EXT2_ERR_NONE;

done:

    return err;
}

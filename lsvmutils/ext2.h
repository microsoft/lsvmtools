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
#ifndef _ext2_h
#define _ext2_h

#include "config.h"

#if !defined(BUILD_EFI)
# include <stdio.h>
# include <stdlib.h>
#endif

#include "blkdev.h"
#include "strarr.h"
#include "sha.h"
#include "buf.h"

/*
**==============================================================================
**
** basic types:
**
**==============================================================================
*/

typedef unsigned int EXT2Ino;
typedef unsigned int EXT2Off;

/*
**==============================================================================
**
** errors:
**
**==============================================================================
*/

#if 0
# define EXT2_USE_TYPESAFE_ERRORS
#endif

#define EXT2_IFERR(ERR) __EXT2IfErr(ERR)
#define EXT2_ERRNO(ERR) __EXT2Errno(ERR)

#if defined(EXT2_USE_TYPESAFE_ERRORS)

typedef struct _EXT2Err
{
    int errno;
}
EXT2Err;

static __inline BOOLEAN __EXT2IfErr(EXT2Err err)
{
    return err.errno ? 1 : 0;
}

static __inline unsigned int __EXT2Errno(EXT2Err err)
{
    return err.errno;
}

extern const EXT2Err EXT2_ERR_NONE;
extern const EXT2Err EXT2_ERR_FAILED;
extern const EXT2Err EXT2_ERR_INVALID_PARAMETER;
extern const EXT2Err EXT2_ERR_FILE_NOT_FOUND;
extern const EXT2Err EXT2_ERR_BAD_MAGIC;
extern const EXT2Err EXT2_ERR_UNSUPPORTED;
extern const EXT2Err EXT2_ERR_OUT_OF_MEMORY;
extern const EXT2Err EXT2_ERR_FAILED_TO_READ_SUPERBLOCK;
extern const EXT2Err EXT2_ERR_FAILED_TO_READ_GROUPS;
extern const EXT2Err EXT2_ERR_FAILED_TO_READ_INODE;
extern const EXT2Err EXT2_ERR_UNSUPPORTED_REVISION;
extern const EXT2Err EXT2_ERR_OPEN_FAILED;
extern const EXT2Err EXT2_ERR_BUFFER_OVERFLOW;
extern const EXT2Err EXT2_ERR_SEEK_FAILED;
extern const EXT2Err EXT2_ERR_READ_FAILED;
extern const EXT2Err EXT2_ERR_WRITE_FAILED;
extern const EXT2Err EXT2_ERR_UNEXPECTED;
extern const EXT2Err EXT2_ERR_SANITY_CHECK_FAILED;
extern const EXT2Err EXT2_ERR_BAD_BLKNO;
extern const EXT2Err EXT2_ERR_BAD_INO;
extern const EXT2Err EXT2_ERR_BAD_GRPNO;
extern const EXT2Err EXT2_ERR_BAD_MULTIPLE;
extern const EXT2Err EXT2_ERR_EXTRANEOUS_DATA;
extern const EXT2Err EXT2_ERR_BAD_SIZE;
extern const EXT2Err EXT2_ERR_PATH_TOO_LONG;

#else /* defined(EXT2_USE_TYPESAFE_ERRORS) */

typedef enum _EXT2Err
{
    EXT2_ERR_NONE,
    EXT2_ERR_FAILED,
    EXT2_ERR_INVALID_PARAMETER,
    EXT2_ERR_FILE_NOT_FOUND,
    EXT2_ERR_BAD_MAGIC,
    EXT2_ERR_UNSUPPORTED,
    EXT2_ERR_OUT_OF_MEMORY,
    EXT2_ERR_FAILED_TO_READ_SUPERBLOCK,
    EXT2_ERR_FAILED_TO_READ_GROUPS,
    EXT2_ERR_FAILED_TO_READ_INODE,
    EXT2_ERR_UNSUPPORTED_REVISION,
    EXT2_ERR_OPEN_FAILED,
    EXT2_ERR_BUFFER_OVERFLOW,
    EXT2_ERR_SEEK_FAILED,
    EXT2_ERR_READ_FAILED,
    EXT2_ERR_WRITE_FAILED,
    EXT2_ERR_UNEXPECTED,
    EXT2_ERR_SANITY_CHECK_FAILED,
    EXT2_ERR_BAD_BLKNO,
    EXT2_ERR_BAD_INO,
    EXT2_ERR_BAD_GRPNO,
    EXT2_ERR_BAD_MULTIPLE,
    EXT2_ERR_EXTRANEOUS_DATA,
    EXT2_ERR_BAD_SIZE,
    EXT2_ERR_PATH_TOO_LONG,
}
EXT2Err;

static __inline BOOLEAN __EXT2IfErr(EXT2Err err)
{
    return err ? 1 : 0;
}

static __inline unsigned int __EXT2Errno(EXT2Err err)
{
    return err;
}

#endif /* defined(EXT2_USE_TYPESAFE_ERRORS) */

const char* EXT2ErrStr(
    EXT2Err err);

/*
**==============================================================================
**
** structure typedefs:
**
**==============================================================================
*/

typedef struct _EXT2 EXT2;
typedef struct _EXT2Block EXT2Block;
typedef struct _EXT2SuperBlock EXT2SuperBlock;
typedef struct _EXT2GroupDesc EXT2GroupDesc;
typedef struct _EXT2Inode EXT2Inode;
typedef struct _EXT2DirectoryEntry EXT2DirEntry;
typedef struct _EXT2_DIR EXT2_DIR;

/*
**==============================================================================
**
** blocks:
**
**==============================================================================
*/

#define EXT2_MAX_BLOCK_SIZE (8 * 1024)

struct _EXT2Block
{
    UINT8 data[EXT2_MAX_BLOCK_SIZE];
    UINT32 size;
};

EXT2Err EXT2ReadBlock(
    const EXT2* ext2,
    UINT32 blkno,
    EXT2Block* block);

EXT2Err EXT2WriteBlock(
    const EXT2* ext2,
    UINT32 blkno,
    const EXT2Block* block);

/*
**==============================================================================
**
** super block:
**
**==============================================================================
*/

/* Offset of super block from start of file system */
#define EXT2_BASE_OFFSET 1024

#define EXT2_S_MAGIC 0xEF53
#define EXT2_GOOD_OLD_REV 0 /* Revision 0 EXT2 */
#define EXT2_DYNAMIC_REV 1 /* Revision 1 EXT2 */

struct _EXT2SuperBlock
{
    /* General */
    UINT32 s_inodes_count;
    UINT32 s_blocks_count;
    UINT32 s_r_blocks_count;
    UINT32 s_free_blocks_count;
    UINT32 s_free_inodes_count;
    UINT32 s_first_data_block;
    UINT32 s_log_block_size;
    UINT32 s_log_frag_size;
    UINT32 s_blocks_per_group;
    UINT32 s_frags_per_group;
    UINT32 s_inodes_per_group;
    UINT32 s_mtime;
    UINT32 s_wtime;
    UINT16 s_mnt_count;
    UINT16 s_max_mnt_count;
    UINT16 s_magic;
    UINT16 s_state;
    UINT16 s_errors;
    UINT16 s_minor_rev_level;
    UINT32 s_lastcheck;
    UINT32 s_checkinterval;
    UINT32 s_creator_os;
    UINT32 s_rev_level;
    UINT16 s_def_resuid;
    UINT16 s_def_resgid;

    /* DYNAMIC_REV Specific */
    UINT32 s_first_ino;
    UINT16 s_inode_size;
    UINT16 s_block_group_nr;
    UINT32 s_feature_compat;
    UINT32 s_feature_incompat;
    UINT32 s_feature_ro_compat;
    UINT8 s_uuid[16];
    UINT8 s_volume_name[16];
    UINT8 s_last_mounted[64];
    UINT32 s_algo_bitmap;

    /* Performance Hints */
    UINT8 s_prealloc_blocks;
    UINT8 s_prealloc_dir_blocks;
    UINT16 __alignment;

    /* Journaling Support */
    UINT8 s_journal_uuid[16];
    UINT32 s_journal_inum;
    UINT32 s_journal_dev;
    UINT32 s_last_orphan;

    /* Directory Indexing Support */
    UINT32 s_hash_seed[4];
    UINT8 s_def_hash_version;
    UINT8 padding[3];

    /* Other options */
    UINT32 s_default_mount_options;
    UINT32 s_first_meta_bg;
    UINT8 __unused[760];
};

void EXT2DumpSuperBlock(
    const EXT2SuperBlock* sb);

/*
**==============================================================================
**
** groups:
**
**==============================================================================
*/

struct _EXT2GroupDesc
{
    UINT32 bg_block_bitmap;
    UINT32 bg_inode_bitmap;
    UINT32 bg_inode_table;
    UINT16 bg_free_blocks_count;
    UINT16 bg_free_inodes_count;
    UINT16 bg_used_dirs_count;
    UINT16 bg_pad;
    UINT8 bg_reserved[12];
};

/*
**==============================================================================
**
** inodes:
**
**==============================================================================
*/

#define EXT2_BAD_INO 1
#define EXT2_ROOT_INO 2
#define EXT2_ACL_IDX_INO 3
#define EXT2_ACL_DATA_INO 4
#define EXT2_BOOT_LOADER_INO 5
#define EXT2_UNDEL_DIR_INO 6
#define EXT2_FIRST_INO 11

#define EXT2_SINGLE_INDIRECT_BLOCK 12
#define EXT2_DOUBLE_INDIRECT_BLOCK 13
#define EXT2_TRIPLE_INDIRECT_BLOCK 14

#define EXT2_S_IFSOCK 0xC000
#define EXT2_S_IFLNK 0xA000
#define EXT2_S_IFREG 0x8000
#define EXT2_S_IFBLK 0x6000
#define EXT2_S_IFDIR 0x4000
#define EXT2_S_IFCHR 0x2000
#define EXT2_S_IFIFO 0x1000

#define EXT2_S_ISUID 0x0800
#define EXT2_S_ISGID 0x0400
#define EXT2_S_ISVTX 0x0200

#define EXT2_S_IRUSR 0x0100
#define EXT2_S_IWUSR 0x0080
#define EXT2_S_IXUSR 0x0040
#define EXT2_S_IRGRP 0x0020
#define EXT2_S_IWGRP 0x0010
#define EXT2_S_IXGRP 0x0008
#define EXT2_S_IROTH 0x0004
#define EXT2_S_IWOTH 0x0002
#define EXT2_S_IXOTH 0x0001

#define EXT2_SECRM_FL 0x00000001
#define EXT2_UNRM_FL 0x00000002
#define EXT2_COMPR_FL 0x00000004
#define EXT2_SYNC_FL 0x00000008
#define EXT2_IMMUTABLE_FL 0x00000010
#define EXT2_APPEND_FL 0x00000020
#define EXT2_NODUMP_FL 0x00000040
#define EXT2_NOATIME_FL 0x00000080
#define EXT2_DIRTY_FL 0x00000100
#define EXT2_COMPRBLK_FL 0x00000200
#define EXT2_NOCOMPR_FL 0x00000400
#define EXT2_ECOMPR_FL 0x00000800
#define EXT2_BTREE_FL 0x00001000
#define EXT2_INDEX_FL 0x00001000
#define EXT2_IMAGIC_FL 0x00002000
#define EXT3_JOURNAL_DATA_FL 0x00004000
#define EXT2_RESERVED_FL 0x80000000

struct _EXT2Inode
{
    UINT16 i_mode;
    UINT16 i_uid;
    UINT32 i_size;
    UINT32 i_atime;
    UINT32 i_ctime;
    UINT32 i_mtime;
    UINT32 i_dtime;
    UINT16 i_gid;
    UINT16 i_links_count;
    UINT32 i_blocks;
    UINT32 i_flags;
    UINT32 i_osd1;
    /*
       0:11 -- direct block numbers
       12 -- indirect block number
       13 -- double-indirect block number
       14 -- triple-indirect block number
    */
    UINT32 i_block[15];
    UINT32 i_generation;
    UINT32 i_file_acl;
    UINT32 i_dir_acl;
    UINT32 i_faddr;
    UINT8 i_osd2[12];
    UINT8 dummy[128]; /* sometimes the inode is bigger */
};

void EXT2DumpInode(
    const EXT2* ext2,
    const EXT2Inode* inode);

EXT2Err EXT2ReadInode(
    const EXT2* ext2,
    EXT2Ino ino,
    EXT2Inode* inode);

EXT2Err EXT2PathToIno(
    const EXT2* ext2,
    const char* path,
    UINT32* ino);

EXT2Err EXT2PathToInode(
    const EXT2* ext2,
    const char* path,
    EXT2Ino* ino,
    EXT2Inode* inode);

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
    EXT2Block* block);

EXT2Err EXT2WriteBlockBitmap(
    const EXT2* ext2,
    UINT32 group_index,
    const EXT2Block* block);

EXT2Err EXT2readReadInodeBitmap(
    const EXT2* ext2,
    UINT32 group_index,
    EXT2Block* block);

/*
**==============================================================================
**
** directories:
**
**==============================================================================
*/

#define EXT2_PATH_MAX 256

#define EXT2_FT_UNKNOWN 0
#define EXT2_FT_REG_FILE 1
#define EXT2_FT_DIR 2
#define EXT2_FT_CHRDEV 3
#define EXT2_FT_BLKDEV 4
#define EXT2_FT_FIFO 5
#define EXT2_FT_SOCK 6
#define EXT2_FT_SYMLINK 7

struct _EXT2DirectoryEntry
{
    UINT32 inode;
    UINT16 rec_len;
    UINT8 name_len;
    UINT8 file_type;
    char name[EXT2_PATH_MAX];
};

#define EXT2_DX_HASH_LEGACY 0
#define EXT2_DX_HASH_HALF_MD4 1
#define EXT2_DX_HASH_TEA 2

#define EXT2_DT_UNKNOWN 0
#define EXT2_DT_FIFO 1
#define EXT2_DT_CHR 2
#define EXT2_DT_DIR 4
#define EXT2_DT_BLK 6
#define EXT2_DT_REG 8
#define EXT2_DT_LNK 10
#define EXT2_DT_SOCK 12
#define EXT2_DT_WHT 14

typedef struct _EXT2DirEnt 
{
   EXT2Ino d_ino;
   EXT2Off d_off;
   UINT16 d_reclen;
   UINT8 d_type;
   char d_name[EXT2_PATH_MAX];
}
EXT2DirEnt;

EXT2_DIR *EXT2OpenDir(
    const EXT2* ext2,
    const char *name);

EXT2_DIR *EXT2OpenDirIno(
    const EXT2* ext2,
    EXT2Ino ino);

EXT2DirEnt *EXT2ReadDir(
    EXT2_DIR *dir);

EXT2Err EXT2CloseDir(
    EXT2_DIR* dir);

EXT2Err EXT2ListDirInode(
    const EXT2* ext2,
    UINT32 global_ino,
    EXT2DirEnt** entries,
    UINT32* num_entries);

/*
**==============================================================================
**
** files:
**
**==============================================================================
*/

EXT2Err EXT2LoadFileFromInode(
    const EXT2* ext2,
    const EXT2Inode* inode,
    void** data,
    UINT32* size);

EXT2Err EXT2LoadFileFromPath(
    const EXT2* ext2,
    const char* path,
    void** data,
    UINT32* size);

EXT2Err EXT2LoadFile(
    const char* path, 
    void** data,
    UINT32* size);

/*
**==============================================================================
**
** EXT2:
**
**==============================================================================
*/

struct _EXT2
{
    Blkdev* dev;
    EXT2SuperBlock sb;
    UINT32 block_size; /* block size in bytes */
    UINT32 group_count;
    EXT2GroupDesc* groups;
    EXT2Inode root_inode;
};

static __inline BOOLEAN EXT2Valid(
    const EXT2* ext2)
{
    return ext2 != NULL && ext2->sb.s_magic == EXT2_S_MAGIC;
}

EXT2Err EXT2New(
    Blkdev* dev,
    EXT2** ext2);

void EXT2Delete(
    EXT2* ext2);

EXT2Err EXT2Dump(
    const EXT2* ext2);

EXT2Err EXT2Check(
    const EXT2* ext2);

EXT2Err EXT2Trunc(
    EXT2* ext2,
    const char* path);

EXT2Err EXT2Update(
    EXT2* ext2,
    const void* data,
    UINT32 size,
    const char* path);

EXT2Err EXT2Rm(
    EXT2* ext2,
    const char* path);

/* rw-r--r-- */
#define EXT2_FILE_MODE_RW0_R00_R00 \
    (EXT2_S_IFREG|EXT2_S_IRUSR|EXT2_S_IWUSR|EXT2_S_IRGRP|EXT2_S_IROTH)

/* rw------- */
#define EXT2_FILE_MODE_RW0_000_000 (EXT2_S_IFREG|EXT2_S_IRUSR|EXT2_S_IWUSR)

EXT2Err EXT2Put(
    EXT2* ext2,
    const void* data,
    UINT32 size,
    const char* path,
    UINT16 mode); /* See EXT2_S_* flags above */

/* rwxrx-rx- */
#define EXT2_DIR_MODE_RWX_R0X_R0X \
    (EXT2_S_IFDIR | \
    (EXT2_S_IRUSR|EXT2_S_IWUSR|EXT2_S_IXUSR) | \
    (EXT2_S_IRGRP|EXT2_S_IXGRP) | \
    (EXT2_S_IROTH|EXT2_S_IXOTH))

EXT2Err EXT2MkDir(
    EXT2* ext2,
    const char* path,
    UINT16 mode); /* See EXT2_S_* flags above */

EXT2Err EXT2Lsr(
    EXT2* ext2,
    const char* root,
    StrArr* paths);

EXT2Err EXT2HashDir(
    EXT2* ext2,
    const char* root,
    SHA1Hash* sha1,
    SHA256Hash* sha256);

/*
**==============================================================================
**
** EXT2File:
**
**==============================================================================
*/

typedef struct _EXT2File EXT2File;

typedef enum _EXT2FileMode
{
    EXT2FILE_RDWR,
    EXT2FILE_RDONLY,
    EXT2FILE_WRONLY
}
EXT2FileMode;

EXT2File* EXT2OpenFile(
    EXT2* ext2,
    const char* path,
    EXT2FileMode mode);

INTN EXT2ReadFile(
    EXT2File* file,
    void* data,
    UINTN size);

INTN EXT2WriteFile(
    EXT2File* file,
    const void* data,
    UINTN size);

int EXT2SeekFile(
    EXT2File* file,
    INTN offset);

INTN EXT2TellFile(
    EXT2File* file);

INTN EXT2SizeFile(
    EXT2File* file);

int EXT2FlushFile(
    EXT2File* file);

int EXT2CloseFile(
    EXT2File* file);

EXT2Err EXT2WhoseBlock(
    EXT2* ext2,
    UINT32 blkno,
    BOOLEAN* found,
    char path[EXT2_PATH_MAX]);

EXT2Err EXT2GetBlockNumbers(
    EXT2* ext2,
    const char* path,
    BufU32* blknos);

UINTN EXT2BlknoToLBA(
    const EXT2* ext2,
    UINT32 blkno);

/* Get the block number (LBA) of the first block of this file */
EXT2Err EXT2GetFirstBlkno(
    const EXT2* ext2,
    EXT2Ino ino,
    UINT32* blkno);

#endif /* _ext2_h */

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
#ifndef _vfat_h
#define _vfat_h

#include "config.h"
#include "eficommon.h"
#include "blkdev.h"
#include "buf.h"
#include "strarr.h"

#define VFAT_SECTOR_SIZE 512

#define VFAT_LEAD_SIG_INITIALIZER { 0x52, 0x52, 0x61, 0x41 }

#define VFAT_STRUC_SIG_INITIALIZER { 0x72, 0x72, 0x41, 0x61 }

#define VFAT_PATH_SIZE 256

typedef struct _VFATBPB32
{
    UINT32 FATSz32;
    UINT16 ExtFlags;
    UINT16 FSVer;
    UINT32 RootClus;
    UINT16 FSInfo;
    UINT16 BkBootSec;
    UINT8 Reserved0[12];
    UINT8 DrvNum;
    UINT8 Reserved1;
    UINT8 BootSig;
    UINT32 VolID;
    UINT8 VolLab[11];
    UINT8 FilSysType[8];
}
__attribute__((packed))
VFATBPB32;

typedef struct _VFATBPB12
{
    UINT8 NumPHDrive;
    UINT8 Reserved0;
    UINT8 BootSig;
    UINT32 NumSerial;
    UINT8 VolLab[11];
    UINT8 FilSysType[8];
}
__attribute__((packed))
VFATBPB12;

typedef struct _VFATBPB16
{
    UINT8 NumPHDrive;
    UINT8 Reserved0;
    UINT8 BootSig;
    UINT32 NumSerial;
    UINT8 VolLab[11];
    UINT8 FilSysType[8];
}
__attribute__((packed))
VFATBPB16;

/* Located at first sector (512 bytes) */
typedef struct _VFATBPB
{
    /* Base fields */
    UINT8 jmpBoot[3];
    UINT8 OEMName[8];
    UINT16 BytsPerSec;
    UINT8 SecPerClus;
    UINT16 ResvdSecCnt;
    UINT8 NumFATs;
    UINT16 RootEntCnt;
    UINT16 TotSec16;
    UINT8 Media;
    UINT16 FATSz16;
    UINT16 SecPerTrk;
    UINT16 NumHeads;
    UINT32 HiddSec;
    UINT32 TotSec32;

    /* FAT32 extended fields (FAT12 and FAT16 not supported) */
    union 
    {
        VFATBPB12 s12;
        VFATBPB16 s16;
        VFATBPB32 s32;
    }
    __attribute__((packed))
    u;
    UINT8 padding[422];
}
__attribute__((packed))
VFATBPB;

void DumpBPB(const VFATBPB* p);

/* Only used for FAT32: located at second sector (512 bytes) */
typedef struct _VFATFSInfo
{
    UINT32 LeadSig; /* 0x52 0x52 0x61 0x41 ("RRaA") */
    UINT8 Reserved1[480];
    UINT32 StrucSig; /* 0x72 0x72 0x41 0x61 ("rrAa") */
    UINT32 Free_Count;
    UINT32 Nxt_Free;
    UINT8 Reserved2[12];
    UINT32 TrailSig; /* 0xAA550000 */
}
__attribute__((packed))
VFATFSInfo;

#define ATTR_READ_ONLY 0x01 
#define ATTR_HIDDEN 0x02 
#define ATTR_SYSTEM 0x04 
#define ATTR_VOLUME_ID 0x08 
#define ATTR_DIRECTORY 0x10 
#define ATTR_ARCHIVE 0x20 
#define ATTR_LONG_NAME (ATTR_READ_ONLY|ATTR_HIDDEN|ATTR_SYSTEM|ATTR_VOLUME_ID)

#define ATTR_LONG_NAME_MASK (ATTR_READ_ONLY|ATTR_HIDDEN|ATTR_SYSTEM|\
    ATTR_VOLUME_ID|ATTR_DIRECTORY|ATTR_ARCHIVE)

/* Short directory entry (size=32) */
typedef struct _VFATDirectoryEntry
{
    UINT8 name[11];
    UINT8 attr;
    UINT8 res;
    UINT8 crtTimeTenth;
    UINT16 crtTime;
    UINT16 crtDate;
    UINT16 lstAccDate;
    UINT16 fstClusHI;
    UINT16 wrtTime;
    UINT16 wrtDate;
    UINT16 fstClusLO;
    UINT32 fileSize;
}
__attribute__((packed))
VFATDirectoryEntry;

void VFATDumpDirectoryEntry(
    const VFATDirectoryEntry* ent);

/* Long directory entry (size=32) */
typedef struct _VFATLongDirectoryEntry
{
    UINT8 ord;
    UINT16 name1[5];
    UINT8 attr;
    UINT8 type;
    UINT8 chksum;
    UINT16 name2[6];
    UINT16 fstClusLO;
    UINT16 name3[2];
}
__attribute__((packed))
VFATLongDirectoryEntry;

void DumpLongDirectoryEntry(
    const VFATLongDirectoryEntry* ent);

typedef enum _VFATType
{
    FAT12 = 12,
    FAT16 = 16,
    FAT32 = 32
}
VFATType;

typedef struct _VFAT
{
    VFATBPB bpb; /* at sector 0 (0 bytes) */
    VFATFSInfo fsi; /* at sector 1 (512 bytes offset) */

    /* Block device */
    Blkdev* dev;

    /* Sectors occupied by root directory */
    UINT32 RootDirSectors;

    /* Total data sectors */
    UINT32 DataSec;

    /* Total count of data clusters in data region */
    UINT32 CountOfClusters;

    /* First sector in the data region */
    UINT32 FirstDataSector;

    /* Root sector in the data region */
    UINT32 FirstRootDirSecNum;

    /* FAT size in sectors */
    UINT32 FATSz;

    /* Total sectors */
    UINT32 TotSec;

    /* VFAT Type */
    VFATType FATType;

    /* Cluster size in bytes */
    UINT32 ClusterSize;

    /* File Allocation Table */
    Buf fat;

    /* Root directory */
    Buf rootdir;

    /* Cluster number of the root directory (0 if not FAT32) */
    UINT32 rootdirClustno;
}
VFAT;

int VFATInit(
    Blkdev* dev, 
    VFAT** vfat);

int VFATStatFile(
    const VFAT* vfat, 
    const char* path,
    VFATDirectoryEntry* entry);

int VFATGetFile(
    VFAT* vfat,
    const char* path,
    void** data,
    UINTN* size);

int VFATPutFile(
    VFAT* vfat,
    const char* path,
    const void* data,
    UINTN size);

int VFATMkdir(
    VFAT* vfat,
    const char* path);

void VFATRelease(VFAT* vfat);

int VFATDump(
    const VFAT* vfat);

int VFATDir(
    VFAT* vfat,
    const char* path,
    StrArr* paths);

#endif /* _vfat_h */

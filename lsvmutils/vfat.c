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
#include "vfat.h"
#include "strings.h"
#include "print.h"
#include "guid.h"
#include "alloc.h"
#include "dump.h"
#include "buf.h"
#include "print.h"
#include "chksum.h"

#define TRACE PRINTF("TRACE: %a(%d)\n", __FILE__, __LINE__)

#if 1
# define GOTO(LABEL) goto LABEL
#else
# define GOTO(LABEL) \
    do \
    { \
        PRINTF("GOTO: %a(%d)\n", __FILE__, __LINE__); \
        goto LABEL; \
    } \
    while (0)
#endif

VFATType GetFATType(const VFATBPB* bpb)
{
    UINT32 TotSec;
    UINT32 FATSz;
    UINT32 RootDirSectors;
    UINT32 CountOfClusters;
    UINT32 DataSec;

    if (bpb->FATSz16 != 0)
        FATSz = bpb->FATSz16;
    else
        FATSz = bpb->u.s32.FATSz32;

    if (bpb->TotSec16 != 0)
        TotSec = bpb->TotSec16;
    else
        TotSec = bpb->TotSec32;

    RootDirSectors = ((bpb->RootEntCnt * 32) + (bpb->BytsPerSec - 1)) / 
        bpb->BytsPerSec;

    DataSec = TotSec - 
        (bpb->ResvdSecCnt + (bpb->NumFATs * FATSz) + RootDirSectors);

    CountOfClusters = DataSec / bpb->SecPerClus;

    if (CountOfClusters < 4085)
        return FAT12;
    else if (CountOfClusters < 65525)
        return FAT16;
    else
        return FAT32;
}

static __inline BOOLEAN __DumpCHAR16Str(const CHAR16* str, UINTN len)
{
    UINTN i;

    for (i = 0; i < len; i++)
    {
        if (str[i] == '\0')
            return TRUE;

        PRINTF("%c", (char)str[i]);
    }

    return FALSE;
}

static __inline UINT8 _VFATChkSum(const UINT8 name[11])
{
    UINTN i;
    UINT8 r = 0;

    for (i = 0; i < 11; i++)
        r = ((r & 1) ? 0x80 : 0) + (r >> 1) + name[i];

    return r;
}

void DumpBPB(const VFATBPB* p)
{
    PRINTF("=== VFATBPB (%u bytes)\n", (int)sizeof(VFATBPB));
    PRINTF("jmpBoot[]=%02X%02X%02X\n", 
        p->jmpBoot[0], p->jmpBoot[1], p->jmpBoot[2]);
    PRINTF("OEMName=%.*a\n", 8, p->OEMName);
    PRINTF("BytsPerSec=%u\n", p->BytsPerSec);
    PRINTF("SecPerClus=%u\n", p->SecPerClus);
    PRINTF("ResvdSecCnt=%u\n", p->ResvdSecCnt);
    PRINTF("NumFATs=%u\n", p->NumFATs);
    PRINTF("RootEntCnt=%u\n", p->RootEntCnt);
    PRINTF("TotSec16=%u\n", p->TotSec16);
    PRINTF("Media=%u\n", p->Media);
    PRINTF("FATSz16=%u\n", p->FATSz16);
    PRINTF("SecPerTrk=%u\n", p->SecPerTrk);
    PRINTF("NumHeads=%u\n", p->NumHeads);
    PRINTF("HiddSec=%u\n", p->HiddSec);
    PRINTF("TotSec32=%u\n", p->TotSec32);

    if (GetFATType(p) == FAT12)
    {
        PRINTF("NumPHDrive=%u\n", p->u.s12.NumPHDrive);
        PRINTF("Reserved0=%u\n", p->u.s12.Reserved0);
        PRINTF("BootSig=%u\n", p->u.s12.BootSig);
        PRINTF("NumSerial=%u\n", p->u.s12.NumSerial);
        PRINTF("VolLab=%.*a\n", 11, p->u.s12.VolLab);
        PRINTF("FilSysType[]=%.*a\n", 8, p->u.s12.FilSysType);
    }
    else if (GetFATType(p) == FAT16)
    {
        PRINTF("NumPHDrive=%u\n", p->u.s16.NumPHDrive);
        PRINTF("Reserved0=%u\n", p->u.s16.Reserved0);
        PRINTF("BootSig=%u\n", p->u.s16.BootSig);
        PRINTF("NumSerial=%u\n", p->u.s16.NumSerial);
        PRINTF("VolLab=%.*a\n", 11, p->u.s16.VolLab);
        PRINTF("FilSysType[]=%.*a\n", 8, p->u.s16.FilSysType);
    }
    if (GetFATType(p) == FAT32)
    {
        PRINTF("FATSz32=%u\n", p->u.s32.FATSz32);
        PRINTF("ExtFlags=%u\n", p->u.s32.ExtFlags);
        PRINTF("FSVer=%u\n", p->u.s32.FSVer);
        PRINTF("RootClus=%u\n", p->u.s32.RootClus);
        PRINTF("FSInfo=%u\n", p->u.s32.FSInfo);
        PRINTF("BkBootSec=%u\n", p->u.s32.BkBootSec);
        PRINTF("DrvNum=%u\n", p->u.s32.DrvNum);
        PRINTF("Reserved1=%u\n", p->u.s32.Reserved1);
        PRINTF("BootSig=%u\n", p->u.s32.BootSig);
        PRINTF("VolID=%u\n", p->u.s32.VolID);
        PRINTF("VolLab=%.*a\n", 11, p->u.s32.VolLab);
        PRINTF("FilSysType[]=%.*a\n", 8, p->u.s32.FilSysType);
    }
}

void DumpFSInfo(const VFATFSInfo* p)
{
    PRINTF("=== VFATFSInfo (%u bytes)\n", (int)sizeof(VFATFSInfo));
    PRINTF("LeadSig=%08X\n", p->LeadSig);
    PRINTF("StrucSig=%u\n", p->StrucSig);
    PRINTF("Free_Count=%u\n", p->Free_Count);
    PRINTF("Nxt_Free=%u\n", p->Nxt_Free);
    PRINTF("TrailSig=%08X\n", p->TrailSig);
}

void VFATDumpDirectoryEntry(const VFATDirectoryEntry* ent)
{
    PRINTF("=== VFATDirectoryEntry (%u bytes)\n", 
        (int)sizeof(VFATDirectoryEntry));
    PRINTF("name={%.*a}\n", 11, ent->name);
    PRINTF("attr=%u\n", ent->attr);
    PRINTF("res=%u\n", ent->res);
    PRINTF("crtTimeTenth=%u\n", ent->crtTimeTenth);
    PRINTF("crtTime=%u\n", ent->crtTime);
    PRINTF("crtDate=%u\n", ent->crtDate);
    PRINTF("lstAccDate=%u\n", ent->lstAccDate);
    PRINTF("fstClusHI=%u\n", ent->fstClusHI);
    PRINTF("wrtTime=%u\n", ent->wrtTime);
    PRINTF("wrtDate=%u\n", ent->wrtDate);
    PRINTF("fstClusLO=%u\n", ent->fstClusLO);
    PRINTF("fileSize=%u\n", ent->fileSize);

    if (ent->attr & ATTR_READ_ONLY)
        PRINTF("ATTR_READ_ONLY{%u}\n", ATTR_READ_ONLY);
    if (ent->attr & ATTR_HIDDEN)
        PRINTF("ATTR_HIDDEN{%u}\n", ATTR_HIDDEN);
    if (ent->attr & ATTR_SYSTEM)
        PRINTF("ATTR_SYSTEM{%u}\n", ATTR_SYSTEM);
    if (ent->attr & ATTR_VOLUME_ID)
        PRINTF("ATTR_VOLUME_ID{%u}\n", ATTR_VOLUME_ID);
    if (ent->attr & ATTR_DIRECTORY)
        PRINTF("ATTR_DIRECTORY{%u}\n", ATTR_DIRECTORY);
    if (ent->attr & ATTR_ARCHIVE)
        PRINTF("ATTR_ARCHIVE{%u}\n", ATTR_ARCHIVE);
}

void DumpLongDirectoryEntry(const VFATLongDirectoryEntry* ent)
{
    BOOLEAN eos = FALSE;

    PRINTF("=== VFATLongDirectoryEntry (%u bytes)\n",
        (int)sizeof(VFATLongDirectoryEntry));

    PRINTF("ord=%u (%u)\n", ent->ord, (ent->ord & ~0x40));

    PRINTF0("name1={");
    eos = __DumpCHAR16Str(ent->name1, ARRSIZE(ent->name1));
    PRINTF0("}\n");

    PRINTF("attr=%u\n", ent->attr);
    PRINTF("type=%u\n", ent->type);
    PRINTF("chksum=%u\n", ent->chksum);

    PRINTF0("name2={");
    if (!eos)
        eos = __DumpCHAR16Str(ent->name2, ARRSIZE(ent->name2));
    PRINTF0("}\n");

    PRINTF("fstClusLO=%u\n", ent->fstClusLO);

    PRINTF0("name3={");
    if (!eos)
        eos = __DumpCHAR16Str(ent->name3, ARRSIZE(ent->name3));
    PRINTF0("}\n");
}

static __inline UINT32 _Max(UINT32 x, UINT32 y)
{
    return x > y ? x : y;
}

static __inline UINT32 _FirstSectorOfCluster(const VFAT* vfat, UINT32 n)
{
    return ((n - 2) * vfat->bpb.SecPerClus) + vfat->FirstDataSector;
}

static __inline UINT32 _GetFATOffset(const VFAT* vfat, UINT32 clustno)
{
    UINT32 offset = 0;
    
    if (vfat->FATType == FAT12)
        offset = clustno + (clustno / 2);
    else if (vfat->FATType == FAT16)
        offset = clustno * 2;
    else if (vfat->FATType == FAT32)
        offset = clustno * 4;

    return offset;
}

static __inline UINT32 GetFATEntry(const VFAT* vfat, UINT32 clustno)
{
    UINT32 offset = _GetFATOffset(vfat, clustno);
    const UINT8* p = (const UINT8*)vfat->fat.data + offset;

    switch (vfat->FATType)
    {
        case FAT12:
        {
            UINT16 x = *((const UINT16*)p);

            if (clustno & 0x0001)
                x >>= 4;
            else
                x &= 0x0FFF;

            return x;
        }
        case FAT16:
        {
            return *((const UINT16*)p);
        }
        case FAT32:
        {
            return *((const UINT32*)p) & 0x0FFFFFFF;
        }
    }

    /* Unreachable */
    return 0;
}

static void SetFATEntry(VFAT* vfat, UINT32 clustno, UINT32 x)
{
    UINT32 offset = _GetFATOffset(vfat, clustno);
    UINT8* p = (UINT8*)vfat->fat.data + offset;

    switch (vfat->FATType)
    {
        case FAT12:
        {
            UINT16 r = *((UINT16*)p);


            if (clustno & 0x0001)
            {
                x <<= 4;
                r &= 0x000F;
            }
            else
            {
                x &= 0x0FFF;
                r &= 0xF000;
            }

            *((UINT16*)p) = r | x;
            break;
        }
        case FAT16:
        {
            *((UINT16*)p) = x;
            break;
        }
        case FAT32:
        {
            /* Preserve top four bits of origninal entry */
            UINT32 r = *((UINT32*)p);
            *((UINT32*)p) = (r & 0xF0000000) | (x & 0x0FFFFFFF);
            break;
        }
    }
}

static __inline UINT32 GetEOC(const VFAT* vfat)
{
    switch (vfat->FATType)
    {
        case FAT12:
            return 0x0FF8;
        case FAT16:
            return 0x0FF8;
        case FAT32:
            return 0x0FFFFFF8;
    }

    /* Unreachable! */
    return 0;
}

static __inline BOOLEAN IsEOF(const VFAT* vfat, UINT32 x)
{
    if (vfat->FATType == FAT12)
    {
        if (x >= 0x0FF8)
            return TRUE;
    }
    else if (vfat->FATType == FAT16)
    {
        if (x >= 0xFFF8)
            return TRUE;
    }
    else if (vfat->FATType == FAT32)
    {
        if (x >= 0x0FFFFFF8)
            return TRUE;
    }

    return FALSE;
}

int ReadCluster(
    const VFAT* vfat, 
    UINT32 clustno,
    Buf* buf)
{
    int rc = -1;
    UINT32 sectno = _FirstSectorOfCluster(vfat, clustno);

    if (BufReserve(buf, buf->size + vfat->ClusterSize) != 0)
        GOTO(done);

    if (BlkdevRead(
        vfat->dev, 
        sectno, 
        (UINT8*)buf->data + buf->size,
        vfat->ClusterSize) != 0)
    {
        GOTO(done);
    }

    buf->size += vfat->ClusterSize;

    rc = 0;

done:
    return rc;
}

int ReadClusters(
    const VFAT* vfat, 
    UINT32 clustno,
    Buf* buf)
{
    int rc = -1;

    while (clustno && !IsEOF(vfat, clustno))
    {
        /* Read this cluster */
        if (ReadCluster(vfat, clustno, buf) != 0)
            GOTO(done);

        /* Advance to the next cluster */
        clustno = GetFATEntry(vfat, clustno);
    }

    rc = 0;

done:
    return rc;
}

int WriteCluster(
    const VFAT* vfat, 
    UINT32 clustno,
    const void* data)
{
    int rc = -1;
    UINT32 sectno = _FirstSectorOfCluster(vfat, clustno);

    if (BlkdevWrite(
        vfat->dev, 
        sectno,
        data,
        vfat->ClusterSize) != 0)
    {
        GOTO(done);
    }

    rc = 0;

done:
    return rc;
}

int WriteClusters(
    const VFAT* vfat, 
    UINT32 clustno,
    const void* data,
    UINTN size)
{
    int rc = -1;
    const UINT8* ptr = (const UINT8*)data;
    const UINT8* end = ptr + size;
    void* cluster = NULL;

    while (clustno && !IsEOF(vfat, clustno))
    {
        /* If not enough data for an entire cluster */
        if (end - ptr < vfat->ClusterSize)
        {
            if (!(cluster = Calloc(1, vfat->ClusterSize)))
                goto done;

            Memcpy(cluster, ptr, end - ptr);

            if (WriteCluster(vfat, clustno, cluster) != 0)
                GOTO(done);

            ptr = end;
            break;
        }
        else
        {
            if (WriteCluster(vfat, clustno, ptr) != 0)
                GOTO(done);

            ptr += vfat->ClusterSize;
        }

        /* Advance to the next cluster */
        clustno = GetFATEntry(vfat, clustno);
    }

    if (ptr != end)
        GOTO(done);

    rc = 0;

done:

    if (cluster)
        Free(cluster);

    return rc;
}

/* "XYZ.DAT" becomes "XYZ     DAT" (not zero-terminated) */
int InternShortname(char buf[12], const char* shortname)
{
    int rc = -1;
    const char *dot;
    UINTN i;

    if ((dot = Strchr(shortname, '.')))
    {
        UINTN n1 = dot - shortname;
        UINTN n2 = Strlen(dot + 1);

        /* 8.3-format: "ABCDEFGH.IJK" */
        if (n1 > 8 || n2 > 3)
            GOTO(done);

        Memcpy(buf, shortname, n1);

        /* Pad name out to 8 characters */
        for (i = n1; i < 8; i++)
            buf[i] = ' ';

        /* Append extension if any */
        if (n2)
            Memcpy(buf + 8, dot + 1, n2);

        /* Pad name out to 11 characters */
        for (i = 8 + n2; i < 11; i++)
            buf[i] = ' ';
    }
    else
    {
        UINTN n = Strlen(shortname);
        UINTN i;

        if (n > 11)
            GOTO(done);

        Memcpy(buf, shortname, n);

        for (i = n; i < 11; i++)
            buf[i] = ' ';
    }

    /* Convert to upper case */
    for (i = 0; i < 11; i++)
        buf[i] = Toupper(buf[i]);

    /* Null terminate */
    buf[11] = '\0';

    rc = 0;

done:
    return rc;
}

void ExternShortname(char buf[13], const char shortname[11])
{
    UINTN n = 0;
    UINTN i;
    BOOLEAN more = FALSE;

    /* Copy over up to 8-byte filename */
    for (i = 0; i < 8; i++)
    {
        if (shortname[i] != ' ')
            buf[n++] = shortname[i];
    }

    /* If more non-space characters */
    for (i = 8; i < 11; i++)
    {
        if (shortname[i] != ' ')
            more = TRUE;
    }

    /* Copy over up to 3-byte extension */
    if (more)
    {
        buf[n++] = '.';

        for (i = 8; i < 11; i++)
        {
            if (shortname[i] != ' ')
                buf[n++] = shortname[i];
        }
    }

    /* Null terminate */
    buf[n] = '\0';
}

int FindDirectoryEntry(
    const VFAT* vfat, 
    const char* shortname,
    const void* directoryData,
    UINTN directorySize,
    VFATDirectoryEntry* entry)
{
    int rc = -1;
    const VFATDirectoryEntry* de = (const VFATDirectoryEntry*)directoryData;
    BOOLEAN found = FALSE;
    char name[12];

    if (!vfat || !shortname || !directoryData || !directoryData || !entry)
        GOTO(done);

    Memset(entry, 0, sizeof(VFATDirectoryEntry));

    /* Fix up the name */
    if (Strcmp(shortname, ".") == 0)
        Strlcpy(name, ".          ", sizeof(name));
    else if (Strcmp(shortname, "..") == 0)
        Strlcpy(name, "..         ", sizeof(name));
    else if (InternShortname(name, shortname) != 0)
        GOTO(done);

    for (; de->name[0]; de++)
    {
        /* If exhausted data */
        if ((void*)de >= (directoryData + directorySize))
            GOTO(done);

        if (de->name[0] != 0xE5)
        {
            if (de->attr != ATTR_LONG_NAME)
            {
                if (Memcmp(de->name, name, 11) == 0)
                {
                    Memcpy(entry, de, sizeof(VFATDirectoryEntry));
                    found = TRUE;
                    break;
                }
            }
        }
    }

    if (found)
        rc = 0;

done:
    return rc;
}

int VFATStatFile(
    const VFAT* vfat, 
    const char* path,
    VFATDirectoryEntry* entry)
{
    int rc = -1;
    char buf[VFAT_PATH_SIZE];
    const char* elements[32];
    const UINTN NELEMENTS = ARRSIZE(elements);
    UINT8 nelements = 0;
    UINT32 clustno = 0;
    UINTN i;
    BOOLEAN found = FALSE;

    /* Check parameters */
    if (!vfat || !path || !entry)
        GOTO(done);

    /* Reject non-absolute paths */
    if (path[0] != '/')
        GOTO(done);

    /* Split the path into its individual elements */
    {
        char* p;
        char* save = NULL;

        /* Inject root directory */
        elements[nelements++] = "/";

        Strlcpy(buf, path, sizeof(buf));

        /* For each element of the path */
        for (p = Strtok(buf, "/", &save); p; p = Strtok(NULL, "/", &save))
        {
            if (nelements == NELEMENTS)
                GOTO(done);

            elements[nelements++] = p;
        }
    }

    /* Load each cluster along the path until we find it */
    for (i = 0; i < nelements; i++)
    {
        VFATDirectoryEntry ent;

        if (Strcmp(elements[i], "/") == 0)
            continue;

        /* Search the root directory */
        if (i == 1)
        {
            /* Search the root directory */
            if (FindDirectoryEntry(
                vfat,
                elements[i],
                vfat->rootdir.data,
                vfat->rootdir.size,
                &ent) != 0)
            {
                GOTO(done);
            }

            /* Convert cluster number to 32-bit */
            clustno = ((UINT32)ent.fstClusHI << 16) | (UINT32)ent.fstClusLO;
        }
        else
        {
            Buf buf = BUF_INITIALIZER;

            /* Read all the clusters for this directory file */
            if (ReadClusters(vfat, clustno, &buf) != 0)
                GOTO(done);

            if (FindDirectoryEntry(
                vfat,
                elements[i],
                buf.data,
                buf.size,
                &ent) != 0)
            {
                BufRelease(&buf);
                GOTO(done);
            }

            /* Convert cluster number to 32-bit */
            clustno = ((UINT32)ent.fstClusHI << 16) | (UINT32)ent.fstClusLO;

            BufRelease(&buf);
        }

        /* If this was the final component, then succeess! */
        if (i + 1 == nelements)
        {
            Memcpy(entry, &ent, sizeof(VFATDirectoryEntry));
            found = TRUE;
        }
    }

    if (found)
        rc = 0;

done:
    return rc;
}

int LoadFileFromDirectoryEntry(
    const VFAT* vfat, 
    VFATDirectoryEntry* ent,
    Buf* buf)
{
    int rc = -1;
    UINT32 clustno;

    /* Check parameters */
    if (!vfat || !ent || !buf)
        GOTO(done);

    /* Clear the buffer */
    BufClear(buf);

    /* Form the 32-bit cluster number */
    clustno = ((UINT32)ent->fstClusHI << 16) | (UINT32)ent->fstClusLO;

    /* For each cluster */
    while (clustno && !IsEOF(vfat, clustno))
    {
        /* Read this cluster */
        if (ReadCluster(vfat, clustno, buf) != 0)
            GOTO(done);

        /* Advance to the next cluster */
        clustno = GetFATEntry(vfat, clustno);
    }

    /* If not able to read enough bytes */
    if (buf->size < ent->fileSize)
        GOTO(done);

    /* Size down to file size */
    if (!(ent->attr & ATTR_DIRECTORY))
        buf->size = ent->fileSize;

    rc = 0;

done:
    return rc;
}

void DumpLayout(const VFAT* vfat)
{
    UINT32 sectno = 0;
    UINT32 i;

    PRINTF0("=== Sector layout:\n");

    PRINTF("Reserved: [offset=%u:count=%u] (includes BPB and FSI)\n", 
        sectno, vfat->bpb.ResvdSecCnt);
    sectno += vfat->bpb.ResvdSecCnt;

    for (i = 0; i < vfat->bpb.NumFATs; i++)
    {
        PRINTF("FAT%u: [offset=%u:count=%u]\n", i, sectno, vfat->FATSz);
        sectno += vfat->FATSz;
    }

    PRINTF("RootDirSectors: [offset=%u:count=%u]\n", 
        sectno, vfat->RootDirSectors);
    sectno += vfat->RootDirSectors;

    PRINTF("DataSectors: [offset=%u:count=%u]\n", sectno, vfat->DataSec);
    sectno += vfat->DataSec;

    PRINTF("End: [offset=%u]\n", sectno);
}

int VFATDump(const VFAT* vfat)
{
    int rc = -1;

    if (!vfat)
        GOTO(done);

    DumpBPB(&vfat->bpb);
    DumpFSInfo(&vfat->fsi);

    PRINTF("RootDirSectors{%u}\n", vfat->RootDirSectors);
    PRINTF("FATSz{%u}\n", vfat->FATSz);
    PRINTF("TotSec{%u}\n", vfat->TotSec);
    PRINTF("DataSec{%u}\n", vfat->DataSec);
    PRINTF("CountOfClusters{%u}\n", vfat->CountOfClusters);
    PRINTF("FirstDataSector{%u}\n", vfat->FirstDataSector);
    PRINTF("FirstRootDirSecNum{%u}\n", vfat->FirstRootDirSecNum);
    PRINTF("Type{%u}\n", vfat->FATType);

    DumpLayout(vfat);

    rc = 0;

done:
    return rc;
}

static int _LoadDir(
    VFAT* vfat,
    const char* path,
    void** data,
    UINTN* size,
    UINT32* clustno)
{
    int rc = -1;
    Buf buf = BUF_INITIALIZER;

    /* Check parameters */
    if (!vfat || !path || !data || !size || !clustno)
        GOTO(done);

    /* Initialize output buffer */
    *data = NULL;
    *size = 0;

    /* Handle root path separately */
    if (Strcmp(path, "/") == 0)
    {
        BufAppend(&buf, vfat->rootdir.data, vfat->rootdir.size);
        *clustno = vfat->rootdirClustno;
    }
    else
    {
        VFATDirectoryEntry entry;

        /* Stat the file */
        if (VFATStatFile(vfat, path, &entry) != 0)
            GOTO(done);

        /* Be sure file is a directory */
        if (!(entry.attr & ATTR_DIRECTORY))
            GOTO(done);

        /* Load directory file into memory */
        if (LoadFileFromDirectoryEntry(vfat, &entry, &buf) != 0)
            GOTO(done);

        *clustno = ((UINT32)entry.fstClusHI << 16) | (UINT32)entry.fstClusLO;
    }

    *data = buf.data;
    *size = buf.size;
    rc = 0;

done:

    if (rc != 0)
        BufRelease(&buf);

    return rc;
}

static int _AllocateFATChain(
    VFAT* vfat,
    UINTN numClusters, /* number of clusters in FAT chain */
    UINT32* clustno) /* head cluster number of FAT chain */
{
    int rc = -1;
    UINT32 i;
    BufU32 buf = BUF_U32_INITIALIZER;

    /* ATTN-C: rework to not use dynamic memory */

    /* Check parameters */
    if (!clustno)
        GOTO(done);

    *clustno = 0;

    /* Handle zero-sized chain */
    if (numClusters == 0)
    {
        *clustno = 0;
        rc = 0;
        GOTO(done);
    }

    /* Find N free entries (where N=numClusters) */
    for (i = 2; i < vfat->CountOfClusters + 2 && buf.size < numClusters; i++)
    {
        UINT32 c = GetFATEntry(vfat, i);

        /* If available */
        if (c == 0)
            BufU32Append(&buf, &i, 1);
    }

    /* If not enough free clusters found */
    if (buf.size != numClusters)
        GOTO(done);

    /* Reserve the entries */
    {
        /* Set the head of the chain */
        *clustno = buf.data[0];

        /* Set subsequent entries */
        for (i = 0; i < buf.size; i++)
        {
            if (i + 1 == buf.size)
                SetFATEntry(vfat, buf.data[i], GetEOC(vfat));
            else
                SetFATEntry(vfat, buf.data[i], buf.data[i+1]);
        }
    }

    rc = 0;

done:

    BufU32Release(&buf);
    return rc;
}

int VFATInit(
    Blkdev* dev, 
    VFAT** vfatOut)
{
    int rc = -1;
    VFAT* vfat = NULL;

    if (!dev || !vfatOut)
        GOTO(done);

    /* Allocate the vfat object */
    if (!(vfat = Calloc(1, sizeof(VFAT))))
        GOTO(done);

    /* Read the BPB */
    if (BlkdevRead(dev, 0, &vfat->bpb, sizeof(vfat->bpb)) != 0)
        GOTO(done);

    /* Check signature of BPB */
    {
        const UINT8* p = vfat->bpb.jmpBoot;

        if (!(p[0] == 0xEB && p[2] == 0x90) && !(p[0] == 0xE9))
            GOTO(done);
    }

    /* See if there is an FSI (FAT32 only) */
    if (GetFATType(&vfat->bpb) == FAT32)
    {
        /* Read the BPB */
        if (BlkdevRead(
            dev, 
            vfat->bpb.u.s32.FSInfo, 
            &vfat->fsi, 
            sizeof(vfat->fsi)) != 0)
        {
            GOTO(done);
        }

        /* Check signatures of FSI */
        if (vfat->fsi.LeadSig != 0x41615252 || vfat->fsi.TrailSig != 0xAA550000)
            GOTO(done);
    }
    
    /* Set the block device */
    vfat->dev = dev;

    /* Reject sector sizes that are not 512 */
    if (vfat->bpb.BytsPerSec != VFAT_SECTOR_SIZE)
        GOTO(done);

    /* Precompute some useful values */
    {
        /* Compute VFAT.FATSz */
        if (vfat->bpb.FATSz16 != 0)
            vfat->FATSz = vfat->bpb.FATSz16;
        else
            vfat->FATSz = vfat->bpb.u.s32.FATSz32;

        /* Compute VFAT.TotSec */
        if (vfat->bpb.TotSec16 != 0)
            vfat->TotSec = vfat->bpb.TotSec16;
        else
            vfat->TotSec = vfat->bpb.TotSec32;

        /* Compute VFAT.RootDirSectors */
        vfat->RootDirSectors = 
            ((vfat->bpb.RootEntCnt * 32) + (vfat->bpb.BytsPerSec - 1)) / 
            vfat->bpb.BytsPerSec;

        /* Compute VFAT.FirstRootDirSecNum */
        vfat->FirstRootDirSecNum = vfat->bpb.ResvdSecCnt + 
            (vfat->bpb.NumFATs * vfat->FATSz);

        /* Compute VFAT.FirstDataSector */
        vfat->FirstDataSector = vfat->bpb.ResvdSecCnt + 
            (vfat->bpb.NumFATs * vfat->FATSz) + vfat->RootDirSectors;

        /* Compute VFAT.DataSec */
        vfat->DataSec = vfat->TotSec - 
            (vfat->bpb.ResvdSecCnt + (vfat->bpb.NumFATs * vfat->FATSz) + 
            vfat->RootDirSectors);

        /* Compute VFAT.CountOfClusters */
        vfat->CountOfClusters = vfat->DataSec / vfat->bpb.SecPerClus;

        /* Compute cluster size in bytes */
        vfat->ClusterSize = vfat->bpb.SecPerClus * vfat->bpb.BytsPerSec;
    }

    /* Determine Type of FAT */
    if (vfat->CountOfClusters < 4085)
        vfat->FATType = FAT12;
    else if (vfat->CountOfClusters < 65525)
        vfat->FATType = FAT16;
    else
        vfat->FATType = FAT32;

    /* Load one of the FATs into memory */
    {
        UINTN nbytes = vfat->FATSz * vfat->bpb.BytsPerSec;

        if (BufReserve(&vfat->fat, nbytes) != 0)
            GOTO(done);

        if (BlkdevRead(
            vfat->dev, 
            vfat->bpb.ResvdSecCnt, 
            vfat->fat.data, 
            nbytes) != 0)
        {
            GOTO(done);
        }

        vfat->fat.size += nbytes;

        /* If more than one FAT, read the second one in an compare */
        if (vfat->bpb.NumFATs > 1)
        {
            Buf buf = BUF_INITIALIZER;

            if (BufReserve(&buf, nbytes) != 0)
                GOTO(done);

            if (BlkdevRead(
                vfat->dev, 
                vfat->bpb.ResvdSecCnt + vfat->FATSz, 
                buf.data,
                nbytes) != 0)
            {
                BufRelease(&buf);
                GOTO(done);
            }

            buf.size += nbytes;

            if (buf.size != vfat->fat.size ||
                Memcmp(buf.data, vfat->fat.data, buf.size) != 0)
            {
                BufRelease(&buf);
                GOTO(done);
            }

            BufRelease(&buf);
        }

#if 0
        HexDump(vfat->fat.data, vfat->fat.size);
#endif
    }

    /* Load the root directory into memory */
    if (vfat->FATType == FAT32)
    {
        /* Read the root directory file into memory */
        if (ReadClusters(vfat, vfat->bpb.u.s32.RootClus, &vfat->rootdir) != 0)
            GOTO(done);

        vfat->rootdirClustno = vfat->bpb.u.s32.RootClus;
    }
    else
    {
        UINTN nbytes = vfat->RootDirSectors * vfat->bpb.BytsPerSec;

        if (BufReserve(&vfat->rootdir, nbytes) != 0)
            GOTO(done);

        if (BlkdevRead(
            vfat->dev, 
            vfat->FirstRootDirSecNum, 
            vfat->rootdir.data, 
            nbytes) != 0)
        {
            GOTO(done);
        }

        vfat->rootdir.size += nbytes;
        vfat->rootdirClustno = 0;
    }

    *vfatOut = vfat;
    rc = 0;

done:

    if (rc != 0)
    {
        if (vfat)
        {
            /* Don't let VFATRelease() free the caller's device */
            vfat->dev = NULL;
            VFATRelease(vfat);
        }
    }

    return rc;
}

void VFATRelease(VFAT* vfat)
{
    if (!vfat)
        return;

    if (vfat->dev)
        vfat->dev->Close(vfat->dev);

    BufRelease(&vfat->fat);
    BufRelease(&vfat->rootdir);
    Free(vfat);
}

int VFATGetFile(
    VFAT* vfat,
    const char* path,
    void** data,
    UINTN* size)
{
    int rc = -1;
    VFATDirectoryEntry ent;
    Buf buf = BUF_INITIALIZER;

    /* Check parameters */
    if (!vfat || !data || !size)
        GOTO(done);

    *data = NULL;
    *size = 0;

    if (VFATStatFile(vfat, path, &ent) != 0)
        GOTO(done);

    if (LoadFileFromDirectoryEntry(vfat, &ent, &buf) != 0)
        GOTO(done);

    *data = buf.data;
    *size = buf.size;

    rc = 0;

done:

    if (rc != 0)
        BufRelease(&buf);

    return rc;
}

static char* _GetDirName(
    char buf[VFAT_PATH_SIZE],
    const char* path)
{
    char* slash;

    Strlcpy(buf, path, VFAT_PATH_SIZE);

    if (!(slash = Strrchr(buf, '/')))
        return NULL;

    if (slash == buf)
        buf[1] = '\0';
    else
        buf[slash - buf] = '\0';

    return buf;
}

static char* _GetBaseName(
    char buf[VFAT_PATH_SIZE],
    const char* path)
{
    char* slash;

    if (!(slash = Strrchr(path, '/')))
        return NULL;

    if (slash[1] == '\0')
        return NULL;

    Strlcpy(buf, slash + 1, VFAT_PATH_SIZE);

    return buf;
}

static int _FlushFAT(
    VFAT* vfat)
{
    int rc = -1;
    UINTN i;

    /* Check parameters */
    if (!vfat)
        GOTO(done);

    /* Update each copy of the FAT on disk */
    for (i = 0; i < vfat->bpb.NumFATs; i++)
    {
        /* Write FAT to disk */
        if (BlkdevWrite(
            vfat->dev,
            vfat->bpb.ResvdSecCnt + (i * vfat->FATSz), 
            vfat->fat.data, 
            vfat->fat.size) != 0)
        {
            GOTO(done);
        }
    }

    rc = 0;

done:
    return rc;
}

static int _FlushFile(
    VFAT* vfat,
    const void* data,
    UINTN size,
    UINT32 clustno)
{
    int rc = -1;

    /* Check parameters */
    if (!vfat || !data)
        GOTO(done);

    /* If no data to flush */
    if (size == 0)
    {
        rc = 0;
        GOTO(done);
    }

    /* If flushing root directory on a non-FAT32 file system */
    if (clustno == 0)
    {
        UINT32 rootDirSize = vfat->RootDirSectors * vfat->bpb.BytsPerSec;

        if (rootDirSize != size)
            GOTO(done);

        if (BlkdevWrite(
            vfat->dev, 
            vfat->FirstRootDirSecNum, 
            data,
            size) != 0)
        {
            GOTO(done);
        }
    }
    else
    {
        if (WriteClusters(vfat, clustno, data, size) != 0)
            GOTO(done);
    }

    rc = 0;

done:
    return rc;
}

int _PutFile(
    VFAT* vfat,
    BOOLEAN isDir,
    const char* path,
    void* data,
    UINTN size)
{
    int rc = -1;
    VFATDirectoryEntry ent;
    char dirname[VFAT_PATH_SIZE];
    char basename[VFAT_PATH_SIZE];
    void* dirData = NULL;
    UINTN dirSize;
    UINT32 dirClustno;
    UINT32 clustno = 0;
    const VFATDirectoryEntry dot =
    {
        { '.', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' }, /* name */
        ATTR_ARCHIVE | ATTR_DIRECTORY, /* attr */
        0, /* res */
        0, /* crtTimeTenth */
        1709, /* crtTime */
        18706, /* crtDate */
        18706, /* lstAccDate */
        0, /* fstClusHI */
        22131, /* wrtTime */
        18706, /* wrtDate */
        0, /* fstClusLO */
        0, /* fileSize */
    };
    const VFATDirectoryEntry dotdot =
    {
        { '.', '.', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' }, /* name */
        ATTR_ARCHIVE | ATTR_DIRECTORY, /* attr */
        0, /* res */
        0, /* crtTimeTenth */
        1709, /* crtTime */
        18706, /* crtDate */
        18706, /* lstAccDate */
        0, /* fstClusHI */
        22131, /* wrtTime */
        18706, /* wrtDate */
        0, /* fstClusLO */
        0, /* fileSize */
    };

    /* Check parameters */
    if (!vfat || !path || !data)
        GOTO(done);

    /* If a directory, then copy "." and ".." entries */
    if (isDir)
    {
        Memset(data, 0, size);
        Memcpy(data, &dot, sizeof(dot));
        Memcpy((UINT8*)data + sizeof(dot), &dotdot, sizeof(dotdot));
    }

    /* Fail if file already exists */
    if (VFATStatFile(vfat, path, &ent) == 0)
        GOTO(done);

    /* Reject non-absolute paths */
    if (path[0] != '/')
        GOTO(done);

    /* Get the base name from the path (final component) */
    if (_GetBaseName(basename, path) == NULL)
        GOTO(done);

    /* Get the directory name from the path (excludes final component) */
    if (_GetDirName(dirname, path) == NULL)
        GOTO(done);

    /* Load directory into memory */
    if (_LoadDir(vfat, dirname, &dirData, &dirSize, &dirClustno) != 0)
        GOTO(done);

    /* If creating directory file, update the parent directory cluster */
    if (isDir)
    {
        VFATDirectoryEntry* p = (VFATDirectoryEntry*)data;
        p[1].fstClusHI = (dirClustno & 0xFFFF0000) << 16;
        p[1].fstClusLO = (dirClustno & 0x0000FFFF);
    }

    /* Inject directory entry */
    {
        /* ATTN-C: times and dates are hardcoded! */
        VFATDirectoryEntry entry =
        {
            { '\0' }, /* name */
            0, /* attr */
            0, /* res */
            100, /* crtTimeTenth */
            1884, /* crtTime */
            18706, /* crtDate */
            18706, /* lstAccDate */
            0, /* fstClusHI */
            1884, /* wrtTime */
            18706, /* wrtDate */
            0, /* fstClusLO */
            0, /* fileSize */
        };

        /* Set VFATDirectoryEntry.attr */
        {
            entry.attr = ATTR_ARCHIVE;

            if (isDir)
                entry.attr |= ATTR_DIRECTORY;
        }

        /* Set VFATDirectoryEntry.name */
        {
            char shortname[12];
            InternShortname(shortname, basename);
            Memcpy(entry.name, shortname, sizeof(entry.name));
        }

        /* Set VFATDirectoryEntry.fileSize */
        if (!isDir)
            entry.fileSize = size;

        /* Allocate FAT chain for this new file (update in-memory FAT copy) */
        {
            UINTN r = (size + vfat->ClusterSize - 1) / vfat->ClusterSize;

            if (_AllocateFATChain(vfat, r, &clustno) != 0)
                GOTO(done);
        }

        /* If creating directory file, update the directory cluster */
        if (isDir)
        {
            VFATDirectoryEntry* p = (VFATDirectoryEntry*)data;
            p[0].fstClusHI = (clustno & 0xFFFF0000) << 16;
            p[0].fstClusLO = (clustno & 0x0000FFFF);
        }

        /* Update directory entry cluster number */
        entry.fstClusHI = (clustno & 0xFFFF0000) << 16;
        entry.fstClusLO = (clustno & 0x0000FFFF);

        /* Inject entry into directory file memory */
        {
            VFATDirectoryEntry* p = (VFATDirectoryEntry*)dirData;

            for (; p->name[0]; p++)
            {
                /* Use this entry if deleted */
                if (p->name[0] == 0xE5)
                    break;

                /* Skip long-name entries */
                if (p->attr == ATTR_LONG_NAME)
                    continue;
            }

            /* ATTN-B: handle overflow of directory files */
            /* If no more room for a directory entry */
            if ((void*)(p + 1) >= (dirData + dirSize))
                GOTO(done);

            /* Update the entry */
            Memcpy(p, &entry, sizeof(*p));
        }
    }

    /* Flush file to disk */
    if (_FlushFile(vfat, data, size, clustno) != 0)
        GOTO(done);

    /* If this is the root directory, then refresh the cache */
    if (dirClustno == vfat->rootdirClustno)
    {
        if (vfat->rootdir.size != dirSize)
            goto done;

        Memcpy(vfat->rootdir.data, dirData, dirSize);
    }

    /* Flush directory to disk */
    if (_FlushFile(vfat, dirData, dirSize, dirClustno) != 0)
        GOTO(done);

    /* Flush FAT to disk */
    if (_FlushFAT(vfat))
        GOTO(done);

    rc = 0;

done:

    if (dirData)
        Free(dirData);

    return rc;
}

int VFATMkdir(
    VFAT* vfat,
    const char* path)
{
    int rc = -1;
    void* data = NULL;
    UINTN size;

    if (!vfat || !path)
        goto done;
    
    size = vfat->ClusterSize;

    if (!(data = Calloc(1, vfat->ClusterSize)))
        goto done;

    if (_PutFile(vfat, TRUE, path, data, size) != 0)
        goto done;

    rc = 0;

done:

    if (data)
        Free(data);

    return rc;
}

int VFATPutFile(
    VFAT* vfat,
    const char* path,
    const void* data,
    UINTN size)
{
    return _PutFile(vfat, FALSE, path, (void*)data, size);
}

int VFATDir(
    VFAT* vfat,
    const char* path,
    StrArr* paths)
{
    int rc = -1;
    void* data = NULL;
    UINTN size = 0;
    UINT32 clustno = 0;

    /* Check parameters */
    if (!vfat || !path || !paths)
        GOTO(done);

    /* Load the directory file into memory */
    if (_LoadDir(vfat, path, &data, &size, &clustno) != 0)
        GOTO(done);

    /* Scan this directory and form list of files */
    {
        VFATDirectoryEntry* p = (VFATDirectoryEntry*)data;

        /* ATTN: be sure not to overrun size */

        for (; p->name[0]; p++)
        {
            /* Skip deleted entries */
            if (p->name[0] == 0xE5)
                continue;

            /* Skip long-name entries */
            if (p->attr == ATTR_LONG_NAME)
                continue;

            /* Format name */
            {
                char buf[13];
                ExternShortname(buf, (const char*)p->name);
                StrArrAppend(paths, buf);
            }
        }
    }

    rc = 0;

done:

    if (data)
        Free(data);

    return rc;
}

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
#ifndef _lsvmutils_cpio_h
#define _lsvmutils_cpio_h

#include "config.h"
#include <lsvmutils/eficommon.h>
#include <lsvmutils/strings.h>
#include <lsvmutils/strarr.h>

#define CPIO_MODE_IFMT 00170000
#define CPIO_MODE_IFSOCK 0140000
#define CPIO_MODE_IFLNK 0120000
#define CPIO_MODE_IFREG 0100000
#define CPIO_MODE_IFBLK 0060000
#define CPIO_MODE_IFDIR 0040000
#define CPIO_MODE_IFCHR 0020000
#define CPIO_MODE_IFIFO 0010000
#define CPIO_MODE_ISUID 0004000
#define CPIO_MODE_ISGID 0002000
#define CPIO_MODE_ISVTX 0001000

#define CPIO_MODE_IRWXU 00700
#define CPIO_MODE_IRUSR 00400
#define CPIO_MODE_IWUSR 00200
#define CPIO_MODE_IXUSR 00100

#define CPIO_MODE_IRWXG 00070
#define CPIO_MODE_IRGRP 00040
#define CPIO_MODE_IWGRP 00020
#define CPIO_MODE_IXGRP 00010

#define CPIO_MODE_IRWXO 00007
#define CPIO_MODE_IROTH 00004
#define CPIO_MODE_IWOTH 00002
#define CPIO_MODE_IXOTH 00001

typedef struct _CPIOHeader 
{
   char magic[6];
   char ino[8];
   char mode[8];
   char uid[8];
   char gid[8];
   char nlink[8];
   char mtime[8];
   char filesize[8];
   char devmajor[8];
   char devminor[8];
   char rdevmajor[8];
   char rdevminor[8];
   char namesize[8];
   char check[8];
}
CPIOHeader;

int CPIOMatchMagicNumber(
    const char* s);

int CPIOTest(
    const void* data, 
    UINTN size);

int CPIOCheckEntry(
    const CPIOHeader* header,
    const void* cpioEnd);

int CPIOGetEntrySize(
    const CPIOHeader* header);

int CPIONextHeader(
    const CPIOHeader** header,
    const void* cpioEnd);

/* Creates a new (empty) CPIO archive */
int CPIONew(
    void** data,
    UINTN* size);

int CPIOAddFile(
    const void* cpioDataIn,
    UINTN cpioSizeIn,
    const char* path,
    const void* fileData, /* null for directories */
    UINTN fileDataSize, /* ignored for directories */
    unsigned int mode,
    void** cpioDataOut,
    UINTN* cpioSizeOut);

int CPIORemoveFile(
    const void* cpioDataIn,
    UINTN cpioSizeIn,
    const char* path,
    void** cpioDataOut,
    UINTN* cpioSizeOut);

int CPIOAddFile(
    const void* cpioDataIn,
    UINTN cpioSizeIn,
    const char* path,
    const void* fileData,
    UINTN fileSize,
    unsigned int mode,
    void** cpioDataOut,
    UINTN* cpioSizeOut);

int CPIOGetFile(
    const void* cpioData,
    UINTN cpioSize,
    const char* src,
    void** destData,
    UINTN* destSize);

int CPIODumpFile(
    const void* cpioData,
    UINTN cpioSize);

int CPIOCheckFile(
    const void* cpioData,
    UINTN cpioSize,
    const char* path);

BOOLEAN CPIOIsFile(
    const void* cpioData,
    UINTN cpioSize,
    const char* path);

BOOLEAN CPIOIsDir(
    const void* cpioData,
    UINTN cpioSize,
    const char* path);

#define CPIO_MAX_SUBFILES 8

int CPIOSplitFile(
    const void* cpioData,
    UINTN cpioSize,
    const void* subfilesData[CPIO_MAX_SUBFILES],
    UINTN subfilesSize[CPIO_MAX_SUBFILES],
    UINTN* numSubfiles);

int CPIOFindFilesByBaseName(
    const void* cpioData,
    UINTN cpioSize,
    const char* baseName,
    StrArr* arr);

#endif /* _lsvmutils_cpio_h */

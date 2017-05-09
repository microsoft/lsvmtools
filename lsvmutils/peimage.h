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
#ifndef _peimage_h
#define _peimage_h

#include "config.h"
#include "eficommon.h"
#include <lsvmutils/sha.h>

#define IMAGE_FILE_MACHINE_X64 0x8664

#define EFI_IMAGE_DOS_SIGNATURE ((UINT16)'M' | (UINT16)'Z' << 8)

#define EFI_IMAGE_NT_SIGNATURE ((UINT32)'P' | (UINT32)'E' << 8)

#define EFI_IMAGE_SIZEOF_FILE_HEADER 20

#define EFI_IMAGE_FILE_RELOCS_STRIPPED 0x00000001

#define EFI_IMAGE_NT_OPTIONAL_HDR64_MAGIC 0x20b

#define EFI_IMAGE_SIZEOF_NT_OPTIONAL64_HEADER sizeof (EFI_IMAGE_NT_HEADERS64)

#define EFI_IMAGE_SIZEOF_SHORT_NAME 8

#define EFI_IMAGE_SIZEOF_SECTION_HEADER 40

#define EFI_IMAGE_SCN_MEM_DISCARDABLE 0x02000000

/* Indices into directory entry table */
#define EFI_IMAGE_DIRECTORY_ENTRY_EXPORT 0
#define EFI_IMAGE_DIRECTORY_ENTRY_IMPORT 1
#define EFI_IMAGE_DIRECTORY_ENTRY_RESOURCE 2
#define EFI_IMAGE_DIRECTORY_ENTRY_EXCEPTION 3
#define EFI_IMAGE_DIRECTORY_ENTRY_SECURITY 4
#define EFI_IMAGE_DIRECTORY_ENTRY_BASERELOC 5
#define EFI_IMAGE_DIRECTORY_ENTRY_DEBUG 6
#define EFI_IMAGE_DIRECTORY_ENTRY_COPYRIGHT 7
#define EFI_IMAGE_DIRECTORY_ENTRY_GLOBALPTR 8
#define EFI_IMAGE_DIRECTORY_ENTRY_TLS 9
#define EFI_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG 10
#define EFI_IMAGE_NUMBER_OF_DIRECTORY_ENTRIES 16

/* relocation types */
#define EFI_IMAGE_REL_BASED_ABSOLUTE 0
#define EFI_IMAGE_REL_BASED_HIGH 1
#define EFI_IMAGE_REL_BASED_LOW 2
#define EFI_IMAGE_REL_BASED_HIGHLOW 3
#define EFI_IMAGE_REL_BASED_HIGHADJ 4
#define EFI_IMAGE_REL_BASED_MIPS_JMPADDR 5
#define EFI_IMAGE_REL_BASED_ARM_MOV32A 5
#define EFI_IMAGE_REL_BASED_ARM_MOV32T 7
#define EFI_IMAGE_REL_BASED_IA64_IMM64 9
#define EFI_IMAGE_REL_BASED_MIPS_JMPADDR16 9
#define EFI_IMAGE_REL_BASED_DIR64 10

/* Indices into directory entry table */
#define EFI_TE_IMAGE_DIRECTORY_ENTRY_BASERELOC  0
#define EFI_TE_IMAGE_DIRECTORY_ENTRY_DEBUG      1

typedef struct _EFI_IMAGE_DOS_HEADER
{
    UINT16 e_magic;
    UINT16 e_cblp;
    UINT16 e_cp;
    UINT16 e_crlc;
    UINT16 e_cparhdr;
    UINT16 e_minalloc;
    UINT16 e_maxalloc;
    UINT16 e_ss;
    UINT16 e_sp;
    UINT16 e_csum;
    UINT16 e_ip;
    UINT16 e_cs;
    UINT16 e_lfarlc;
    UINT16 e_ovno;
    UINT16 e_res[4];
    UINT16 e_oemid;
    UINT16 e_oeminfo;
    UINT16 e_res2[10];
    UINT32 e_lfanew;
}
EFI_IMAGE_DOS_HEADER;

typedef struct _EFI_IMAGE_FILE_HEADER
{
    UINT16 Machine;
    UINT16 NumberOfSections;
    UINT32 TimeDateStamp;
    UINT32 PointerToSymbolTable;
    UINT32 NumberOfSymbols;
    UINT16 SizeOfOptionalHeader;
    UINT16 Characteristics;
}
EFI_IMAGE_FILE_HEADER;

typedef struct _EFI_IMAGE_DATA_DIRECTORY
{
    UINT32 VirtualAddress;
    UINT32 Size;
}
EFI_IMAGE_DATA_DIRECTORY;

typedef struct _EFI_IMAGE_OPTIONAL_HEADER64
{
    UINT16 Magic;
    UINT8 MajorLinkerVersion;
    UINT8 MinorLinkerVersion;
    UINT32 SizeOfCode;
    UINT32 SizeOfInitializedData;
    UINT32 SizeOfUninitializedData;
    UINT32 AddressOfEntryPoint;
    UINT32 BaseOfCode;
    UINT64 ImageBase;
    UINT32 SectionAlignment;
    UINT32 FileAlignment;
    UINT16 MajorOperatingSystemVersion;
    UINT16 MinorOperatingSystemVersion;
    UINT16 MajorImageVersion;
    UINT16 MinorImageVersion;
    UINT16 MajorSubsystemVersion;
    UINT16 MinorSubsystemVersion;
    UINT32 Win32VersionValue;
    UINT32 SizeOfImage;
    UINT32 SizeOfHeaders;
    UINT32 CheckSum;
    UINT16 Subsystem;
    UINT16 DllCharacteristics;
    UINT64 SizeOfStackReserve;
    UINT64 SizeOfStackCommit;
    UINT64 SizeOfHeapReserve;
    UINT64 SizeOfHeapCommit;
    UINT32 LoaderFlags;
    UINT32 NumberOfRvaAndSizes;
    EFI_IMAGE_DATA_DIRECTORY DataDirectory[
        EFI_IMAGE_NUMBER_OF_DIRECTORY_ENTRIES];
}
EFI_IMAGE_OPTIONAL_HEADER64;

typedef struct _EFI_IMAGE_NT_HEADERS64
{
    UINT32 Signature;
    EFI_IMAGE_FILE_HEADER FileHeader;
    EFI_IMAGE_OPTIONAL_HEADER64 OptionalHeader;
}
EFI_IMAGE_NT_HEADERS64;

typedef struct _EFI_IMAGE_SECTION_HEADER
{
    UINT8 Name[EFI_IMAGE_SIZEOF_SHORT_NAME];
    union
    {
        UINT32 PhysicalAddress;
        UINT32 VirtualSize;
    }
    Misc;
    UINT32 VirtualAddress;
    UINT32 SizeOfRawData;
    UINT32 PointerToRawData;
    UINT32 PointerToRelocations;
    UINT32 PointerToLinenumbers;
    UINT16 NumberOfRelocations;
    UINT16 NumberOfLinenumbers;
    UINT32 Characteristics;
}
EFI_IMAGE_SECTION_HEADER;

typedef struct _EFI_IMAGE_BASE_RELOCATION
{
    UINT32 VirtualAddress;
    UINT32 SizeOfBlock;
}
EFI_IMAGE_BASE_RELOCATION;

typedef struct _EFI_TE_IMAGE_HEADER
{
    UINT16 Signature;
    UINT16 Machine;
    UINT8 NumberOfSections;
    UINT8 Subsystem;
    UINT16 StrippedSize;
    UINT32 AddressOfEntryPoint;
    UINT32 BaseOfCode;
    UINT64 ImageBase;
    EFI_IMAGE_DATA_DIRECTORY DataDirectory[2];
}
EFI_TE_IMAGE_HEADER;

typedef struct _Region
{
    const void* data;
    UINTN size;
}
Region;

typedef struct _Image
{
    const void* imageData;
    UINTN imageSize;
    const void* signatureData;
    UINTN signatureSize;
    Region* regions;
    UINTN nregions;
}
Image;

int ParseImage(
    Image* image,
    IN const void* imageData,
    IN UINTN imageSize);

int HashImage(
    const Image* image,
    OUT SHA1Hash* sha1,
    OUT SHA256Hash* sha256);

int ParseAndHashImage(
    const void* imageData,
    UINTN imageSize,
    OUT SHA1Hash* sha1,
    OUT SHA256Hash* sha256);

int DumpImage(
    IN const void* imageData,
    IN UINTN imageSize);

int CheckCert(
    const void* imageData,
    UINTN imageSize,
    const void* certData,
    UINTN certSize);

BOOLEAN IsEFIImage(
    IN const void* imageData,
    IN UINTN imageSize);

BOOLEAN IsKernelImage(
    IN const void* imageData,
    IN UINTN imageSize);

#endif /* _peimage_h */

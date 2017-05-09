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
#include <lsvmutils/eficommon.h>
#include <lsvmutils/peimage.h>
#include "image.h"
#include "console.h"
#include "strings.h"
#include <lsvmutils/alloc.h>
#include <lsvmutils/tpm2.h>
#include "trace.h"
#include "log.h"

#if !defined(__x86_64__)
# error "only X86 64-bit is supported"
#endif

/* Test whether object lies with region */
static int __ChkBounds(
    const void* region,
    UINTN regionSize,
    const void* object,
    UINTN objectSize)
{
    if ((UINT8*)object < (UINT8*)region)
        return -1;

    if ((UINT8*)object + objectSize > (UINT8*)region + regionSize)
        return -1;

    /* Success */
    return 0;
}

#define CHKBOUNDS(REGION, REGIONSIZE, OBJECT, OBJECTSIZE) \
    do \
    { \
        if (__ChkBounds(REGION, REGIONSIZE, OBJECT, OBJECTSIZE) != 0) \
        { \
            status = EFI_COMPROMISED_DATA; \
            GOTO(done); \
        } \
    } \
    while (0)

typedef struct _Headers
{
    /* A copy of the DOS header */
    EFI_IMAGE_DOS_HEADER dos;

    /* A copy of the FileHeader */
    EFI_IMAGE_FILE_HEADER fh;

    /* A copy of the OptionalHeader */
    EFI_IMAGE_OPTIONAL_HEADER64 oh;

    /* Points to the NT header */
    EFI_IMAGE_NT_HEADERS64 *ptr;

    /* Points to the base relocation directory entry */
    const EFI_IMAGE_DATA_DIRECTORY* relocDir;

    /* Points to the first section header */
    const EFI_IMAGE_SECTION_HEADER* firstSection;
}
Headers;

static EFI_STATUS _GetHeaders(
    const UINT8 *image, 
    UINT32 imageSize,
    Headers *headers)
{
    EFI_STATUS status = EFI_UNSUPPORTED;

    /* Check parameters */
    if (!image || !imageSize || !headers)
        GOTO(done);

    /* Resolve DOS header */
    {
        const EFI_IMAGE_DOS_HEADER* dos = (const EFI_IMAGE_DOS_HEADER*)image;

        CHKBOUNDS(image, imageSize, dos, sizeof(EFI_IMAGE_DOS_HEADER));

        /* Check the magic number in the DOS header */
        if (dos->e_magic != EFI_IMAGE_DOS_SIGNATURE)
            GOTO(done);

        /* Make a copy of the DOS header for later */
        Memcpy(&headers->dos, dos, sizeof(EFI_IMAGE_DOS_HEADER));
    }

    /* Check signature in TE heder */
    {
        const EFI_TE_IMAGE_HEADER* te =
            (const EFI_TE_IMAGE_HEADER*)(image + headers->dos.e_lfanew);

        CHKBOUNDS(image, imageSize, te, sizeof(EFI_TE_IMAGE_HEADER));

        if (te->Signature != EFI_IMAGE_NT_SIGNATURE) 
            GOTO(done);
    }

    /* Resolve NT header (follows the DOS header) */
    {
        const EFI_IMAGE_NT_HEADERS64* nt =
            (const EFI_IMAGE_NT_HEADERS64*)(image + headers->dos.e_lfanew);

        CHKBOUNDS(image, imageSize, nt, sizeof(EFI_IMAGE_NT_HEADERS64));

        /* Check the magic number of the NT header */
        if (nt->OptionalHeader.Magic != EFI_IMAGE_NT_OPTIONAL_HDR64_MAGIC)
            GOTO(done);

        /* Verify that the machine is X86-64-bit */
        if (nt->FileHeader.Machine != IMAGE_FILE_MACHINE_X64)
            GOTO(done);

        /* No way to relocate if relocations have been stripped */
        if (nt->FileHeader.Characteristics & EFI_IMAGE_FILE_RELOCS_STRIPPED) 
            GOTO(done);

        /* Make a copy of the file header */
        Memcpy(&headers->fh, &nt->FileHeader, sizeof(headers->fh));

        /* Make a copy of the optional header */
        Memcpy(&headers->oh, &nt->OptionalHeader, sizeof(headers->oh));

        /* Cache pointer to NT header */
        headers->ptr = (EFI_IMAGE_NT_HEADERS64*)nt;
    }

    /* Check the size of the image (allow for extra bytes at end of image) */
    if (imageSize < headers->oh.SizeOfImage)
        GOTO(done);

    /* If too many directory entries */
    if (headers->oh.NumberOfRvaAndSizes > EFI_IMAGE_NUMBER_OF_DIRECTORY_ENTRIES)
        GOTO(done);

    /* Check size of optional header (mandatory for executables) */
    if (headers->fh.SizeOfOptionalHeader != sizeof(EFI_IMAGE_OPTIONAL_HEADER64))
        GOTO(done);

    /* Check whether there's enough room left for section headers */
    {
        UINTN size = 
            headers->dos.e_lfanew + /* DOS header */
            sizeof(UINT32) + /* Signature */
            sizeof(EFI_IMAGE_FILE_HEADER) + /* FileHeader */
            sizeof(EFI_IMAGE_OPTIONAL_HEADER64) + /* OptionalHeader */
            (headers->fh.NumberOfSections * EFI_IMAGE_SIZEOF_SECTION_HEADER);
        UINTN align = headers->oh.FileAlignment;

        if (size > imageSize)
            GOTO(done);

        /* Align the size to the 'FileAlignment' */
        size = (size + align - 1) / align * align;

        if (size != headers->oh.SizeOfHeaders)
            GOTO(done);
    }

    /* Cache a pointer to the base relocation directory */
    {
        headers->relocDir = &headers->ptr->OptionalHeader.DataDirectory[
            EFI_IMAGE_DIRECTORY_ENTRY_BASERELOC];

        CHKBOUNDS(
            image,
            imageSize,
            image + headers->relocDir->VirtualAddress,
            headers->relocDir->Size);
    }

    /* Cache a pointer to first section header */
    {
        headers->firstSection = (EFI_IMAGE_SECTION_HEADER*)(
            (UINT8*)headers->ptr + 
            sizeof(UINT32) +
            sizeof(EFI_IMAGE_FILE_HEADER) +
            headers->fh.SizeOfOptionalHeader);

        CHKBOUNDS(
            image,
            imageSize,
            headers->firstSection,
            sizeof(EFI_IMAGE_FILE_HEADER));
    }
    
    /* Verify the offsets and sizes within the sections */
    {
        const EFI_IMAGE_SECTION_HEADER* section;
        int i;

        section = headers->firstSection;

        for (i = 0; i < headers->fh.NumberOfSections; i++, section++) 
        {
            UINT32 offset = section->PointerToRawData;
            UINT32 size = section->Misc.VirtualSize;

            if (size > section->SizeOfRawData)
                size = section->SizeOfRawData;

            CHKBOUNDS(image, imageSize, image + offset, size);
        }
    }

    status = EFI_SUCCESS;

done:
    return status;
}

static EFI_STATUS _ApplyRelocations(
    Headers *headers,
    const EFI_IMAGE_SECTION_HEADER *relocSection,
    const UINT8 *image, 
    UINT32 imageSize,
    UINT8 *newImage)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    const EFI_IMAGE_BASE_RELOCATION *p;
    const EFI_IMAGE_BASE_RELOCATION *pend;
    UINT64 offset;

    /* Check parmeters */
    if (!headers || !relocSection || !image || !newImage)
    {
        GOTO(done);
    }

    /* Set pointer to new memory where relocation will occurs */
    headers->ptr->OptionalHeader.ImageBase = (UINT64)(unsigned long)newImage;

    /* Set pointers to start and end of the relocations region */
    p = (EFI_IMAGE_BASE_RELOCATION*)(image + relocSection->PointerToRawData);
    pend = (EFI_IMAGE_BASE_RELOCATION*)(
        (UINT8*)p + relocSection->Misc.VirtualSize);

    CHKBOUNDS(image, imageSize, p, pend - p);

    /* Calculate offset to first byte of image (when loaded into memory). */
    if (!(offset = (UINTN)newImage - headers->oh.ImageBase))
    {
        status = EFI_SUCCESS;
        goto done;
    }

    /* For each base relocation object: [header] [block] */
    while (p < pend)
    {
        const UINT16* q;
        const UINT16* qend;
        UINT8* base;

        /* Set pointers to start and end of this block */
        q = (UINT16*)((UINT8*)p + sizeof (EFI_IMAGE_BASE_RELOCATION));
        qend = (UINT16*)((UINT8*)p + p->SizeOfBlock);

        CHKBOUNDS(image, imageSize, q, qend - q);

        /* Check whether block would really fit withing the relocation area */
        if (p->SizeOfBlock == 0 || p->SizeOfBlock > headers->relocDir->Size) 
            GOTO(done);

        /* Set pointer to area to be fixed up */
        base = newImage + p->VirtualAddress;

        /* For each word (UINT16) in the relocation block */
        for (; q < qend; q++) 
        {
            UINT16 addr;
            UINT16 tag;

            /* The address is contained in the lower 12 bits */
            addr = *q & 0x0FFF;

            /* The tag is contained in the upper 4 bits */
            tag = (*q & 0xF000) >> 12;

            UINT8* ptr = base + addr;

            switch (tag)
            {
                case EFI_IMAGE_REL_BASED_ABSOLUTE:
                {
                    break;
                }
                case EFI_IMAGE_REL_BASED_HIGH:
                {
                    UINT16* ptr16 = (UINT16*)ptr;
                    CHKBOUNDS(newImage, imageSize, ptr16, sizeof(UINT16));
                    *ptr16 = (UINT16)(*ptr16 + ((UINT16)((UINT32)offset>>16)));
                    break;
                }
                case EFI_IMAGE_REL_BASED_LOW:
                {
                    UINT16* ptr16 = (UINT16*)ptr;
                    CHKBOUNDS(newImage, imageSize, ptr16, sizeof(UINT16));
                    *ptr16 = (UINT16)(*ptr16 + (UINT16)offset);
                    break;
                }
                case EFI_IMAGE_REL_BASED_HIGHLOW:
                {
                    UINT32* ptr32 = (UINT32*)ptr;
                    CHKBOUNDS(newImage, imageSize, ptr32, sizeof(UINT32));
                    *ptr32 = *ptr32 + (UINT32)offset;
                    break;
                }
                case EFI_IMAGE_REL_BASED_DIR64:
                {
                    UINT64* ptr64 = (UINT64*)ptr;
                    CHKBOUNDS(newImage, imageSize, ptr64, sizeof(UINT64));
                    *ptr64 += (UINT64)offset;
                    break;
                }
                default:
                {
                    GOTO(done);
                }
            }
        }

        p = (const EFI_IMAGE_BASE_RELOCATION*)qend;
    }

    status = EFI_SUCCESS;

done:
    return status;
}

typedef EFI_STATUS (EFIAPI *EntryPoint)(
    EFI_HANDLE image_handle, 
    EFI_SYSTEM_TABLE *system_table);

static EFI_STATUS _FixupImage(
    const UINT8 *image, 
    unsigned int imageSize,
    EFI_LOADED_IMAGE *li,
    EntryPoint* entryPoint)
{
    EFI_STATUS status = EFI_UNSUPPORTED;
    Headers headers;
    UINT8 *newImage = NULL;
    UINT8* baseReloc;
    UINT32 baseRelocSize;
    const EFI_IMAGE_SECTION_HEADER *relocSection = NULL;
    const EFI_IMAGE_SECTION_HEADER *section = NULL;
    int i;

    /* Get and check the headers */
    if ((status = _GetHeaders(image, imageSize, &headers)) != EFI_SUCCESS)
    {
        GOTO(done);
    }

    /* SizeOfImage could be smaller due to extra data at end of image */
    imageSize = headers.oh.SizeOfImage;

    /* Allocate a newImage to hold the new image */
    if (!(newImage = Calloc(headers.oh.SizeOfImage, 1)))
    {
        GOTO(done);
    }

    /* Copy over the headers to the new image */
    CHKBOUNDS(image, imageSize, image, headers.oh.SizeOfHeaders);
    Memcpy(newImage, image, headers.oh.SizeOfHeaders);

    /* Assign pointer and size to relocation region */
    baseReloc = newImage + headers.relocDir->VirtualAddress;
    baseRelocSize = headers.relocDir->Size;

    CHKBOUNDS(
        newImage, 
        imageSize,
        newImage + headers.relocDir->VirtualAddress,
        headers.relocDir->Size);

    /* Get pointer to first section (already bounds checked) */
    section = headers.firstSection;

    /* For each section */
    for (i = 0; i < headers.fh.NumberOfSections; i++, section++) 
    {
        UINT8 *base = newImage + section->VirtualAddress;
        UINT32 baseSize = section->Misc.VirtualSize;
        const UINT8 RELOC_NAME[EFI_IMAGE_SIZEOF_SHORT_NAME] =
            { '.', 'r', 'e', 'l', 'o', 'c', '\0', '\0' };

        CHKBOUNDS(image, imageSize, section, sizeof(EFI_IMAGE_SECTION_HEADER));
        CHKBOUNDS(newImage, imageSize, base, baseSize);

        if (baseSize > section->SizeOfRawData)
            baseSize = section->SizeOfRawData;

        /* If this is a relocation section */
        if (Memcmp(section->Name, RELOC_NAME, sizeof(RELOC_NAME)) == 0)
        {
            /* Duplicate relocation section */
            if (relocSection) 
                GOTO(done);

            if (section->SizeOfRawData > 0 && section->Misc.VirtualSize > 0 &&
                baseReloc == base && baseRelocSize == baseSize) 
            {
                relocSection = section;
            }
        }

        /* Skip discardable sections */
        if (section->Characteristics & EFI_IMAGE_SCN_MEM_DISCARDABLE)
            continue;

        /* Copy the section to the new newImage */
        if (section->SizeOfRawData > 0)
        {
            CHKBOUNDS(
                image, 
                imageSize, 
                image + section->PointerToRawData, 
                baseSize);
            Memcpy(base, image + section->PointerToRawData, baseSize);
        }

        /* Zero out an residual memory */
        if (section->Misc.VirtualSize > baseSize)
        {
            CHKBOUNDS(
                newImage, 
                imageSize, 
                base + baseSize,
                section->Misc.VirtualSize - baseSize);
            Memset(base + baseSize, 0, section->Misc.VirtualSize - baseSize);
        }
    }

    /* If no base relocation directory entry */
    if (headers.oh.NumberOfRvaAndSizes <= EFI_IMAGE_DIRECTORY_ENTRY_BASERELOC)
        GOTO(done);

    /* If relocation entry found and non-zero in size, perform relocations */
    if (relocSection && headers.relocDir->Size > 0)
    {
        if ((status = _ApplyRelocations(
            &headers, 
            relocSection, 
            image, 
            imageSize,
            newImage)) != EFI_SUCCESS)
        {
            GOTO(done);
        }
    }

    /* Set the entry point */
    CHKBOUNDS(
        newImage, 
        imageSize, 
        newImage + headers.oh.AddressOfEntryPoint,
        sizeof(entryPoint));
    *entryPoint = (EntryPoint)(newImage + headers.oh.AddressOfEntryPoint);

    /* Set the image base and size */
    CHKBOUNDS(newImage, imageSize, newImage, headers.oh.SizeOfImage);
    li->ImageBase = newImage;
    li->ImageSize = headers.oh.SizeOfImage;

    /* Set the load options (none for now) */
    li->LoadOptions = NULL;
    li->LoadOptionsSize = 0;

    status = EFI_SUCCESS;

done:

    if (status != EFI_SUCCESS)
        Free(newImage);

    return status;
}

EFI_STATUS Exec(
    IN EFI_HANDLE parentImageHandle,
    IN EFI_SYSTEM_TABLE *systemTable,
    IN void* efiData,
    IN UINTN efiSize)
{
    EFI_STATUS status = EFI_SUCCESS;
    UINT8* image = (UINT8*)efiData;
    UINTN imageSize = efiSize;
    EFI_LOADED_IMAGE* loadedImage = NULL;
    EFI_LOADED_IMAGE loadedImageTmp;
    EntryPoint entryPoint = NULL;

    /* Obtain the LOADED_IMAGE_PROTOCOL */
    {
        EFI_GUID loadedImageProtocol = LOADED_IMAGE_PROTOCOL;

        if ((status = uefi_call_wrapper(
            BS->HandleProtocol, 
            3, 
            parentImageHandle,
            &loadedImageProtocol, 
            (void**)&loadedImage)) != EFI_SUCCESS)
        {
            goto done;
        }
    }

    /* Save the old loaded image */
    Memcpy(&loadedImageTmp, loadedImage, sizeof(EFI_LOADED_IMAGE));

    /* Relocate the image */
    if ((status = _FixupImage(
        image, 
        imageSize, 
        loadedImage, 
        &entryPoint)) != EFI_SUCCESS)
    {
        goto done;
    }

    /* Execute the program */
    if ((status = uefi_call_wrapper(
        entryPoint, 
        2, 
        parentImageHandle, 
        systemTable)) != EFI_SUCCESS)
    {
        goto done;
    }

    /* Restore the loaded image (probably not reachable) */
    Memcpy(loadedImage, &loadedImageTmp, sizeof(EFI_LOADED_IMAGE));

done:
    return status;
}

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
#include "efivfat.h"
#include <lsvmutils/vfat.h>
#include <lsvmutils/guid.h>
#include <lsvmutils/alloc.h>
#include <lsvmutils/vfat.h>
#include <lsvmutils/memblkdev.h>
#include <lsvmutils/strings.h>
#include <lsvmutils/grubcfg.h>
#include "globals.h"
#include "diskbio.h"
#include "measure.h"
#include "log.h"
#include "logging.h"
#include "paths.h"
#include "bootfs.h"

/* EFI VFAT embedded file system */
extern unsigned char efivfat[];
extern unsigned int efivfat_size;

static BOOLEAN _Contains(
    const char* value,
    const char* const* skiplist,
    UINTN skiplen)
{
    UINTN i;
    UINTN valLen = Strlen(value);

    for (i = 0; i < skiplen; i++)
    {
        if (Strlen(skiplist[i]) == valLen &&
            Strncmp(skiplist[i], value, valLen) == 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

static int _AddCwdGrubcfg(
    VFAT* vfat,
    const char* const* skiplist,
    UINTN skiplen,
    char pathSep)
{
    int rc = -1;
    CHAR16* dirName = NULL;
    char path[EXT2_PATH_MAX];
    char *pathPtr;
    char grubcfg[] = "/grub.cfg";

    if (!vfat || !skiplist)
    {
        LOGE(L"%a; bad parameter", __FUNCTION__);
        goto done;
    }

    dirName = StrDuplicate(globals.lsvmloadPath);
    if (dirName == NULL)
    {
        LOGE(L"%a: out of memory", __FUNCTION__);
        goto done;
    }

    if (Dirname(dirName) != 0)
    {
        LOGE(L"Unable to resolve dirname");
        goto done;
    }

    if (Wcslen(dirName) + ARRSIZE(grubcfg) > ARRSIZE(path))
    {
        LOGE(L"Path too large: %s", dirName);
        goto done;
    }

    StrWcslcpy(path, dirName, ARRSIZE(path));

    /* Replace the standard \\ seperator with the one used by VFAT */
    for (pathPtr = path; *pathPtr; pathPtr++)
    {
        if (*pathPtr == '\\')
        {
            *pathPtr = pathSep;
        }
    }

    /* Make all the parent directories before making the grub.cfg */
    for (pathPtr = path; *pathPtr; pathPtr++)
    {
        /* Skip first pathSep for abosolute paths */
        if (pathPtr == path)
        {
            continue;
        }


        /* Ignore non pathSep unless at the end to handle cases like /EFI/BOOT */
        if (*pathPtr != pathSep && *(pathPtr+1) != '\0')
        {
            continue;
        }

        if (*pathPtr == pathSep)
        {
            *pathPtr = '\0';
        }

        if (!_Contains(path, skiplist, skiplen))
        {
            if (VFATMkdir(vfat, path) != 0)
            {
                LOGE(L"Failed to make dir path: %a", path);
                goto done;
            }
        }
     
        if (*pathPtr == '\0')
        {
            *pathPtr = pathSep;
        }
    }
    
    /* Now make the grub.cfg path. */
    Strncat(path, ARRSIZE(path), grubcfg, ARRSIZE(grubcfg)-1);
    if (VFATPutFile(
        vfat, 
        path,
        globals.grubcfgData, 
        globals.grubcfgSize) != 0)
    {
        LOGE(L"VFATPutFile(): failed to create %a", path);
        goto done;
    }
    rc = 0;

done:
    if (dirName != NULL)
    {
        Free(dirName);
    }
    return rc;
}

int MapEFIVFAT(
    EFI_HANDLE imageHandle)
{
    int rc = -1;
    GPTEntry* entry = NULL;
    const int partitionNumber = 1;
    Blkdev* memdev = NULL;
    VFAT* vfat = NULL;

    /* Check parameters */
    if (!imageHandle || !globals.bootfs || !globals.efiVendorDir)
    {
        LOGE(L"%a: bad parameter", __FUNCTION__);
        goto done;
    }

    /* Load grub.cfg from the boot partition */
    if (!globals.grubcfgData)
    {
        const CHAR16 PATH1[] = L"/grub2/grub.cfg";
        const CHAR16 PATH2[] = L"/grub/grub.cfg";

        if (LoadFileFromBootFS(
            imageHandle,
            globals.tcg2Protocol,
            globals.bootfs,
            PATH1,
            &globals.grubcfgData,
            &globals.grubcfgSize) == EFI_SUCCESS)
        {
            LOGI(L"Loaded %s", Wcs(PATH1));
        }
        else if (LoadFileFromBootFS(
            imageHandle,
            globals.tcg2Protocol,
            globals.bootfs,
            PATH2,
            &globals.grubcfgData,
            &globals.grubcfgSize) == EFI_SUCCESS)
        {
            LOGI(L"Loaded %s", Wcs(PATH2));
        }
        else
        {
            LOGE(L"Failed to load grub.cfg");
            goto done;
        }
    }

    /* Apply patches to grub.cfg */
    GrubcfgPatch(globals.grubcfgData, globals.grubcfgSize);

    /* Create "/EFI/<VENDOR-EFI-DIR>/GRUB.CFG" file in embedded-VFAT parition */
    {
        /* Create a memory block device to be used by VFAT */
        if (!(memdev = BlkdevFromMemory((void*)efivfat, efivfat_size)))
        {
            LOGE(L"BlkdevFromMemory() failed");
            goto done;
        }

        /* Initialize the VFAT object */
        if (VFATInit(memdev, &vfat) != 0)
        {
            LOGE(L"VFATInit() failed");
            goto done;
        }

        /* Create the "EFI" directory */
        if (VFATMkdir(vfat, "/EFI") != 0)
        {
            LOGE(L"VFATMkdir(): failed to create /EFI");
            goto done;
        }

        /* Create the EFI vendor directory ("/EFI/UBUNTU") */
        {
            char path[EXT2_PATH_MAX];

            Strlcpy(path, "/EFI/", ARRSIZE(path));
            StrWcslcat(path, globals.efiVendorDir, ARRSIZE(path));

            if (VFATMkdir(vfat, path) != 0)
            {
                LOGE(L"VFATMkdir(): failed to create %a", Str(path));
                goto done;
            }
        }

        /* Create GRUB.CFG under vendor directory ("/EFI/UBUNTU/GRUB.CFG") */
        {
            char path[EXT2_PATH_MAX];

            Strlcpy(path, "/EFI/", ARRSIZE(path));
            StrWcslcat(path, globals.efiVendorDir, ARRSIZE(path));
            Strlcat(path, "/GRUB.CFG", ARRSIZE(path));

            if (VFATPutFile(
                vfat, 
                path,
                globals.grubcfgData, 
                globals.grubcfgSize) != 0)
            {
                LOGE(L"VFATPutFile(): failed to create %a", path);
                goto done;
            }
        }

        /* SUSE uses the current directory to find the grub.cfg. So, put there as well. */
        {
            char path[EXT2_PATH_MAX];
            const char* skip[2];

            skip[0] = "/EFI";
            Strlcpy(path, "/EFI/", ARRSIZE(path));
            StrWcslcat(path, globals.efiVendorDir, ARRSIZE(path));
            skip[1] = path;

            if (_AddCwdGrubcfg(vfat, skip, ARRSIZE(skip), '/') != 0)
            {
                LOGE(L"_AddCwdGrubcfg() failed");
                goto done;
            }
        }
	
    }

#if 0
    /* Log the hash of this memory */
    {
        SHA1Hash sha1;
        ComputeSHA1(efivfat, efivfat_size, &sha1);
        LogHash1(L"VFAT", &sha1);
    }
#endif

    /* ATTN: assuming EFI partition is /dev/sda1 (use detection) */
    if (GetGPTEntry(partitionNumber, &entry) != 0)
        goto done;

    /* Add a region to handle these blocks */
    if (AddRegion(
        REGION_ID_ESP,
        entry->startingLBA,
        entry->endingLBA,
        efivfat_size / GPT_BLOCK_SIZE,
        TRUE,
        (Block*)efivfat,
        NULL) != 0)
    {
        goto done;
    }

    /* Success! */
    rc = 0;

done:

    if (vfat)
        VFATRelease(vfat);
    else if (memdev)
        memdev->Close(memdev);

    return rc;
}

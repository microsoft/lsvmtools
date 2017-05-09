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
#include "initrd.h"
#include <lsvmutils/alloc.h>
#include <lsvmutils/initrd.h>
#include <lsvmutils/ext2.h>
#include <lsvmutils/grubcfg.h>
#include <time.h>
#include "globals.h"
#include "bootfs.h"
#include "measure.h"
#include "log.h"
#include "progress.h"

#define BENCH(COND)

int GetInitrdPath(
    char path[PATH_MAX])
{
    int rc = -1;
    char matched[PATH_MAX];
    char title[PATH_MAX];

    /* Check parameters */
    if (!path ||
        !globals.imageHandle || 
        !globals.tcg2Protocol ||
        !globals.bootfs)
    {
        LOGE(L"%a(): bad parametrer", Str(__FUNCTION__));
        goto done;
    }

    /* Attempt to open grub.cfg */
    if (!globals.grubcfgData)
    {
        const CHAR16 PATH1[] = L"/grub2/grub.cfg";
        const CHAR16 PATH2[] = L"/grub/grub.cfg";

        if (LoadFileFromBootFS(
            globals.imageHandle,
            globals.tcg2Protocol,
            globals.bootfs,
            PATH1,
            &globals.grubcfgData,
            &globals.grubcfgSize) == EFI_SUCCESS)
        {
            LOGI(L"Loaded %s", Wcs(PATH1));
        }
        else if (LoadFileFromBootFS(
            globals.imageHandle,
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

    /* Get the initrd path from grub.cfg */
    if (GRUBCfgFindInitrd(
        globals.grubcfgData, 
        globals.grubcfgSize, 
        matched,
        title,
        path) != 0)
    {
        LOGE(L"failed to get initrd paths from grub.cfg");
        goto done;
    }

    rc = 0;

done:

    return rc;
}

int PatchInitrd(
    EFI_HANDLE imageHandle,
    EFI_TCG2_PROTOCOL* tcg2Protocol,
    EXT2* bootfs,
    const char* initrdPath)
{
    int rc = -1;
    void* initrdData = NULL;
    UINTN initrdSize;
    void* newInitrdData = NULL;
    UINTN newInitrdSize;

    /* Reject bad parameters */
    if (!imageHandle || !tcg2Protocol || !bootfs || !initrdPath)
        goto done;

    /* Only perform patch if rootkey is valid */
    if (globals.rootkeyValid)
    {
        /* Avoid doing this if keys were not found */
        if (!globals.rootkeyFound || !globals.bootkeyFound)
        {
            LOGW(L"either bootkey or rootkey not found");
            goto done;
        }

        PutProgress(L"Pathching %a", Str(initrdPath));

        /* Inject keys into this initrd */
        {
            CHAR16 wcs[PATH_MAX];
            WcsStrlcpy(wcs, initrdPath, ARRSIZE(wcs));

            /* Try to load the file from the bootfs */
            if (LoadFileFromBootFS(
                imageHandle,
                tcg2Protocol,
                bootfs,
                wcs,
                &initrdData,
                &initrdSize) != EFI_SUCCESS)
            {
                LOGE(L"failed to load %s", Wcs(wcs));
                goto done;
            }
            else
            {
                LOGI(L"Loaded initrd: %s", Wcs(wcs));
            }

            {
                BENCH( posix_time_t t = posix_time(NULL); )

                /* Inject the keys into the bootfs and remove keyboard driver */
                if (InitrdInjectFiles(
                    initrdData,
                    initrdSize,
                    globals.bootkeyData,
                    globals.bootkeySize,
                    globals.rootkeyData,
                    globals.rootkeySize,
                    &newInitrdData,
                    &newInitrdSize) != 0)
                {
                    LOGE(L"failed to inject keys: %s", Wcs(wcs));
                    goto done;
                }

                BENCH( Print(L"inject: %ld\n", (long)(posix_time(NULL) - t)); Wait(); )
            }

            /* Replace original initrd with new one */
            {
                {
                    BENCH( posix_time_t t = posix_time(NULL); )

                    /* Remove original initrd */
                    if (EXT2Rm(bootfs, initrdPath) != EXT2_ERR_NONE)
                    {
                        LOGE(L"failed to remove %s", Wcs(wcs));
                        goto done;
                    }

                    BENCH( Print(L"remove: %ld\n", (long)(posix_time(NULL) - t)); Wait(); )
                }

                {
                    BENCH( posix_time_t t = posix_time(NULL); )

                    /* Create new initrd */
                    if (EXT2Put(
                        bootfs, 
                        newInitrdData, 
                        newInitrdSize, 
                        initrdPath,
                        EXT2_FILE_MODE_RW0_R00_R00) != EXT2_ERR_NONE)
                    {
                        LOGE(L"failed to rewrite %s", Wcs(wcs));
                        goto done;
                    }

                    BENCH( Print(L"rewrite: %ld\n", (long)(posix_time(NULL) - t)); Wait(); )
                }
            }

            LOGI(L"Injected keys into initrd: %s", Wcs(wcs));
        }
    }



    rc = 0;

done:

    if (initrdData)
        Free(initrdData);

    if (newInitrdData)
        Free(newInitrdData);

    return rc;
}

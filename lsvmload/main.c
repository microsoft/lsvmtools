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
#include <lsvmutils/strings.h>
#include <lsvmutils/print.h>
#include <lsvmutils/tpm2.h>
#include <lsvmutils/efifile.h>
#include <lsvmutils/buf.h>
#include <lsvmutils/alloc.h>
#include <lsvmutils/strings.h>
#include <lsvmutils/cacheblkdev.h>
#include <xz/lzmaextras.h>
#include <zlib/zlibextras.h>
#include <time.h>
#include "measure.h"
#include "paths.h"
#include "log.h"
#include "logging.h"
#include "loadconf.h"
#include "simpletext.h"
#include "globals.h"
#include "bootfs.h"
#include "rootfs.h"
#include "keys.h"
#include "kernel.h"
#include "shim.h"
#include "bootbio.h"
#include "console.h"
#include "devpath.h"
#include "initrd.h"
#include "specialize.h"
#include "progress.h"
#include "dbxupdate.h"

extern unsigned char g_logo[];
extern unsigned int g_logo_size;

extern const char timestamp[];
extern char __timestamp[];
extern char __version[];

void PrintSplashScreen(void)
{
    ClearScreen();
    SetColors(EFI_LIGHTGREEN, EFI_BLACK);
    Print(L"%a", Str((const char*)g_logo));
    SetColors(EFI_WHITE, EFI_BLACK);
    Print(L"Version: %a\n", Str(LSVMLOAD_VERSION));
    Print(L"Timestamp: %a\n\n", Str(timestamp));
    SetColors(EFI_LIGHTGRAY, EFI_BLACK);
}

void lsvmload_trace(const char* file, int line)
{
    LOGE(L"lsvmload_trace(): %a(%d)", file, line);
}

__attribute__((visibility("default")))
EFI_STATUS efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE *systemTable)
{
    EFI_STATUS status = EFI_SUCCESS;
    EFI_TCG2_PROTOCOL* tcg2Protocol = NULL;
    Error err;
    EXT2* bootfs = NULL;
    BOOLEAN unsealedBootkeyValid = FALSE;
    BOOLEAN haveTPM = FALSE;

    /* No-op to keep linker from removing these symbols */
    __version[Strlen(__version)] = '\0';
    __timestamp[Strlen(__timestamp)] = '\0';

    /* Set the global image handle */
    globals.imageHandle = imageHandle;
    globals.systemTable = systemTable;

    /* Initlize the EFI library */
    InitializeLib(imageHandle, systemTable);

    /* Set measured boot failure flag (innocent till proven guilty) */
    globals.measuredBootFailed = TRUE;

    /* Print the logo */
    PrintSplashScreen();

    /* Resolve file locations */
    if ((status = ResolvePaths(imageHandle, &err)) != EFI_SUCCESS)
    {
        LOGE(L"failed to resolve file locations: %s", err.buf);
        status = EFI_UNSUPPORTED;
        goto done;
    }

    /* Delete the log file */
    TruncLog();

    /* Print path of this image */
    {
        CHAR16 path[PATH_SIZE];
        LOGI(L"Started %s", DosToUnixPath(path, globals.lsvmloadPath));
    }

    /* Print the build timestamp */
    LOGI(L"Build timestamp: %a", Str(timestamp));

    /* Get the configuration options */
    if (LoadConf(globals.imageHandle, &err) != 0)
    {
        LOGE(L"failed to load %s: %s", Wcs(globals.lsvmconfPath), Wcs(err.buf));
        status = EFI_UNSUPPORTED;
        goto done;
    }

    /* Write the log level to the log */
    LOGI(L"LogLevel=%a", Str(LogLevelToStr(GetLogLevel())));

    /* Set the global TCG2 protocol pointer */
    if (!(tcg2Protocol = TCG2_GetProtocol()))
    {
        LOGE(L"Failed to get the TCG2 protocol");
        status = EFI_UNSUPPORTED;
        goto done;
    }

    /* Set the global TCG2 protocol pointers */
    globals.tcg2Protocol = tcg2Protocol;

    /* Log the event log */
    if (GetLogLevel() >= DEBUG)
    {
        LogPaths(); 
        LogEventLog(tcg2Protocol, imageHandle);
    }

    /* Check whether TPM chip is present */
    if ((status = TPM2X_CheckTPMPresentFlag(tcg2Protocol)) == EFI_SUCCESS)
    {
        LOGI(L"TPM chip detected");
        haveTPM = TRUE;
    }
    else
    {
        LOGE(L"TPM chip detect failed");
    }

    /* Log the PCRs */
    if (haveTPM && GetLogLevel() >= DEBUG)
        LogPCRs(tcg2Protocol, imageHandle, L"Initial PCR Values");

    /* Initialize TPM and other things */
    if ((status = Initialize(
        tcg2Protocol, 
        imageHandle, 
        haveTPM,
        &err)) != EFI_SUCCESS)
    {
        const CHAR16 MSG[] = L"measured boot failed: %s";
        LOGE(MSG);
        PrintErrorWait(MSG, Wcs(err.buf));
        status = EFI_UNSUPPORTED;
        goto done;
    }

    /* Measure the Linux scenario into PCR[11] */
    if (haveTPM && MeasureLinuxScenario(tcg2Protocol, imageHandle) != EFI_SUCCESS)
    {
        LOGE(L"failed to measure Linux scenario");
        goto done;
    }

    if (haveTPM && GetLogLevel() >= DEBUG)
        LogPCRs(tcg2Protocol, imageHandle, L"After PCR11 Values");

    /* Attempt to unseal the keys */
    if (haveTPM && UnsealKeys(imageHandle, tcg2Protocol) != EFI_SUCCESS)
    {
        LOGE(L"failed to unseal keys");
        /* Will ask for passphrase later */
    }

    /* Cap PCR[11] */
    if (haveTPM && CapPCR(imageHandle, tcg2Protocol, 11) != EFI_SUCCESS)
    {
        LOGE(L"failed to cap PCR[11]");
        goto done;
    }

    PutProgress(L"Checking boot partition");

    /* Try to unlock the boot parition with the unsealed bootkey */
    while (globals.bootkeyData)
    {
        /* Open the boot file system */
        if (!(bootfs = OpenBootFS(
            imageHandle,
            tcg2Protocol,
            globals.bootkeyData,
            globals.bootkeySize)) != EFI_SUCCESS)
        {
            LOGE(L"failed to open the boot parition");
            Free(globals.bootkeyData);
            globals.bootkeyData = NULL;
            globals.bootkeySize = 0;
            break;
        }

        unsealedBootkeyValid = TRUE;
        break;
    }

    PutProgress(L"Checking root partition");

    /* Test the passphrase for the root device */
    if (TestRootDevice(
        imageHandle,
        tcg2Protocol,
        globals.rootkeyData,
        globals.rootkeySize) == 0)
    {
        globals.rootkeyValid = TRUE;
        LOGI(L"root device key okay");
    }
    else
    {
        LOGE(L"bad root device key");
    }

    /* Ask for bootkey interactively */
    if (!unsealedBootkeyValid)
    {
        UINTN i;
        const UINTN N = 3;

        /* Ask for passphrase up to N times */
        for (i = 0; i < N; i++)
        {
            CHAR16 wcs[64];
            char str[64];
            UINTN len;

            /* Get the bootkey passphrase */
            if (GetInput(
                L"Enter boot partition passphrase: ", 
                FALSE, 
                wcs, 
                ARRSIZE(wcs)) != EFI_SUCCESS)
            {
                continue;
            }

            /* Convert to single character */
            StrWcslcpy(str, wcs, ARRSIZE(str));

            /* Save length of the string */
            len = Strlen(str);

            /* Open the boot file system */
            if (!(bootfs = OpenBootFS(
                imageHandle,
                tcg2Protocol,
                (const UINT8*)str,
                Strlen(str))) != EFI_SUCCESS)
            {
                continue;
            }

            /* Copy the passphrase into globals */
            {
                if (!(globals.bootkeyData = (UINT8*)Malloc(len + 1)))
                {
                    LOGE(L"out of memory");
                    goto done;
                }

                Memcpy(globals.bootkeyData, str, len + 1);
                globals.bootkeySize = len;
            }

            break;
        }

        if (!bootfs)
        {
            LOGE(L"failed to open the boot partition");
            goto done;
        }
    }

    /* Set global pointer to boot file system */
    globals.bootfs = bootfs;

    /* Load and decrypt and copy specialize file to boot partition */
    if (LoadDecryptCopySpecializeFile(
        imageHandle,
        globals.bootdev,
        globals.bootfs,
        globals.specializePath) != 0)
    {
        LOGW(L"Failed to load or decrypt specialization file");
    }

    /* Disable writes to boot device */
    if (globals.cachedev)
    {
        globals.cachedev->SetFlags(globals.cachedev, BLKDEV_ENABLE_CACHING);
    }

    /* If unsealed bootkey was able to unlock the boot partition */
    if (unsealedBootkeyValid)
    {
        char path[PATH_MAX];

        /* Apply DBX Update and reseal 'sealedkeys' */
        if (haveTPM)
        {
            BOOLEAN reboot;

            if (ApplyDBXUpdate(
                imageHandle, 
                tcg2Protocol, 
                bootfs, 
                &reboot) != 0)
            {
                LOGE(L"DBXUpdate(): failed");
                goto done;
            }

            if (reboot)
            {
                if (RT->ResetSystem(EfiResetWarm, 0, 0, NULL) != EFI_SUCCESS)
                {
                    LOGE(L"Reboot failed");
                    return 0;
                }
            }
        }

        /* Find initrd paths */
        if (GetInitrdPath(path) != 0)
        {
            LOGE(L"GetInitrdPath(): failed");
            goto done;
        }

        /* Inject bootkey and rootkey into initrd */
        if (PatchInitrd(imageHandle, tcg2Protocol, bootfs, path) != 0)
        {
            LOGE(L"PatchInitrd(): failed: %a", Str(path));
        }
    }

#if 0
    /* Print cachedev stats */
    if (globals.cachedev)
    {
        UINTN numBlocks;
        UINTN numChains;
        UINTN maxChains;
        UINTN longestChain;

        CacheBlkdevStats(
            globals.cachedev, 
            &numBlocks, 
            &numChains, 
            &maxChains, 
            &longestChain);

        Print(L"numBlocks: %ld\n", (long)numBlocks);
        Print(L"numChains: %ld\n", (long)numChains);
        Print(L"maxChains: %ld\n", (long)maxChains);
        Print(L"longestChains: %ld\n", (long)longestChain);
        Wait();
    }
#endif

    /* If this line was reached, measured boot worked */
    globals.measuredBootFailed = FALSE;

#if 0
    /* Disable keyboard input to keep GRUB from interactive mode */
    if (DisableKeyboardInput(systemTable) != EFI_SUCCESS)
    {
        LOGE(L"DisableKeyboardInput() failed");
        goto done;
    }
    else
    {
        LOGI(L"Disabled keyboard input");
    }
#endif

    /* Start the boot loader */
    if ((status = StartShim(
        imageHandle,
        systemTable,
        tcg2Protocol,
        bootfs)) != EFI_SUCCESS)
    {
        LOGE(L"failed to start boot loader");
        status = EFI_UNSUPPORTED;
        goto done;
    }

done:

    LOGE(L"No operating system was loaded: '%s'", globals.lsvmconfPath);
    return 1;
}

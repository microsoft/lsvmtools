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
#include <lsvmutils/tcg2.h>
#include <lsvmutils/tpm2.h>
#include <lsvmutils/alloc.h>
#include "logging.h"
#include "log.h"
#include "paths.h"

/*
**==============================================================================
**
** LogPCRs()
**
**==============================================================================
*/

void LogPCRs(
    EFI_TCG2_PROTOCOL* tcg2Protocol,
    EFI_HANDLE imageHandle,
    const CHAR16* message)
{
    UINT32 i;

    LOGD(Wcs(message));

    for (i = 0; i < 16; i++)
    {
        TPM_RC rc;
        SHA256Hash sha256;
        SHA256Str sha256Str;

        if ((rc = TPM2X_ReadPCRSHA256(
            tcg2Protocol, 
            i, 
            &sha256)) != TPM_RC_SUCCESS)
        {
            continue;
        }

        sha256Str = SHA256ToStr(&sha256);
        LOGD(L"PCR[%02d]=%a", U32(i), Str(sha256Str.buf));
    }
}

/*
**==============================================================================
**
** LogEventLog()
**
**==============================================================================
*/

static void _LogEvent(
    EFI_HANDLE imageHandle,
    const TCG_PCR_EVENT* event)
{
    /* Event header line */
    LOGD(L"=== EVENT:");

    /* TCG_PCR_EVENT.PCRIndex */
    LOGD(L"PCRIndex=%d", U32(event->PCRIndex));

    /* TCG_PCR_EVENT.EventType */
    LOGD(L"EventType=%X", U32(event->EventType));

    /* TCG_PCR_EVENT.Digest */
    {
        char digest[sizeof(TCG_DIGEST) * 2 + 1];
        TPM2X_BinaryToHexStr(event->Digest, sizeof(event->Digest), digest);
        LOGD(L"Digest=%a", Str(digest));
    }

    /* TCG_PCR_EVENT.EventSize */
    LOGD(L"EventSize=%d", U32(event->EventSize));

    /* TCG_PCR_EVENT.Event */
    if (event->EventSize > 0 && event->EventSize < 256)
    {
        char* data = AllocatePool((event->EventSize*2+1) * sizeof(char));

        if (!data)
            return;

        TPM2X_BinaryToHexStr(event->Event, event->EventSize, data);

        LOGD(L"Event=%a", Str(data));

        FreePool(data);
    }
}

static void _LogEventLog(
    EFI_HANDLE imageHandle,
    EFI_PHYSICAL_ADDRESS first,
    EFI_PHYSICAL_ADDRESS last)
{
    const TCG_PCR_EVENT* p = (TCG_PCR_EVENT*)first;
    const TCG_PCR_EVENT* end = (TCG_PCR_EVENT*)last;

    while (p)
    {
        _LogEvent(imageHandle, p);

        if (p == end)
            break;

        p = (TCG_PCR_EVENT*)((BYTE*)(p + 1) + p->EventSize);

        if (p > end)
            break;
    }
}

void LogEventLog(
    EFI_TCG2_PROTOCOL* tcg2Protocol,
    EFI_HANDLE imageHandle)
{
    EFI_PHYSICAL_ADDRESS EventLogLocation = 0;
    EFI_PHYSICAL_ADDRESS EventLogLastEntry = 0;
    BOOLEAN EventLogTruncated = TRUE;

    if (!tcg2Protocol)
        return;

    EFI_STATUS rc = tcg2Protocol->GetEventLog(
        tcg2Protocol,
        EFI_TCG2_EVENT_LOG_FORMAT_TCG_1_2,
        &EventLogLocation,
        &EventLogLastEntry,
        &EventLogTruncated);

    if (rc == EFI_SUCCESS)
    {
        LOGD(L"=== EVENTLOG:");

        _LogEventLog(imageHandle, EventLogLocation,
            EventLogLastEntry);
    }
}

void LogPaths()
{
    LOGD(L"globals.lsvmloadPath{%s}", globals.lsvmloadPath);
    LOGD(L"globals.lsvmlogPath{%s}", globals.lsvmlogPath);
    LOGD(L"globals.lsvmconfPath{%s}", globals.lsvmconfPath);
}

void LogSHA1(
    const CHAR16* name,
    const SHA1Hash* sha1)
{
    SHA1Str str = SHA1ToStr(sha1);
    LOGI(L"SHA1(%s)=%a", Wcs(name), Str(str.buf));
}

void LogSHA256(
    const CHAR16* name,
    const SHA256Hash* sha256)
{
    SHA256Str str = SHA256ToStr(sha256);
    LOGI(L"SHA256(%s)=%a", Wcs(name), Str(str.buf));
}

int LogHexStr(
    const CHAR16* name,
    const void* data, 
    UINTN size)
{
    int rc = -1;
    char* hexstr = NULL;

    if (!name || !data)
        goto done;

    if (!(hexstr = (char*)Malloc((size * 2) + 1)))
        goto done;

    TPM2X_BinaryToHexStr(data, size, hexstr);
    LOGI(L"%s{%a}", Wcs(name), Str(hexstr));

    rc = 0;

done:

    if (hexstr)
        Free(hexstr);

    return rc;
}

int LogASCIIStr(
    const CHAR16* name,
    const void* data, 
    UINTN size)
{
    int rc = -1;
    UINT8* copy = NULL;
    UINTN i;

    if (!name || !data)
        goto done;

    if (!(copy = Malloc(size + 1)))
        goto done;

    Memcpy(copy, data, size);
    copy[size] = '\0';

    for (i = 0; i < size; i++)
    {
        char c = copy[i];

        if (!(c >= ' ' || c <= '~'))
            copy[i] = '?';
    }

    LOGI(L"%s{%a}", Wcs(name), copy);

    rc = 0;

done:

    if (copy)
        Free(copy);

    return rc;
}

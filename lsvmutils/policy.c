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
#include "policy.h"

#if !defined(BUILD_EFI)
# include <stdio.h>
# include <stdlib.h>
#endif

#include <lsvmutils/tpm2.h>
#include <lsvmutils/alloc.h>
#include <lsvmutils/strings.h>
#include <lsvmutils/utils.h>
#include <lsvmutils/strarr.h>
#include "measure.h"
#include "print.h"

#if !defined(BUILD_EFI)
#include <lsvmutils/file.h>
#endif /* !defined(BUILD_EFI) */

static const char* _typeNames[] =
{
    "EFIVAR",
    "PEIMAGE",
    "BINARY",
    "BINARY32",
    "CAP",
    "PCR",
};

#if !defined(BUILD_EFI)
static char* _SkipSpace(char* p)
{
    while (Isspace(*p))
        p++;

    return p;
}
#endif /* !defined(BUILD_EFI) */

#if !defined(BUILD_EFI)
static char* _GetNextField(char** p)
{
    char* start;
    char* end;

    /* Find start of token */
    start = _SkipSpace(*p);

    /* Skip over token characters */
    for (end = start; *end && *end != ':'; end++)
        ;

    /* Set next pointer */
    if (*end == ':')
        *p = end + 1;
    else
        *p = end;

    /* Zero teriante the token */
    *end = '\0';

    /* Remove trailing spaces */
    while (end != start && isspace(end[-1]))
        *--end = '\0';

    /* Return pointer to this token */
    return start;
}
#endif /* !defined(BUILD_EFI) */

#if !defined(BUILD_EFI)
static const char* _ParseIncludeDirective(char* p)
{
    char* start = NULL;

    /* Fail if it does not start with "#include" */
    if (Strncmp(p, "#include", sizeof("#include") - 1) != 0)
        return NULL;

    /* Skip over "#include" */
    p += sizeof("#include") - 1;

    /* Expect space character */
    if (!isspace(*p++))
        return NULL;

    /* Skip whitespace */
    while (*p && isspace(*p))
        p++;

    /* Expect open quote */
    if (*p++ != '"')
        return NULL;

    /* Find start and end of filename */
    for (start = p; *p && *p != '"'; p++)
        ;

    /* Reject unterminated filename */
    if (*p != '"')
        return NULL;

    /* Reject zero-length filenames */
    if (p == start)
        return NULL;

    /* Zero-terminate filename */
    *p = '\0';

    return start;
}
#endif /* !defined(BUILD_EFI) */

#if !defined(BUILD_EFI)
static const char* _ParseLineDirective(char* p, UINT32* lineOut)
{
    const char* start = NULL;
    const char* filename = NULL;
    UINT32 line = 0;

    if (!p || !lineOut)
        return NULL;

    *lineOut = 0;

    /* Fail if it does not start with "#include" */
    if (Strncmp(p, "#line", sizeof("#line") - 1) != 0)
        return NULL;

    /* Skip over "#include" */
    p += sizeof("#line") - 1;

    /* Expect space character */
    if (!isspace(*p++))
        return NULL;

    /* Skip whitespace */
    while (*p && isspace(*p))
        p++;

    /* Parse the line number */
    {
        /* Find start and end of line number */
        for (start = p; *p && isdigit(*p); p++)
            ;

        /* Reject zero-length filenames */
        if (p == start)
            return NULL;

        /* Expect space character */
        if (!isspace(*p))
            return NULL;

        /* Zero-terminate the line number */
        *p++ = '\0';

        line = atoi(start);
    }

    /* Skip whitespace */
    while (*p && isspace(*p))
        p++;

    /* Parse the file name */
    {
        /* Expect open quote */
        if (*p++ != '"')
            return NULL;

        /* Find start and end of filename */
        for (start = p; *p && *p != '"'; p++)
            ;

        /* Reject unterminated filename */
        if (*p != '"')
            return NULL;

        /* Reject zero-length filenames */
        if (p == start)
            return NULL;

        /* Zero-terminate filename */
        *p = '\0';

        filename = start;
    }

    *lineOut = line;

    return filename;
}
#endif /* !defined(BUILD_EFI) */

#if !defined(BUILD_EFI)
static int _ReadFile(
    const char* path,
    StrArr* arr,
    Error* err)
{
    int rc = -1;
    FILE* is = NULL;
    char buf[POLICY_MAX_LINE];
    UINT32 line = 1;

    /* Open the policy file for input */
    if (!(is = fopen(path, "rb")))
    {
        SetErr(err, "failed to open: %s", path);
        goto done;
    }

    /* Append line directive */
    {
        char tmp[POLICY_MAX_LINE];
        snprintf(tmp, sizeof(tmp), "#line 1 \"%s\"", path);

        if (StrArrAppend(arr, tmp) != 0)
        {
            SetErr(err, "out of memory");
            goto done;
        }
    }

    /* Read line-by-line */
    for (; (fgets(buf, sizeof(buf), is)) != NULL; line++)
    {
        if (Strncmp(buf, "#include", sizeof("#include") - 1) == 0)
        {
            const char* filename;

            if (!(filename = _ParseIncludeDirective(buf)))
            {
                SetErr(err, "%s(%u): syntax", path, line);
                goto done;
            }

            if (_ReadFile(filename, arr, err) != 0)
            {
                /* Error already reported by _ReadFile() */
                goto done;
            }

            /* Append line directive */
            {
                char tmp[POLICY_MAX_LINE];
                snprintf(tmp, sizeof(tmp), "#line %u \"%s\"", line+1, path);

                if (StrArrAppend(arr, tmp) != 0)
                {
                    SetErr(err, "out of memory");
                    goto done;
                }
            }
        }
        else if (StrArrAppend(arr, buf) != 0)
        {
            SetErr(err, "out of memory");
            goto done;
        }
    }

    rc = 0;

done:

    if (is)
        fclose(is);

    return rc;
}
#endif /* !defined(BUILD_EFI) */

#if !defined(BUILD_EFI)
int LoadPolicy(
    const char* path,
    Policy* policy,
    Error* err)
{
    int rc = -1;
    UINT32 line = 1;
    const char* file = path;
    StrArr arr = STRARR_INITIALIZER;
    UINTN i;

    ClearErr(err);

    /* Check for null parameters */
    if (!path || !policy)
    {
        SetErr(err, "invalid parametrer");
        goto done;
    }

    /* Clear the policy */
    Memset(policy, 0, sizeof(Policy));

    /* Read file(s) into string array */
    if (_ReadFile(path, &arr, err) != 0)
    {
        goto done;
    }

    /* Read line-by-line */
    for (i = 0; i < arr.size; i++)
    {
        char* p = arr.data[i];
        PolicyEntry ent;

        /* Handle line directives */
        if (Strncmp(p, "#line", sizeof("#line") - 1) == 0)
        {
            const char* tmpFile = NULL;
            UINT32 tmpLine = 0;

            if (!(tmpFile = _ParseLineDirective(p, &tmpLine)))
            {
                SetErr(err, "%s(%u): invalid line directive", file, line);
                goto done;
            }

            file = tmpFile;
            line = tmpLine;
            continue;
        }

        /* Skip comment lines */
        if (p[0] == '#')
        {
            line++;
            continue;
        }

        /* Too many entries */
        if (policy->nentries == POLICY_MAX_ENTRIES)
        {
            SetErr(err, "%s(%u): too many entries", file, line);
            goto done;
        }

        /* Remove trailing whitespace */
        {
            char* end = p + strlen(p);

            while (end != p && isspace(end[-1]))
                *--end = '\0';
        }

        /* PolicyEntry.path */
        {
            const char* name;
            char tmp[POLICY_MAX_LINE];

            if (!(name = _GetNextField(&p)))
            {
                SetErr(err, "%s(%u): syntax: expected file", file, line);
                goto done;
            }

            /* Get the name */
            Strlcpy(tmp, name, sizeof(tmp));

            /* Expand any environment variables */
            if (ExpandEnvVars(ent.path, sizeof(ent.path), tmp) != 0)
            {
                SetErr(err, "%s(%u): failed to expand variables", file, line);
                goto done;
            }
        }

        /* PolicyEntry.type */
        {
            const char* type;
            
            if (!(type = _GetNextField(&p)))
            {
                SetErr(err, "%s(%u): syntax: expected type", file, line);
                goto done;
            }

            if (strcmp(type, "PEIMAGE") == 0)
                ent.type = POLICY_PEIMAGE;
            else if (strcmp(type, "BINARY") == 0)
                ent.type = POLICY_BINARY;
            else if (strcmp(type, "BINARY32") == 0)
                ent.type = POLICY_BINARY32;
            else if (strcmp(type, "EFIVAR") == 0)
                ent.type = POLICY_EFIVAR;
            else if (strcmp(type, "CAP") == 0)
                ent.type = POLICY_CAP;
            else if (strcmp(type, "PCR") == 0)
                ent.type = POLICY_PCR;
            else
            {
                SetErr(err, "%s(%u): bad hash type: %s", file, line, type);
                goto done;
            }
        }

        /* PolicyEntry.pcr */
        {
            const char* pcrStr;
            
            if (!(pcrStr = _GetNextField(&p)))
            {
                SetErr(err, "%s(%u): syntax: expected PCR", file, line);
                goto done;
            }

            char* end = NULL;
            unsigned long pcr = strtoul(pcrStr, &end, 10);

            if (!end || *end != '\0' || pcr >= MAX_PCRS)
            {
                SetErr(err, "%s(%u): bad PCR index: %s", file, line, pcrStr);
                goto done;
            }

            ent.pcr = (UINT32)pcr;
        }

        if (*p != '\0')
        {
            SetErr(err, "%s(%u): extraneous characters", file, line);
            goto done;
        }

        policy->entries[policy->nentries++] = ent;
        line++;
    }

    rc = 0;

done:

    StrArrRelease(&arr);

    return rc;
}
#endif /* !defined(BUILD_EFI) */

void DumpPolicy(
    const Policy* policy)
{
    UINT32 i;

    if (policy)
    {
        for (i = 0; i < policy->nentries; i++)
        {
            const PolicyEntry* ent = &policy->entries[i];

            PRINTF("%s:%s:%u\n", ent->path, _typeNames[ent->type], ent->pcr);
        }
    }
}

int MeasurePolicy(
    const Vars* vars,
    EFI_TCG2_PROTOCOL *protocol,
    const Policy* policy,
    BOOLEAN log,
    PCRBanks* banks,
    Error* err)
{
    int rc = -1;
    UINT32 i;

    ClearErr(err);

    /* Check for null parameters */
    if (!policy)
        goto done;

    Memset(banks, 0, sizeof(PCRBanks));

    for (i = 0; i < policy->nentries; i++)
    {
        const PolicyEntry* ent = &policy->entries[i];
        SHA1Hash sha1;
        SHA256Hash sha256;
        UINT32 pcr = ent->pcr;

        /* Set this bit in the PCR mask */
        banks->pcrMask |= (1 << pcr);

        /* Measure this entry */
        switch (ent->type)
        {
            case POLICY_EFIVAR:
            {
                if (MeasureEFIVariable(
                    vars,
                    protocol,
                    TRUE, /* predictive */
                    ent->pcr,
                    ent->path,
                    &banks->pcrs1[ent->pcr],
                    &banks->pcrs256[ent->pcr],
                    &sha1,
                    &sha256) != 0)
                {
                    SetErr(err, TCS("measurement failed: %s"), ent->path);
                    goto done;
                }

                break;
            }
            case POLICY_PEIMAGE:
            {

                if (MeasurePEImage(
                    protocol,
                    TRUE, /* predictive */
                    ent->pcr,
                    ent->path,
                    &banks->pcrs1[ent->pcr],
                    &banks->pcrs256[ent->pcr],
                    &sha1,
                    &sha256) != 0)
                {
                    SetErr(err, TCS("measurement failed: %s"), ent->path);
                    goto done;
                }

                break;
            }
            case POLICY_BINARY:
            {
                if (MeasureBinaryFile(
                    vars,
                    protocol,
                    TRUE, /* predictive */
                    ent->pcr,
                    ent->path,
                    &banks->pcrs1[ent->pcr],
                    &banks->pcrs256[ent->pcr],
                    &sha1,
                    &sha256) != 0)
                {
                    SetErr(err, TCS("measurement failed: %s"), ent->path);
                    goto done;
                }
                break;
            }
            case POLICY_BINARY32:
            {
                if (MeasureBinaryFilePad32(
                    vars,
                    protocol,
                    TRUE, /* predictive */
                    ent->pcr,
                    ent->path,
                    &banks->pcrs1[ent->pcr],
                    &banks->pcrs256[ent->pcr],
                    &sha1,
                    &sha256) != 0)
                {
                    SetErr(err, TCS("measurement failed: %s"), ent->path);
                    goto done;
                }
                break;
            }
            case POLICY_CAP:
            {
                if (MeasureCap(
                    protocol,
                    TRUE, /* predictive */
                    ent->pcr,
                    &banks->pcrs1[ent->pcr],
                    &banks->pcrs256[ent->pcr],
                    &sha1,
                    &sha256) != 0)
                {
                    SetErr(err, TCS("measurement failed: %s"), ent->path);
                    goto done;
                }

                break;
            }
            case POLICY_PCR:
            {
                /* Used only to add PCR to mask above */
                break;
            }
            default:
            {
                SetErr(err, TCS("internal error"));
                goto done;
            }
        }

        if (log)
        {
            PRINTF("=== %s\n", ent->path);
            {
                SHA1Str str = SHA1ToStr(&sha1);
                PRINTF("SHA1=%s\n", str.buf);
            }
            {
                SHA256Str str = SHA256ToStr(&sha256);
                PRINTF("SHA256=%s\n", str.buf);
            }
        }
    }

    rc = 0;

done:
    return rc;
}

int SealPolicy(
    const Vars* vars,
    EFI_TCG2_PROTOCOL *protocol,
    Policy* policy,
    BOOLEAN log,
    const BYTE dataIn[TPM2X_MAX_DATA_SIZE],
    UINT16 sizeIn,
    BYTE dataOut[TPM2X_BLOB_SIZE],
    UINTN* sizeOut,
    Error* err)
{
    int rc = -1;
    PCRBanks banks;
    TPM_HANDLE srkHandle;
    BOOLEAN srkHandleNull = TRUE;
    TPM2X_BLOB blob;
    BYTE blobBuf[TPM2X_BLOB_SIZE];
    UINT32 blobBufSize;

    ClearErr(err);

    if (dataOut)
        Memset(dataOut, 0, TPM2X_BLOB_SIZE);

    if (sizeOut)
        *sizeOut = 0;

    /* Check for null parameters */
    if (!protocol || !policy || !dataIn || !sizeIn || !dataOut || !sizeOut)
        goto done;

    /* Perform measurements to populate the PCR banks */
    if (MeasurePolicy(vars, protocol, policy, log, &banks, err) != 0)
    {
        goto done;
    }

    if (log)
    {
        UINTN i;

        for (i = 0; i < MAX_PCRS; i++)
        {
            SHA1Str str1 = SHA1ToStr(&banks.pcrs1[i]);
            PRINTF("%s\n", str1.buf);
        }
    }

    /* Check whether dataIn is too big to be sealed */
    if (sizeIn > TPM2X_MAX_DATA_SIZE)
    {
        SetErr(err, TCS("dataIn too long (> %u)"), 
            (unsigned int)TPM2X_MAX_DATA_SIZE);
        goto done;
    }

    /* Create the SRK key */
    if (TPM2X_CreateSRKKey(protocol, &srkHandle) != TPM_RC_SUCCESS)
    {
        SetErr(err, TCS("failed to create SRK key"));
        goto done;
    }

    srkHandleNull = FALSE;

    /* Seal the dataIn */
    if (TPM2X_Seal(
        protocol, 
        srkHandle, 
        TRUE,
        banks.pcrMask,
        banks.pcrs256,
        dataIn,
        sizeIn,
        &blob,
        err) != TPM_RC_SUCCESS)
    {
        SetErr(err, TCS("failed to seal policy: %s"), err->buf);
        goto done;
    }

    if (TPM2X_SerializeBlob(&blob, blobBuf, sizeof(blobBuf), &blobBufSize) != 0)
    {
        SetErr(err, TCS("failed to serialize blob"));
        goto done;
    }

    Memcpy(dataOut, blobBuf, TPM2X_BLOB_SIZE);
    *sizeOut = blobBufSize;

    rc = 0;

done:

    if (!srkHandleNull)
        TPM2_FlushContext(protocol, srkHandle);

    return rc;
}

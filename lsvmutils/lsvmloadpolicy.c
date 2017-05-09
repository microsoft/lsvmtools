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
#include "lsvmloadpolicy.h"
#include "uefidb.h"
#include "measure.h"
#include "alloc.h"
#include "strings.h"
#include "policy.h"

#define PCR07_MASK (1 << 7)
#define PCR11_MASK (1 << 11)

static const unsigned char _FALSE[] =
{
    0x00
};

static const unsigned char _TRUE[] =
{
    0x01
};

static const unsigned char _SEPARATOR[] =
{
    0x00, 0x00, 0x00, 0x00
};

static const unsigned char _ALLOW_PREBOOT_SEALING[] =
{
    0x10, 0x00, 0x00, 0x00
};

static const unsigned char _LINUX_SCENARIO_ID[] =
{
    0x02, 0x00, 0xF0, 0x00
};

static const unsigned char _LINUX_SCENARIO_VERSION[] =
{
    0x01, 0x00, 0x00, 0x00
};

static Vars _vars =
{
    {
        { 
            "8BE4DF6193CA11D2AA0D00E098032B8C-SecureBoot.False", 
            _FALSE,
            sizeof(_FALSE),
        },
        { 
            "8BE4DF6193CA11D2AA0D00E098032B8C-SecureBoot.True", 
            _TRUE,
            sizeof(_TRUE),
        },
        {
            "SEPARATOR",
            _SEPARATOR,
            sizeof(_SEPARATOR),
        },
        {
            "ALLOW_PREBOOT_SEALING",
            _ALLOW_PREBOOT_SEALING,
            sizeof(_ALLOW_PREBOOT_SEALING),
        },
        {
            "LINUX_SCENARIO_ID",
            _LINUX_SCENARIO_ID,
            sizeof(_LINUX_SCENARIO_ID),
        },
        {
            "LINUX_SCENARIO_VERSION",
            _LINUX_SCENARIO_VERSION,
            sizeof(_LINUX_SCENARIO_VERSION),
        },
    },
    6
};

static Policy _policy =
{
    {
        {"8BE4DF6193CA11D2AA0D00E098032B8C-SecureBoot.False", POLICY_EFIVAR, 7},
        {"8BE4DF6193CA11D2AA0D00E098032B8C-PK", POLICY_EFIVAR, 7}, 
        {"8BE4DF6193CA11D2AA0D00E098032B8C-KEK", POLICY_EFIVAR, 7}, 
        {"D719B2CB3D3A4596A3BCDAD00E67656F-db", POLICY_EFIVAR, 7}, 
        {"D719B2CB3D3A4596A3BCDAD00E67656F-dbx", POLICY_EFIVAR, 7}, 
        {"SEPARATOR", POLICY_BINARY, 7}, 
        {"ALLOW_PREBOOT_SEALING", POLICY_BINARY, 11}, 
        {"LINUX_SCENARIO_ID", POLICY_BINARY, 11}, 
        {"LINUX_SCENARIO_VERSION", POLICY_BINARY, 11}, 
    },
    9,
    (PCR07_MASK | PCR11_MASK)
};

static Policy _secureBootPolicy =
{
    {
        {"8BE4DF6193CA11D2AA0D00E098032B8C-SecureBoot.True", POLICY_EFIVAR, 7}, 
        {"8BE4DF6193CA11D2AA0D00E098032B8C-PK", POLICY_EFIVAR, 7}, 
        {"8BE4DF6193CA11D2AA0D00E098032B8C-KEK", POLICY_EFIVAR, 7}, 
        {"D719B2CB3D3A4596A3BCDAD00E67656F-db", POLICY_EFIVAR, 7}, 
        {"D719B2CB3D3A4596A3BCDAD00E67656F-dbx", POLICY_EFIVAR, 7}, 
        {"SEPARATOR", POLICY_BINARY, 7}, 
        {"D719B2CB3D3A4596A3BCDAD00E67656F-db.Entry", POLICY_EFIVAR, 7}, 
        {"ALLOW_PREBOOT_SEALING", POLICY_BINARY, 11}, 
        {"LINUX_SCENARIO_ID", POLICY_BINARY, 11}, 
        {"LINUX_SCENARIO_VERSION", POLICY_BINARY, 11}, 
    },
    10,
    (PCR07_MASK | PCR11_MASK)
};

static int _InitLSVMLoadPolicy(
    const void* lsvmloadData,
    UINTN lsvmloadSize,
    Error* err)
{
    int rc = -1;
    unsigned char* dbData = NULL;
    UINTN dbSize;
    UINT8* resultData = NULL;
    UINTN resultSize = 0;
    const BOOLEAN quiet = TRUE;

    if (!lsvmloadSize)
    {
        SetErr(err, TCS("_InitLSVMLoadPolicy(): null parameter"));
        goto done;
    }

    /* Load "db" variable */
    if (LoadEFIVar(
        "D719B2CB3D3A4596A3BCDAD00E67656F",
        "db",
        &dbData,
        &dbSize) != 0)
    {
        SetErr(err, TCS("failed to load 'db' UEFI variable"));
        goto done;
    }

    /* Check the lsvmload image (getting the entry) */
    if (CheckImageDB(
        dbData,
        dbSize,
        lsvmloadData,
        lsvmloadSize,
        &resultData,
        &resultSize, 
        quiet) != 0)
    {
        goto done;
    }

    /* Add the variable to the list of variables */
    {
        Var var;

        if (_vars.size == VARS_MAX)
            goto done;

        var.name = "D719B2CB3D3A4596A3BCDAD00E67656F-db.Entry";
        var.data = resultData;
        var.size = resultSize;

        _vars.data[_vars.size++] = var;
    }

    rc = 0;

done:

    if (dbData)
        Free(dbData);

    if (rc != 0)
        Free(resultData);

    return rc;
}

static int _ReleaseLSVMLoadPolicy()
{
    int rc = -1;
    const Var* var;

    if (_vars.size == 0)
        goto done;

    var = &_vars.data[_vars.size-1];

    if (Strcmp(var->name, "D719B2CB3D3A4596A3BCDAD00E67656F-db.Entry") != 0)
        goto done;

    Free((void*)var->data);
    _vars.size--;

    rc = 0;

done:
    return rc;
}

int SealLSVMLoadPolicy(
    EFI_TCG2_PROTOCOL *protocol,
    const void* lsvmloadData,
    UINTN lsvmloadSize,
    BOOLEAN sealForSecureBoot,
    const BYTE dataIn[TPM2X_MAX_DATA_SIZE],
    UINT16 sizeIn,
    BYTE dataOut[TPM2X_BLOB_SIZE],
    UINTN* sizeOut,
    Error* err)
{
    int rc = -1;
    BOOLEAN initialized = FALSE;

    /* Reject null parameters */
    if (!protocol || !lsvmloadSize || !lsvmloadSize || !dataIn || !sizeIn || 
        !dataOut || !sizeOut)
    {
        goto done;
    }

    /* Initialize this policy */
    if (sealForSecureBoot)
    {
        if (_InitLSVMLoadPolicy(lsvmloadData, lsvmloadSize, err) != 0)
            goto done;

        initialized = TRUE;
    }

    /* Perform the sealing */
    if (SealPolicy(
        &_vars,
        protocol,
        sealForSecureBoot ? &_secureBootPolicy : &_policy,
        FALSE,
        dataIn,
        sizeIn,
        dataOut,
        sizeOut,
        err) != 0)
    {
        goto done;
    }

    rc = 0;

done:

    if (initialized)
        _ReleaseLSVMLoadPolicy();

    return rc;
}

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
#include "tcg2.h"
#include "tpm2.h"
#include "strings.h"
#include "print.h"
#include "sha.h"
#include "error.h"
#include "alloc.h"
#include "tpmbuf.h"
#include "dump.h"

#if !defined(BUILD_EFI)
# include <stdio.h>
#endif

/*
**==============================================================================
**
** RETURN()
**
**==============================================================================
*/

#define RETURN(RC) return RC

#if !defined(BUILD_EFI)
static const char* __CodeToStr(UINT32 rc)
{
    const char* str;

    switch (rc)
    {
        case TPM_RC_CONTEXT_GAP:
            str = "TPM_RC_CONTEXT_GAP";
            break;
        case TPM_RC_OBJECT_MEMORY:
            str = "TPM_RC_OBJECT_MEMORY";
            break;
        case TPM_RC_SESSION_MEMORY:
            str = "TPM_RC_SESSION_MEMORY";
            break;
        case TPM_RC_MEMORY:
            str = "TPM_RC_MEMORY";
            break;
        case TPM_RC_SESSION_HANDLES:
            str = "TPM_RC_SESSION_HANDLES";
            break;
        case TPM_RC_OBJECT_HANDLES:
            str = "TPM_RC_OBJECT_HANDLES";
            break;
        case TPM_RC_LOCALITY:
            str = "TPM_RC_LOCALITY";
            break;
        case TPM_RC_YIELDED:
            str = "TPM_RC_YIELDED";
            break;
        case TPM_RC_CANCELED:
            str = "TPM_RC_CANCELED";
            break;
        case TPM_RC_TESTING:
            str = "TPM_RC_TESTING";
            break;
        case TPM_RC_REFERENCE_H0:
            str = "TPM_RC_REFERENCE_H0";
            break;
        case TPM_RC_REFERENCE_H1:
            str = "TPM_RC_REFERENCE_H1";
            break;
        case TPM_RC_REFERENCE_H2:
            str = "TPM_RC_REFERENCE_H2";
            break;
        case TPM_RC_REFERENCE_H3:
            str = "TPM_RC_REFERENCE_H3";
            break;
        case TPM_RC_REFERENCE_H4:
            str = "TPM_RC_REFERENCE_H4";
            break;
        case TPM_RC_REFERENCE_H5:
            str = "TPM_RC_REFERENCE_H5";
            break;
        case TPM_RC_REFERENCE_H6:
            str = "TPM_RC_REFERENCE_H6";
            break;
        case TPM_RC_REFERENCE_S0:
            str = "TPM_RC_REFERENCE_S0";
            break;
        case TPM_RC_REFERENCE_S1:
            str = "TPM_RC_REFERENCE_S1";
            break;
        case TPM_RC_REFERENCE_S2:
            str = "TPM_RC_REFERENCE_S2";
            break;
        case TPM_RC_REFERENCE_S3:
            str = "TPM_RC_REFERENCE_S3";
            break;
        case TPM_RC_REFERENCE_S4:
            str = "TPM_RC_REFERENCE_S4";
            break;
        case TPM_RC_REFERENCE_S5:
            str = "TPM_RC_REFERENCE_S5";
            break;
        case TPM_RC_REFERENCE_S6:
            str = "TPM_RC_REFERENCE_S6";
            break;
        case TPM_RC_NV_RATE:
            str = "TPM_RC_NV_RATE";
            break;
        case TPM_RC_LOCKOUT:
            str = "TPM_RC_LOCKOUT";
            break;
        case TPM_RC_RETRY:
            str = "TPM_RC_RETRY";
            break;
        case TPM_RC_NV_UNAVAILABLE:
            str = "TPM_RC_NV_UNAVAILABLE";
            break;
        case TPM_RC_NOT_USED:
            str = "TPM_RC_NOT_USED";
            break;
        case TPM_RC_INITIALIZE:
            str = "TPM_RC_INITIALIZE";
            break;
        case TPM_RC_FAILURE:
            str = "TPM_RC_FAILURE";
            break;
        case TPM_RC_SEQUENCE:
            str = "TPM_RC_SEQUENCE";
            break;
        case TPM_RC_PRIVATE:
            str = "TPM_RC_PRIVATE";
            break;
        case TPM_RC_HMAC:
            str = "TPM_RC_HMAC";
            break;
        case TPM_RC_DISABLED:
            str = "TPM_RC_DISABLED";
            break;
        case TPM_RC_EXCLUSIVE:
            str = "TPM_RC_EXCLUSIVE";
            break;
        case TPM_RC_AUTH_TYPE:
            str = "TPM_RC_AUTH_TYPE";
            break;
        case TPM_RC_AUTH_MISSING:
            str = "TPM_RC_AUTH_MISSING";
            break;
        case TPM_RC_POLICY:
            str = "TPM_RC_POLICY";
            break;
        case TPM_RC_PCR:
            str = "TPM_RC_PCR";
            break;
        case TPM_RC_PCR_CHANGED:
            str = "TPM_RC_PCR_CHANGED";
            break;
        case TPM_RC_UPGRADE:
            str = "TPM_RC_UPGRADE";
            break;
        case TPM_RC_TOO_MANY_CONTEXTS:
            str = "TPM_RC_TOO_MANY_CONTEXTS";
            break;
        case TPM_RC_AUTH_UNAVAILABLE:
            str = "TPM_RC_AUTH_UNAVAILABLE";
            break;
        case TPM_RC_REBOOT:
            str = "TPM_RC_REBOOT";
            break;
        case TPM_RC_UNBALANCED:
            str = "TPM_RC_UNBALANCED";
            break;
        case TPM_RC_COMMAND_SIZE:
            str = "TPM_RC_COMMAND_SIZE";
            break;
        case TPM_RC_COMMAND_CODE:
            str = "TPM_RC_COMMAND_CODE";
            break;
        case TPM_RC_AUTHSIZE:
            str = "TPM_RC_AUTHSIZE";
            break;
        case TPM_RC_AUTH_CONTEXT:
            str = "TPM_RC_AUTH_CONTEXT";
            break;
        case TPM_RC_NV_RANGE:
            str = "TPM_RC_NV_RANGE";
            break;
        case TPM_RC_NV_SIZE:
            str = "TPM_RC_NV_SIZE";
            break;
        case TPM_RC_NV_LOCKED:
            str = "TPM_RC_NV_LOCKED";
            break;
        case TPM_RC_NV_AUTHORIZATION:
            str = "TPM_RC_NV_AUTHORIZATION";
            break;
        case TPM_RC_NV_UNINITIALIZED:
            str = "TPM_RC_NV_UNINITIALIZED";
            break;
        case TPM_RC_NV_SPACE:
            str = "TPM_RC_NV_SPACE";
            break;
        case TPM_RC_NV_DEFINED:
            str = "TPM_RC_NV_DEFINED";
            break;
        case TPM_RC_BAD_CONTEXT:
            str = "TPM_RC_BAD_CONTEXT";
            break;
        case TPM_RC_CPHASH:
            str = "TPM_RC_CPHASH";
            break;
        case TPM_RC_PARENT:
            str = "TPM_RC_PARENT";
            break;
            break;
        case TPM_RC_NO_RESULT:
            str = "TPM_RC_NO_RESULT";
            break;
        case TPM_RC_SENSITIVE:
            str = "TPM_RC_SENSITIVE";
            break;
        case TPM_RC_ASYMMETRIC:
            str = "TPM_RC_ASYMMETRIC";
            break;
        case TPM_RC_ATTRIBUTES:
            str = "TPM_RC_ATTRIBUTES";
            break;
        case TPM_RC_HASH:
            str = "TPM_RC_HASH";
            break;
        case TPM_RC_VALUE:
            str = "TPM_RC_VALUE";
            break;
        case TPM_RC_HIERARCHY:
            str = "TPM_RC_HIERARCHY";
            break;
        case TPM_RC_KEY_SIZE:
            str = "TPM_RC_KEY_SIZE";
            break;
        case TPM_RC_MGF:
            str = "TPM_RC_MGF";
            break;
        case TPM_RC_MODE:
            str = "TPM_RC_MODE";
            break;
        case TPM_RC_TYPE:
            str = "TPM_RC_TYPE";
            break;
        case TPM_RC_HANDLE:
            str = "TPM_RC_HANDLE";
            break;
        case TPM_RC_KDF:
            str = "TPM_RC_KDF";
            break;
        case TPM_RC_RANGE:
            str = "TPM_RC_RANGE";
            break;
        case TPM_RC_AUTH_FAIL:
            str = "TPM_RC_AUTH_FAIL";
            break;
        case TPM_RC_NONCE:
            str = "TPM_RC_NONCE";
            break;
        case TPM_RC_PP:
            str = "TPM_RC_PP";
            break;
        case TPM_RC_SCHEME:
            str = "TPM_RC_SCHEME";
            break;
        case TPM_RC_SIZE:
            str = "TPM_RC_SIZE";
            break;
        case TPM_RC_SYMMETRIC:
            str = "TPM_RC_SYMMETRIC";
            break;
        case TPM_RC_TAG:
            str = "TPM_RC_TAG";
            break;
        case TPM_RC_SELECTOR:
            str = "TPM_RC_SELECTOR";
            break;
        case TPM_RC_INSUFFICIENT:
            str = "TPM_RC_INSUFFICIENT";
            break;
        case TPM_RC_SIGNATURE:
            str = "TPM_RC_SIGNATURE";
            break;
        case TPM_RC_KEY:
            str = "TPM_RC_KEY";
            break;
        case TPM_RC_POLICY_FAIL:
            str = "TPM_RC_POLICY_FAIL";
            break;
        case TPM_RC_INTEGRITY:
            str = "TPM_RC_INTEGRITY";
            break;
        case TPM_RC_TICKET:
            str = "TPM_RC_TICKET";
            break;
        case TPM_RC_RESERVED_BITS:
            str = "TPM_RC_RESERVED_BITS";
            break;
        case TPM_RC_BAD_AUTH:
            str = "TPM_RC_BAD_AUTH";
            break;
        case TPM_RC_EXPIRED:
            str = "TPM_RC_EXPIRED";
            break;
        case TPM_RC_POLICY_CC:
            str = "TPM_RC_POLICY_CC";
            break;
        case TPM_RC_BINDING:
            str = "TPM_RC_BINDING";
            break;
        case TPM_RC_CURVE:
            str = "TPM_RC_CURVE";
            break;
        case TPM_RC_ECC_POINT:
            str = "TPM_RC_ECC_POINT";
            break;
        default:
            str = NULL;
            break;
    }

    return str;
}
#endif /* !defined(BUILD_EFI) */

#if !defined(BUILD_EFI)
const char* TPM2X_CodeToStr(char buf[TPM2X_CODETOSTR_BUFSIZE], UINT32 rc)
{
    const char* str = __CodeToStr(rc);

    if ((str = __CodeToStr(rc)))
    {
        strcpy(buf, str);
    }
    else if (rc & TPM_RC_N_MASK)
    {
        str = __CodeToStr(~TPM_RC_N_MASK & rc);

        if (str)
        {
            char nstr[32];
            strcpy(buf, str);
            UINT32 n = (TPM_RC_N_MASK & rc) >> 8;
            sprintf(nstr, "%u", n);
            strcat(buf, "(");
            strcat(buf, nstr);
            strcat(buf, ")");
        }
        else
        {
            printf("OOPS\n");
        }
    }

    if (!str)
        strcpy(buf, "UNKNOWN");

    return buf;
}
#endif /* !defined(BUILD_EFI) */

/*
**==============================================================================
**
** _Pack_TPMS_PCR_SELECT()
**
**==============================================================================
*/
static void _Pack_TPMS_PCR_SELECT(
    TPMBuf *buf,
    const TPMS_PCR_SELECT *pcr)
{
    UINT32 i;
    TPMBufPackU8(buf, pcr->sizeofSelect);
    for (i = 0; i < pcr->sizeofSelect; i++)
    {
        TPMBufPackU8(buf, pcr->pcrSelect[i]);
    }
}

/*
**==============================================================================
**
** _Unpack_TPMS_PCR_SELECT()
**
**==============================================================================
*/
static void _Unpack_TPMS_PCR_SELECT(
    TPMBuf *buf,
    TPMS_PCR_SELECT *pcr)
{
    UINT32 i;
    TPMBufUnpackU8(buf, &pcr->sizeofSelect);
    for (i = 0; i < pcr->sizeofSelect; i++)
    {
        TPMBufUnpackU8(buf, &pcr->pcrSelect[i]);
    }
}

/*
**==============================================================================
**
** _Pack_TPMS_PCR_SELECTION()
**
**==============================================================================
*/

static void _Pack_TPMS_PCR_SELECTION(
    TPMBuf* buf,
    const TPMS_PCR_SELECTION* pcrSelection)
{
    UINT32 i;

    TPMBufPackU16(buf, pcrSelection->hash);
    TPMBufPackU8(buf, pcrSelection->sizeofSelect);

    for (i = 0; i < pcrSelection->sizeofSelect; i++)
    {
        TPMBufPackU8(buf, pcrSelection->pcrSelect[i]);
    }
}

/*
**==============================================================================
**
** _Pack_TPML_PCR_SELECTION()
**
**==============================================================================
*/

static void _Pack_TPML_PCR_SELECTION(
    TPMBuf* buf,
    const TPML_PCR_SELECTION* pcrSelection)
{
    UINT32 i;

    TPMBufPackU32(buf, pcrSelection->count);

    for (i = 0; i < pcrSelection->count; i++)
    {
        _Pack_TPMS_PCR_SELECTION(buf, &pcrSelection->pcrSelections[i]);
    }
}

/*
**==============================================================================
**
** _Unpack_TPMS_PCR_SELECTION()
**
**==============================================================================
*/

static void _Unpack_TPMS_PCR_SELECTION(
    TPMBuf* buf,
    TPMS_PCR_SELECTION* pcrSelection)
{
    UINT32 i;

    TPMBufUnpackU16(buf, &pcrSelection->hash);
    TPMBufUnpackU8(buf, &pcrSelection->sizeofSelect);

    for (i = 0; i < pcrSelection->sizeofSelect; i++)
    {
        TPMBufUnpackU8(buf, &pcrSelection->pcrSelect[i]);
    }
}

/*
**==============================================================================
**
** _Unpack_TPML_PCR_SELECTION()
**
**==============================================================================
*/

static void _Unpack_TPML_PCR_SELECTION(
    TPMBuf* buf,
    TPML_PCR_SELECTION* pcrSelection)
{
    UINT32 i;

    TPMBufUnpackU32(buf, &pcrSelection->count);

    for (i = 0; i < pcrSelection->count; i++)
    {
        _Unpack_TPMS_PCR_SELECTION(buf, &pcrSelection->pcrSelections[i]);
    }
}

/*
**==============================================================================
**
** _Unpack_TPM2B()
**
**==============================================================================
*/

static void _Unpack_TPM2B(
    TPMBuf* buf,
    TPM2B* p)
{
    UINT16 i;

    TPMBufUnpackU16(buf, &p->size);

    for (i = 0; i < p->size; i++)
        TPMBufUnpackU8(buf, &p->buffer[i]);
}

/*
**==============================================================================
**
** _Unpack_TPM2B_PRIVATE()
**
**==============================================================================
*/
static void _Unpack_TPM2B_PRIVATE(
    TPMBuf *buf,
    TPM2B_PRIVATE *private)
{
    _Unpack_TPM2B(buf, (TPM2B*) private);
}

/*
**==============================================================================
**
** _Unpack_TPM2B_DIGEST()
**
**==============================================================================
*/

static void _Unpack_TPM2B_DIGEST(
    TPMBuf* buf,
    TPM2B_DIGEST* digest)
{
    _Unpack_TPM2B(buf, (TPM2B*)digest);
}

/*
**==============================================================================
**
** _Unpack_TPML_DIGEST()
**
**==============================================================================
*/

static void _Unpack_TPML_DIGEST(
    TPMBuf* buf,
    TPML_DIGEST* digest)
{
    UINT32 i;

    TPMBufUnpackU32(buf, &digest->count);

    for (i = 0; i < digest->count; i++)
    {
        _Unpack_TPM2B_DIGEST(buf, &digest->digests[i]);
    }
}

/*
**==============================================================================
**
** _Unpack_TPMS_TAGGED_PROPERTY()
**
**==============================================================================
*/

static void _Unpack_TPMS_TAGGED_PROPERTY(
    TPMBuf* buf,
    TPMS_TAGGED_PROPERTY* property)
{
    TPMBufUnpackU32(buf, &property->property);
    TPMBufUnpackU32(buf, &property->value);
}

/*
**==============================================================================
**
** _Unpack_TPMS_CAPABILITY_DATA()
**
**==============================================================================
*/

static void _Unpack_TPMS_CAPABILITY_DATA_tpmProperties(
    TPMBuf* buf,
    TPMS_CAPABILITY_DATA* capabilityData)
{
    UINT32 i;

    TPMBufUnpackU32(buf, &capabilityData->data.tpmProperties.count);

    if (buf->error)
        return;

    for (i = 0; i < capabilityData->data.tpmProperties.count; i++)
    {
        _Unpack_TPMS_TAGGED_PROPERTY(
            buf, &capabilityData->data.tpmProperties.tpmProperty[i]);
    }
}

/*
**==============================================================================
**
** _Pack_TPMS_AUTH_COMMAND()
**
**==============================================================================
*/

static void _Pack_TPMS_AUTH_COMMAND(
    TPMBuf* buf,
    const TPMS_AUTH_COMMAND* authCommand)
{
    UINT32 start;

    /* Pack dummy authCommand size (save authCommand start offset) */
    TPMBufPackU32(buf, 0);
    start = buf->size;

    /* sessionHandle */
    TPMBufPackU32(buf, authCommand->sessionHandle);

    /* nonce */
    TPMBufPackU16(buf, authCommand->nonce.size);
    TPMBufPack(buf, authCommand->nonce.buffer, authCommand->nonce.size);

    /* sessionAttributes */
    TPMBufPackU8(buf, *((const UINT8*)&authCommand->sessionAttributes));

    /* hmac */
    TPMBufPackU16(buf, authCommand->hmac.size);
    TPMBufPack(buf, authCommand->hmac.buffer, authCommand->hmac.size);

    /* Update authCommand size */
    {
        UINT32 tmp =ByteSwapU32(buf->size - start);
        Memcpy(&buf->data[start-sizeof(UINT32)], &tmp, sizeof(tmp));
    }
}

/*
**==============================================================================
**
** _Unpack_TPMS_AUTH_RESPONSE()
**
**==============================================================================
*/

static void _Unpack_TPMS_AUTH_RESPONSE(
    TPMBuf* buf,
    TPMS_AUTH_RESPONSE* p)
{
    /* nonce */
    TPMBufUnpackU16(buf, &p->nonce.size);

    if (p->nonce.size)
        TPMBufUnpack(buf, &p->nonce.buffer, p->nonce.size);

    /* sessionAttributes */
    {
        UINT8 tmp;
        UINT32 tmp32;

        TPMBufUnpackU8(buf, &tmp);

        tmp32 = tmp;
        Memcpy(&p->sessionAttributes, &tmp32, sizeof(UINT32));
    }

    /* hmac */
    TPMBufUnpackU16(buf, &p->hmac.size);

    if (p->hmac.size)
        TPMBufUnpack(buf, &p->hmac.buffer, p->hmac.size);
}

/*
**==============================================================================
**
** _Pack_TPM2B()
**
**==============================================================================
*/

static void _Pack_TPM2B(
    TPMBuf *buf, 
    UINT16 size,
    const BYTE* buffer)
{
    UINT16 i;

    TPMBufPackU16(buf, size);

    for (i = 0; i < size; i++)
        TPMBufPackU8(buf, buffer[i]);
}

/*
**==============================================================================
**
** _Pack_TPM2B_PRIVATE()
**
**==============================================================================
*/
static void _Pack_TPM2B_PRIVATE(
    TPMBuf *buf,
    const TPM2B_PRIVATE *private)
{
    _Pack_TPM2B(buf, private->size, private->buffer);
}

/*
**==============================================================================
**
** _Pack_TPM2B_DIGEST()
**
**==============================================================================
*/
static void _Pack_TPM2B_DIGEST(
    TPMBuf *buf,
    const TPM2B_DIGEST *digest)
{
    _Pack_TPM2B(buf, digest->size, digest->buffer);
}

/*
**==============================================================================
**
** _Pack_TPMS_SENSITIVE_CREATE()
**
**==============================================================================
*/

static void _Pack_TPMS_SENSITIVE_CREATE(
    TPMBuf *buf, 
    TPMS_SENSITIVE_CREATE *p)
{
    _Pack_TPM2B(buf, p->userAuth.size, p->userAuth.buffer);
    _Pack_TPM2B(buf, p->data.size, p->data.buffer);
}

/*
**==============================================================================
**
** _Pack_TPM2B_SENSITIVE_CREATE()
**
**==============================================================================
*/

static void _Pack_TPM2B_SENSITIVE_CREATE(
    TPMBuf *buf, 
    TPM2B_SENSITIVE_CREATE *sensitiveCreate)
{
    if (sensitiveCreate)
    {
        UINT32 start;
        UINT16 size;

        /* size */
        TPMBufPackU16(buf, sensitiveCreate->size);

        start = buf->size;
        _Pack_TPMS_SENSITIVE_CREATE(buf, &sensitiveCreate->sensitive);
        size = ByteSwapU16(buf->size - start);

        Memcpy(&buf->data[start-sizeof(UINT16)], &size, sizeof(size));
    }
    else
    {
        /* size */
        TPMBufPackU16(buf, 0);
    }
}

/*
**==============================================================================
**
** _Pack_TPMA_OBJECT()
**
**==============================================================================
*/

static void _Pack_TPMA_OBJECT(
    TPMBuf *buf, 
    const TPMA_OBJECT *p)
{
    TPMBufPackU32(buf, *((const UINT32*)p));
}

/*
**==============================================================================
**
** _Pack_TPMS_SCHEME_XOR()
**
**==============================================================================
*/

static void _Pack_TPMS_SCHEME_XOR(
    TPMBuf *buf, 
    TPMS_SCHEME_XOR *p)
{
    TPMBufPackU16(buf, p->hashAlg);
    TPMBufPackU16(buf, p->kdf);
}

/*
**==============================================================================
**
** _Pack_TPMS_SCHEME_HMAC()
**
**==============================================================================
*/

static void _Pack_TPMS_SCHEME_HMAC(
    TPMBuf *buf, 
    TPMS_SCHEME_HMAC *p)
{
    TPMBufPackU16(buf, p->hashAlg);
}

/*
**==============================================================================
**
** _Pack_TPMU_SCHEME_KEYEDHASH()
**
**==============================================================================
*/

static void _Pack_TPMU_SCHEME_KEYEDHASH(
    TPMBuf *buf, 
    TPMI_ALG_KEYEDHASH_SCHEME scheme,
    TPMU_SCHEME_KEYEDHASH *p)
{
    switch (scheme)
    {
        case TPM_ALG_HMAC:
            _Pack_TPMS_SCHEME_HMAC(buf, &p->hmac);
            break;
        case TPM_ALG_XOR:
            _Pack_TPMS_SCHEME_XOR(buf, &p->exclusiveOr);
            break;
        case TPM_ALG_NULL:
            break;
        default:
            buf->error = 1;
            break;
    }
}

/*
**==============================================================================
**
** _Pack_TPMT_KEYEDHASH_SCHEME()
**
**==============================================================================
*/

static void _Pack_TPMT_KEYEDHASH_SCHEME(
    TPMBuf *buf, 
    TPMT_KEYEDHASH_SCHEME *p)
{
    TPMBufPackU16(buf, p->scheme);
    _Pack_TPMU_SCHEME_KEYEDHASH(buf, p->scheme, &p->details);
}

/*
**==============================================================================
**
** _Pack_TPMS_KEYEDHASH_PARMS()
**
**==============================================================================
*/

static void _Pack_TPMS_KEYEDHASH_PARMS(
    TPMBuf *buf, 
    TPMS_KEYEDHASH_PARMS *p)
{
    _Pack_TPMT_KEYEDHASH_SCHEME(buf, &p->scheme);
}

/*
**==============================================================================
**
** _Pack_TPMU_SYM_KEY_BITS()
**
**==============================================================================
*/

static void _Pack_TPMU_SYM_KEY_BITS(
    TPMBuf *buf, 
    TPMI_ALG_SYM_OBJECT algorithm,
    TPMU_SYM_KEY_BITS *p)
{
    switch (algorithm)
    {
        case TPM_ALG_AES:
        case TPM_ALG_SM4:
        case TPM_ALG_CAMELLIA:
        case TPM_ALG_XOR:
            TPMBufPackU16(buf, *((const UINT16*)p));
            break;
        case TPM_ALG_NULL:
            break;
    }
}

/*
**==============================================================================
**
** _Pack_TPMU_SYM_MODE()
**
**==============================================================================
*/

static void _Pack_TPMU_SYM_MODE(
    TPMBuf *buf, 
    TPMI_ALG_SYM_OBJECT algorithm,
    TPMU_SYM_MODE *p)
{
    switch (algorithm)
    {
        case TPM_ALG_AES:
        case TPM_ALG_SM4:
        case TPM_ALG_CAMELLIA:
            TPMBufPackU16(buf, *((const UINT16*)p));
            break;
        case TPM_ALG_XOR:
        case TPM_ALG_NULL:
            break;
    }
}

/*
**==============================================================================
**
** _Pack_TPMT_SYM_DEF_OBJECT()
**
**==============================================================================
*/

static void _Pack_TPMT_SYM_DEF_OBJECT(
    TPMBuf *buf, 
    TPMT_SYM_DEF_OBJECT *p)
{
    TPMBufPackU16(buf, p->algorithm);
    _Pack_TPMU_SYM_KEY_BITS(buf, p->algorithm, &p->keyBits);
    _Pack_TPMU_SYM_MODE(buf, p->algorithm, &p->mode);
}

/*
**==============================================================================
**
** _Pack_TPMU_ASYM_SCHEME()
**
**==============================================================================
*/

static void _Pack_TPMU_ASYM_SCHEME(
    TPMBuf *buf, 
    TPMI_ALG_RSA_DECRYPT scheme,
    TPMU_ASYM_SCHEME *p)
{
    switch (scheme)
    {
        case TPM_ALG_NULL:
            break;
        default:
            /* Unsupported */
            buf->error = 1;
            break;
    }
}

/*
**==============================================================================
**
** _Pack_TPMT_RSA_SCHEME()
**
**==============================================================================
*/

static void _Pack_TPMT_RSA_SCHEME(
    TPMBuf *buf, 
    TPMT_RSA_SCHEME *p)
{
    TPMBufPackU16(buf, p->scheme);
    _Pack_TPMU_ASYM_SCHEME(buf, p->scheme, &p->details);
}

/*
**==============================================================================
**
** _Pack_TPMS_RSA_PARMS()
**
**==============================================================================
*/

static void _Pack_TPMS_RSA_PARMS(
    TPMBuf *buf, 
    TPMS_RSA_PARMS *p)
{
    _Pack_TPMT_SYM_DEF_OBJECT(buf, &p->symmetric);
    _Pack_TPMT_RSA_SCHEME(buf, &p->scheme);
    TPMBufPackU16(buf, p->keyBits);
    TPMBufPackU32(buf, p->exponent);
}

/*
**==============================================================================
**
** _Pack_TPMS_SYMCIPHER_PARMS()
**
**==============================================================================
*/

static void _Pack_TPMS_SYMCIPHER_PARMS(
    TPMBuf *buf, 
    TPMS_SYMCIPHER_PARMS *p)
{
    _Pack_TPMT_SYM_DEF_OBJECT(buf, &p->sym);
}

/*
**==============================================================================
**
** _Pack_TPMT_ECC_SCHEME()
**
**==============================================================================
*/

static void _Pack_TPMT_ECC_SCHEME(
    TPMBuf *buf, 
    TPMT_ECC_SCHEME *p)
{
    TPMBufPackU16(buf, p->scheme);
    _Pack_TPMU_ASYM_SCHEME(buf, p->scheme, &p->details);
}

/*
**==============================================================================
**
** _Pack_TPMU_KDF_SCHEME()
**
**==============================================================================
*/

static void _Pack_TPMU_KDF_SCHEME(
    TPMBuf *buf, 
    TPMI_ALG_KDF scheme,
    TPMU_KDF_SCHEME *p)
{
    switch (scheme)
    {
        case TPM_ALG_MGF1:
            TPMBufPackU16(buf, p->mgf1.hashAlg);
            break;
        case TPM_ALG_KDF1_SP800_56A:
            TPMBufPackU16(buf, p->kdf1_sp800_56a.hashAlg);
            break;
        case TPM_ALG_KDF2:
            TPMBufPackU16(buf, p->kdf2.hashAlg);
            break;
        case TPM_ALG_KDF1_SP800_108:
            TPMBufPackU16(buf, p->kdf1_sp800_108.hashAlg);
            break;
        case TPM_ALG_NULL:
            break;
        default:
            buf->error = 1;
            break;
    }
}

/*
**==============================================================================
**
** _Pack_TPMT_KDF_SCHEME()
**
**==============================================================================
*/

static void _Pack_TPMT_KDF_SCHEME(
    TPMBuf *buf, 
    TPMT_KDF_SCHEME *p)
{
    TPMBufPackU16(buf, p->scheme);
    _Pack_TPMU_KDF_SCHEME(buf, p->scheme, &p->details);
}

/*
**==============================================================================
**
** _Pack_TPMS_ECC_PARMS()
**
**==============================================================================
*/

static void _Pack_TPMS_ECC_PARMS(
    TPMBuf *buf, 
    TPMS_ECC_PARMS *p)
{
    _Pack_TPMT_SYM_DEF_OBJECT(buf, &p->symmetric);
    _Pack_TPMT_ECC_SCHEME(buf, &p->scheme);
    TPMBufPackU16(buf, p->curveID);
    _Pack_TPMT_KDF_SCHEME(buf, &p->kdf);
}

/*
**==============================================================================
**
** _Pack_TPMU_PUBLIC_PARMS()
**
**==============================================================================
*/

static void _Pack_TPMU_PUBLIC_PARMS(
    TPMBuf *buf, 
    UINT32 type,
    TPMU_PUBLIC_PARMS *p)
{
    switch (type)
    {
        case TPM_ALG_KEYEDHASH:
            _Pack_TPMS_KEYEDHASH_PARMS(buf, &p->keyedHashDetail);
            break;
        case TPM_ALG_SYMCIPHER:
            _Pack_TPMS_SYMCIPHER_PARMS(buf, &p->symDetail);
            break;
        case TPM_ALG_RSA:
            _Pack_TPMS_RSA_PARMS(buf, &p->rsaDetail);
            break;
        case TPM_ALG_ECC:
            _Pack_TPMS_ECC_PARMS(buf, &p->eccDetail);
            break;
        default:
            buf->error = 1;
            break;
    }
}

/*
**==============================================================================
**
** _Pack_TPMS_ECC_POINT()
**
**==============================================================================
*/

static void _Pack_TPMS_ECC_POINT(
    TPMBuf *buf, 
    TPMS_ECC_POINT *p)
{
    _Pack_TPM2B(buf, p->x.size, p->x.buffer);
    _Pack_TPM2B(buf, p->y.size, p->y.buffer);
}

/*
**==============================================================================
**
** _Pack_TPMU_PUBLIC_ID()
**
**==============================================================================
*/

static void _Pack_TPMU_PUBLIC_ID(
    TPMBuf *buf, 
    TPMI_ALG_PUBLIC type,
    TPMU_PUBLIC_ID *p)
{
    switch(type)
    {
        case TPM_ALG_KEYEDHASH:
            _Pack_TPM2B(buf, p->keyedHash.size, p->keyedHash.buffer);
            break;
        case TPM_ALG_SYMCIPHER:
            _Pack_TPM2B(buf, p->sym.size, p->sym.buffer);
            break;
        case TPM_ALG_RSA:
            _Pack_TPM2B(buf, p->rsa.size, p->rsa.buffer);
            break;
        case TPM_ALG_ECC:
            _Pack_TPMS_ECC_POINT(buf, &p->ecc);
            break;
    }
}

/*
**==============================================================================
**
** _Pack_TPMT_PUBLIC()
**
**==============================================================================
*/

static void _Pack_TPMT_PUBLIC(
    TPMBuf *buf, 
    TPMT_PUBLIC *p)
{
    TPMBufPackU16(buf, p->type);
    TPMBufPackU16(buf, p->nameAlg);
    _Pack_TPMA_OBJECT(buf, &p->objectAttributes);
    _Pack_TPM2B(buf, p->authPolicy.size, p->authPolicy.buffer);
    _Pack_TPMU_PUBLIC_PARMS(buf, p->type, &p->parameters);
    _Pack_TPMU_PUBLIC_ID(buf, p->type, &p->unique);
}

/*
**==============================================================================
**
** _Pack_TPM2B_PUBLIC()
**
**==============================================================================
*/

static void _Pack_TPM2B_PUBLIC(
    TPMBuf *buf, 
    TPM2B_PUBLIC *p)
{
    if (p)
    {
        UINT32 start;
        UINT16 size;

        TPMBufPackU16(buf, p->size);

        start = buf->size;
        _Pack_TPMT_PUBLIC(buf, &p->publicArea);
        size = ByteSwapU16(buf->size - start);
        Memcpy(&buf->data[start-sizeof(UINT16)], &size, sizeof(size));
    }
    else
    {
        TPMBufPackU16(buf, 0);
    }
}

/*
**==============================================================================
**
** _Unpack_TPMA_OBJECT()
**
**==============================================================================
*/

static void _Unpack_TPMA_OBJECT(
    TPMBuf *buf, 
    TPMA_OBJECT *p)
{
    TPMBufUnpackU32(buf, (UINT32*)p);
}

/*
**==============================================================================
**
** _Unpack_TPMS_SCHEME_HMAC()
**
**==============================================================================
*/

static void _Unpack_TPMS_SCHEME_HMAC(
    TPMBuf *buf, 
    TPMS_SCHEME_HMAC *p)
{
    TPMBufUnpackU16(buf, &p->hashAlg);
}

/*
**==============================================================================
**
** _Unpack_TPMS_SCHEME_XOR()
**
**==============================================================================
*/

static void _Unpack_TPMS_SCHEME_XOR(
    TPMBuf *buf, 
    TPMS_SCHEME_XOR *p)
{
    TPMBufUnpackU16(buf, &p->hashAlg);
    TPMBufUnpackU16(buf, &p->kdf);
}

/*
**==============================================================================
**
** _Unpack_TPMU_SCHEME_KEYEDHASH()
**
**==============================================================================
*/

static void _Unpack_TPMU_SCHEME_KEYEDHASH(
    TPMBuf *buf, 
    TPMI_ALG_KEYEDHASH_SCHEME scheme,
    TPMU_SCHEME_KEYEDHASH *p)
{
    switch (scheme)
    {
        case TPM_ALG_HMAC:
            _Unpack_TPMS_SCHEME_HMAC(buf, &p->hmac);
            break;
        case TPM_ALG_XOR:
            _Unpack_TPMS_SCHEME_XOR(buf, &p->exclusiveOr);
            break;
        case TPM_ALG_NULL:
            break;
        default:
            buf->error = 1;
            break;
    }
}

/*
**==============================================================================
**
** _Unpack_TPMT_KEYEDHASH_SCHEME()
**
**==============================================================================
*/

static void _Unpack_TPMT_KEYEDHASH_SCHEME(
    TPMBuf *buf, 
    TPMT_KEYEDHASH_SCHEME *p)
{
    TPMBufUnpackU16(buf, &p->scheme);
    _Unpack_TPMU_SCHEME_KEYEDHASH(buf, p->scheme, &p->details);
}

/*
**==============================================================================
**
** _Unpack_TPMS_KEYEDHASH_PARMS()
**
**==============================================================================
*/

static void _Unpack_TPMS_KEYEDHASH_PARMS(
    TPMBuf *buf, 
    TPMS_KEYEDHASH_PARMS *p)
{
    _Unpack_TPMT_KEYEDHASH_SCHEME(buf, &p->scheme);
}

/*
**==============================================================================
**
** _Unpack_TPMU_SYM_KEY_BITS()
**
**==============================================================================
*/

static void _Unpack_TPMU_SYM_KEY_BITS(
    TPMBuf *buf, 
    TPMI_ALG_SYM_OBJECT algorithm,
    TPMU_SYM_KEY_BITS *p)
{
    switch (algorithm)
    {
        case TPM_ALG_AES:
        case TPM_ALG_SM4:
        case TPM_ALG_CAMELLIA:
        case TPM_ALG_XOR:
            TPMBufUnpackU16(buf, (UINT16*)p);
            break;
        case TPM_ALG_NULL:
            break;
    }
}

/*
**==============================================================================
**
** _Unpack_TPMU_SYM_MODE()
**
**==============================================================================
*/

static void _Unpack_TPMU_SYM_MODE(
    TPMBuf *buf, 
    TPMI_ALG_SYM_OBJECT algorithm,
    TPMU_SYM_MODE *p)
{
    switch (algorithm)
    {
        case TPM_ALG_AES:
        case TPM_ALG_SM4:
        case TPM_ALG_CAMELLIA:
            TPMBufUnpackU16(buf, (UINT16*)p);
            break;
        case TPM_ALG_XOR:
        case TPM_ALG_NULL:
            break;
    }
}

/*
**==============================================================================
**
** _Unpack_TPMT_SYM_DEF_OBJECT()
**
**==============================================================================
*/

static void _Unpack_TPMT_SYM_DEF_OBJECT(
    TPMBuf *buf, 
    TPMT_SYM_DEF_OBJECT *p)
{
    TPMBufUnpackU16(buf, &p->algorithm);
    _Unpack_TPMU_SYM_KEY_BITS(buf, p->algorithm, &p->keyBits);
    _Unpack_TPMU_SYM_MODE(buf, p->algorithm, &p->mode);
}

/*
**==============================================================================
**
** _Unpack_TPMS_SYMCIPHER_PARMS()
**
**==============================================================================
*/

static void _Unpack_TPMS_SYMCIPHER_PARMS(
    TPMBuf *buf, 
    TPMS_SYMCIPHER_PARMS *p)
{
    _Unpack_TPMT_SYM_DEF_OBJECT(buf, &p->sym);
}

/*
**==============================================================================
**
** _Unpack_TPMU_ASYM_SCHEME()
**
**==============================================================================
*/

static void _Unpack_TPMU_ASYM_SCHEME(
    TPMBuf *buf, 
    TPMI_ALG_RSA_DECRYPT scheme,
    TPMU_ASYM_SCHEME *p)
{
    switch (scheme)
    {
        case TPM_ALG_NULL:
            break;
        default:
            /* Unsupported */
            buf->error = 1;
            break;
    }
}

/*
**==============================================================================
**
** _Unpack_TPMT_RSA_SCHEME()
**
**==============================================================================
*/

static void _Unpack_TPMT_RSA_SCHEME(
    TPMBuf *buf, 
    TPMT_RSA_SCHEME *p)
{
    TPMBufUnpackU16(buf, &p->scheme);
    _Unpack_TPMU_ASYM_SCHEME(buf, p->scheme, &p->details);
}

/*
**==============================================================================
**
** _Unpack_TPMS_RSA_PARMS()
**
**==============================================================================
*/

static void _Unpack_TPMS_RSA_PARMS(
    TPMBuf *buf, 
    TPMS_RSA_PARMS *p)
{
    _Unpack_TPMT_SYM_DEF_OBJECT(buf, &p->symmetric);
    _Unpack_TPMT_RSA_SCHEME(buf, &p->scheme);
    TPMBufUnpackU16(buf, &p->keyBits);
    TPMBufUnpackU32(buf, &p->exponent);
}

/*
**==============================================================================
**
** _Unpack_TPMT_ECC_SCHEME()
**
**==============================================================================
*/

static void _Unpack_TPMT_ECC_SCHEME(
    TPMBuf *buf, 
    TPMT_ECC_SCHEME *p)
{
    TPMBufUnpackU16(buf, &p->scheme);
    _Unpack_TPMU_ASYM_SCHEME(buf, p->scheme, &p->details);
}

/*
**==============================================================================
**
** _Unpack_TPMT_KDF_SCHEME()
**
**==============================================================================
*/

static void _Unpack_TPMU_KDF_SCHEME(
    TPMBuf *buf, 
    TPMI_ALG_KDF scheme,
    TPMU_KDF_SCHEME *p)
{
    switch (scheme)
    {
        case TPM_ALG_MGF1:
            TPMBufUnpackU16(buf, &p->mgf1.hashAlg);
            break;
        case TPM_ALG_KDF1_SP800_56A:
            TPMBufUnpackU16(buf, &p->kdf1_sp800_56a.hashAlg);
            break;
        case TPM_ALG_KDF2:
            TPMBufUnpackU16(buf, &p->kdf2.hashAlg);
            break;
        case TPM_ALG_KDF1_SP800_108:
            TPMBufUnpackU16(buf, &p->kdf1_sp800_108.hashAlg);
            break;
        case TPM_ALG_NULL:
            break;
        default:
            buf->error = 1;
            break;
    }
}

/*
**==============================================================================
**
** _Unpack_TPMT_KDF_SCHEME()
**
**==============================================================================
*/

static void _Unpack_TPMT_KDF_SCHEME(
    TPMBuf *buf, 
    TPMT_KDF_SCHEME *p)
{
    TPMBufUnpackU16(buf, &p->scheme);
    _Unpack_TPMU_KDF_SCHEME(buf, p->scheme, &p->details);
}

/*
**==============================================================================
**
** _Unpack_TPMS_ECC_PARMS()
**
**==============================================================================
*/

static void _Unpack_TPMS_ECC_PARMS(
    TPMBuf *buf, 
    TPMS_ECC_PARMS *p)
{
    _Unpack_TPMT_SYM_DEF_OBJECT(buf, &p->symmetric);
    _Unpack_TPMT_ECC_SCHEME(buf, &p->scheme );
    TPMBufUnpackU16(buf, &p->curveID);
    _Unpack_TPMT_KDF_SCHEME(buf, &p->kdf);
}

/*
**==============================================================================
**
** _Unpack_TPMU_PUBLIC_PARMS()
**
**==============================================================================
*/

static void _Unpack_TPMU_PUBLIC_PARMS(
    TPMBuf *buf, 
    UINT32 type,
    TPMU_PUBLIC_PARMS *p)
{
    switch (type)
    {
        case TPM_ALG_KEYEDHASH:
            _Unpack_TPMS_KEYEDHASH_PARMS(buf, &p->keyedHashDetail);
            break;
        case TPM_ALG_SYMCIPHER:
            _Unpack_TPMS_SYMCIPHER_PARMS(buf, &p->symDetail);
            break;
        case TPM_ALG_RSA:
            _Unpack_TPMS_RSA_PARMS(buf, &p->rsaDetail);
            break;
        case TPM_ALG_ECC:
            _Unpack_TPMS_ECC_PARMS(buf, &p->eccDetail);
            break;
        default:
            buf->error = 1;
            break;
    }
}

/*
**==============================================================================
**
** _Unpack_TPMS_ECC_POINT()
**
**==============================================================================
*/

static void _Unpack_TPMS_ECC_POINT(
    TPMBuf *buf, 
    TPMS_ECC_POINT *p)
{
    _Unpack_TPM2B(buf, (TPM2B*)&p->x);
    _Unpack_TPM2B(buf, (TPM2B*)&p->y);
}

/*
**==============================================================================
**
** _Unpack_TPMU_PUBLIC_ID()
**
**==============================================================================
*/

static void _Unpack_TPMU_PUBLIC_ID(
    TPMBuf *buf, 
    TPMI_ALG_PUBLIC type,
    TPMU_PUBLIC_ID *p)
{
    switch(type)
    {
        case TPM_ALG_KEYEDHASH:
            _Unpack_TPM2B(buf, (TPM2B*)&p->keyedHash);
            break;
        case TPM_ALG_SYMCIPHER:
            _Unpack_TPM2B(buf, (TPM2B*)&p->sym);
            break;
        case TPM_ALG_RSA:
            _Unpack_TPM2B(buf, (TPM2B*)&p->rsa);
            break;
        case TPM_ALG_ECC:
            _Unpack_TPMS_ECC_POINT(buf, &p->ecc);
            break;
    }
}

/*
**==============================================================================
**
** _Unpack_TPMT_PUBLIC()
**
**==============================================================================
*/

static void _Unpack_TPMT_PUBLIC(
    TPMBuf *buf, 
    TPMT_PUBLIC *p)
{
    TPMBufUnpackU16(buf, &p->type);
    TPMBufUnpackU16(buf, &p->nameAlg);
    _Unpack_TPMA_OBJECT(buf, &p->objectAttributes);
    _Unpack_TPM2B(buf, (TPM2B*)&p->authPolicy);
    _Unpack_TPMU_PUBLIC_PARMS(buf, p->type, &p->parameters);
    _Unpack_TPMU_PUBLIC_ID(buf, p->type, &p->unique);
}

/*
**==============================================================================
**
** _Unpack_TPM2B_PUBLIC()
**
**==============================================================================
*/

static void _Unpack_TPM2B_PUBLIC(
    TPMBuf *buf, 
    TPM2B_PUBLIC *p)
{
    TPMBufUnpackU16(buf, &p->size);
    _Unpack_TPMT_PUBLIC(buf, &p->publicArea);
}

/*
**==============================================================================
**
** _Unpack_TPMT_TK_CREATION()
**
**==============================================================================
*/

static void _Unpack_TPMT_TK_CREATION(
    TPMBuf *buf, 
    TPMT_TK_CREATION *p)
{
    TPMBufUnpackU16(buf, &p->tag);
    TPMBufUnpackU32(buf, &p->hierarchy);
    _Unpack_TPM2B(buf, (TPM2B*)&p->digest);
}

/*
**==============================================================================
**
** _Pack_TPMT_SYM_DEF()
**
**==============================================================================
*/

static void _Pack_TPMT_SYM_DEF(
    TPMBuf *buf, 
    TPMT_SYM_DEF *p)
{
    TPMBufPackU16(buf, p->algorithm);
    _Pack_TPMU_SYM_KEY_BITS(buf, p->algorithm, &p->keyBits);
    _Pack_TPMU_SYM_MODE(buf, p->algorithm, &p->mode);
}

/*
**==============================================================================
**
** _Pack_TPMU_HA()
**
**==============================================================================
*/

static void _Pack_TPMU_HA(
    TPMBuf *buf, 
    UINT32 hashAlg,
    TPMU_HA *p)
{
    switch (hashAlg)
    {
        case TPM_ALG_SHA1:
            TPMBufPack(buf, p->sha1, sizeof(p->sha1));
            break;
        case TPM_ALG_SHA256:
            TPMBufPack(buf, p->sha256, sizeof(p->sha256));
            break;
        case TPM_ALG_SHA384:
            TPMBufPack(buf, p->sha384, sizeof(p->sha384));
            break;
        case TPM_ALG_SHA512:
            TPMBufPack(buf, p->sha512, sizeof(p->sha512));
            break;
        case TPM_ALG_SM3_256:
            TPMBufPack(buf, p->sm3_256, sizeof(p->sm3_256));
            break;
        default:
            /* unsupported */
            buf->error = 1;
            break;
    }
}

/*
**==============================================================================
**
** _Pack_TPMT_HA()
**
**==============================================================================
*/

static void _Pack_TPMT_HA(
    TPMBuf *buf, 
    TPMT_HA *p)
{
    TPMBufPackU16(buf, p->hashAlg);
    _Pack_TPMU_HA(buf, p->hashAlg, &p->digest);
}

/*
**==============================================================================
**
** _Pack_TPML_DIGEST_VALUES()
**
**==============================================================================
*/

static void _Pack_TPML_DIGEST_VALUES(
    TPMBuf *buf, 
    TPML_DIGEST_VALUES *p)
{
    UINT32 i;

    TPMBufPackU32(buf, p->count);

    for (i = 0; i < p->count; i++)
    {
        _Pack_TPMT_HA(buf, &p->digests[i]);
    }
}

/*
**==============================================================================
**
** _Pack_TPMU_SENSITIVE_COMPOSITE()
**
**==============================================================================
*/

static void _Pack_TPMU_SENSITIVE_COMPOSITE(
    TPMBuf *buf, 
    UINT32 type,
    TPMU_SENSITIVE_COMPOSITE *p)
{
    switch(type)
    {
        case TPM_ALG_RSA:
            _Pack_TPM2B(buf, p->rsa.size, p->rsa.buffer);
            break;
        case TPM_ALG_ECC:
            _Pack_TPM2B(buf, p->ecc.size, p->ecc.buffer);
            break;
        case TPM_ALG_KEYEDHASH:
            _Pack_TPM2B(buf, p->bits.size, p->bits.buffer);
            break;
        case TPM_ALG_SYMCIPHER:
            _Pack_TPM2B(buf, p->sym.size, p->sym.buffer);
            break;
    }
}

/*
**==============================================================================
**
** _Pack_TPMT_SENSITIVE()
**
**==============================================================================
*/

static void _Pack_TPMT_SENSITIVE(
    TPMBuf *buf, 
    TPMT_SENSITIVE *p)
{
    TPMBufPackU16(buf, p->sensitiveType);
    _Pack_TPM2B(buf, p->authValue.size, p->authValue.buffer);
    _Pack_TPM2B(buf, p->seedValue.size, p->seedValue.buffer);
    _Pack_TPMU_SENSITIVE_COMPOSITE(buf, p->sensitiveType, &p->sensitive);
}

/*
**==============================================================================
**
** _Pack_TPM2B_SENSITIVE()
**
**==============================================================================
*/

static void _Pack_TPM2B_SENSITIVE(
    TPMBuf *buf, 
    TPM2B_SENSITIVE *p)
{
    UINT32 start;
    UINT16 size;

    TPMBufPackU16(buf, 0);

    start = buf->size;
    _Pack_TPMT_SENSITIVE(buf, &p->sensitiveArea);
    size = buf->size - start;

    size = ByteSwapU16(size);
    Memcpy(&buf->data[start-sizeof(UINT16)], &size, sizeof(size));
}

/*
**==============================================================================
**
** _Unpack_TPMT_TK_HASHCHECK()
**
**==============================================================================
*/

static void _Unpack_TPMT_TK_HASHCHECK(
    TPMBuf *buf, 
    TPMT_TK_HASHCHECK *p)
{
    TPMBufUnpackU16(buf, &p->tag);
    TPMBufUnpackU32(buf, &p->hierarchy);
    _Unpack_TPM2B(buf, (TPM2B*)&p->digest);
}

/*
**==============================================================================
**
** _TCG2GetCapabilities()
**
**==============================================================================
*/

static const EFI_TCG2_BOOT_SERVICE_CAPABILITY* _TCG2GetCapabilities(
    EFI_TCG2_PROTOCOL* protocol)
{
    static EFI_TCG2_BOOT_SERVICE_CAPABILITY _capability;
    static BOOLEAN _firstTime = TRUE;
    
    if (_firstTime)
    {
        _firstTime = FALSE;

        if (TCG2_GetCapability(protocol, &_capability) != EFI_SUCCESS)
        {
            Memset(&_capability, 0, sizeof(_capability));
        }
    }

    return &_capability;
}

/*
**==============================================================================
**
** _SubmitCommand()
**
**==============================================================================
*/

TPM_RC _SubmitCommand(
    IN EFI_TCG2_PROTOCOL *protocol,
    TPMI_ST_COMMAND_TAG tag,
    TPM_CC commandCode,
    TPM_RC* responseCode,
    const TPMBuf* in,
    TPMBuf* out)
{
    TPMBuf buf;
    UINT32 outputParameterBlockSize;

    /* Pack command header response fields */
    {
        TPMBufInit(&buf);
        TPMBufPackU16(&buf, tag);
        TPMBufPackU32(&buf, 0);
        TPMBufPackU32(&buf, commandCode);
        TPMBufPack(&buf, in->data, in->size);
        UINT32 cmdSize = ByteSwapU32(buf.size);
        Memcpy(&buf.data[sizeof(UINT16)], &cmdSize, sizeof(cmdSize));

        if (buf.error)
            RETURN(TPM_RC_FAILURE);

#if 0
        HexDump(buf.data, buf.size);
#endif
    }

    /* Resovle 'outputParameterBlockSize' parameter to TCG2_SubmitCommand() */
    {
        const EFI_TCG2_BOOT_SERVICE_CAPABILITY* capability;
        
        capability = _TCG2GetCapabilities(protocol);
        outputParameterBlockSize = capability->MaxResponseSize;

        if (outputParameterBlockSize > 0)
        {
            /* Subtract one to workaround TianoCore bug */
            outputParameterBlockSize--;

            if (outputParameterBlockSize > out->cap)
                outputParameterBlockSize = out->cap;
        }
        else
        {
            outputParameterBlockSize = out->cap - 1;
        }
    }

    /* Submit the request */
    {
        EFI_STATUS status = TCG2_SubmitCommand(
            protocol,
            buf.size,
            buf.data,
            outputParameterBlockSize,
            out->data);

        if (status != EFI_SUCCESS)
            RETURN(TPM_RC_FAILURE);
    }

    /* Unpack the response */
    {
        TPMI_ST_COMMAND_TAG tagOut;
        UINT32 commandSize;

        out->size = sizeof(UINT16) + sizeof(UINT32) + sizeof(UINT32);
        TPMBufUnpackU16(out, &tagOut);
        TPMBufUnpackU32(out, &commandSize);
        TPMBufUnpackU32(out, responseCode);
        out->size = commandSize;

        if (out->error)
            RETURN(TPM_RC_FAILURE);

#if 0
        HexDump(out->data, out->size);
#endif
    }

    return TPM_RC_SUCCESS;
}

/*
**==============================================================================
**
** TPM2_GetCapability()
**
**     Wire request:
**         TPMI_ST_COMMAND_TAG tag
**         UINT32 commandSize
**         TPM_CC commandCode
**         =======================
**         TPM_CAP capability
**         UINT32 property
**         UINT32 propertyCount
**
**     Wire response:
**         TPM_ST tag
**         UINT32 responseSize
**         TPM_RC responseCode
**         =======================
**         TPMI_YES_NO moreData
**         TPMS_CAPABILITY_DATA capabilityData
**
**==============================================================================
*/

/*
    rval = Tss2_Sys_GetCapability(
        sysContext,
        NULL,
        TPM_CAP_TPM_PROPERTIES,
        TPM_PT_REVISION,
        1,
        NULL,
        &data,
        NULL);

    SEND:
    00000000 80 01 00 00 00 16 00 00 01 7A 00 00 00 06 00 00
    00000016 01 02 00 00 00 01

    RECV:
    00000000 80 01 00 00 00 1B 00 00 00 00 01 00 00 00 06 00
    00000016 00 00 01 00 00 01 02 00 00 00 67
*/

TPM_RC TPM2_GetCapability(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN const TPMS_AUTH_COMMAND *authCommand,
    IN TPM_CAP capability,
    IN UINT32 property,
    IN UINT32 propertyCount,
    OUT TPMI_YES_NO *moreData,
    OUT TPMS_CAPABILITY_DATA *capabilityData,
    OUT TPMS_AUTH_RESPONSE *authResponse)
{
    TPMBuf in;
    TPMBuf out;
    TPMI_YES_NO moreDataDummy;
    TPMS_CAPABILITY_DATA capabilityDataDummy;
    TPMS_AUTH_RESPONSE authResponseDummy;
    TPMI_ST_COMMAND_TAG tag = authCommand ? TPM_ST_SESSIONS:TPM_ST_NO_SESSIONS;

    /* Check input parameters */
    if (!protocol)
        RETURN(TPM_RC_FAILURE);

    /* Set null parameters to dummy variables */
    {
        if (!moreData)
            moreData = &moreDataDummy;

        if (!capabilityData)
            capabilityData = &capabilityDataDummy;

        if (!authResponse)
            authResponse = &authResponseDummy;
    }

    /* Pack the request body */
    {
        TPMBufInit(&in);
        TPMBufPackU32(&in, capability);
        TPMBufPackU32(&in, property);
        TPMBufPackU32(&in, propertyCount);

        if (in.error)
            RETURN(TPM_RC_FAILURE);
    }

    /* Submit the request */
    {
        TPM_RC responseCode;

        TPMBufInit(&out);
        TPM_RC rc = _SubmitCommand(
            protocol,
            tag,
            TPM_CC_GetCapability,
            &responseCode,
            &in,
            &out);

        if (rc != EFI_SUCCESS)
            RETURN(TPM_RC_FAILURE);

        if (responseCode != TPM_RC_SUCCESS)
            RETURN(responseCode);
    }

    /* Unpack the response */
    {
        TPM_CAP cap;
        UINT32 parameterSize;

        if (tag == TPM_ST_SESSIONS)
            TPMBufUnpackU32(&out, &parameterSize);

        TPMBufUnpackU8(&out, moreData);
        TPMBufUnpackU32(&out, &cap);

        if (out.error)
            RETURN(TPM_RC_FAILURE);

        if (cap == TPM_CAP_TPM_PROPERTIES)
        {
            _Unpack_TPMS_CAPABILITY_DATA_tpmProperties(&out,
                capabilityData);
        }
        else
        {
            /* Unsupported capability type */
            RETURN(TPM_RC_FAILURE);
        }

        if (tag == TPM_ST_SESSIONS)
            _Unpack_TPMS_AUTH_RESPONSE(&out, authResponse);

        if (out.error)
            RETURN(TPM_RC_FAILURE);
    }

    return TPM_RC_SUCCESS;
}

/*
**==============================================================================
**
** TPM2_Read_PCR()
**
**     Wire request:
**         TPMI_ST_COMMAND_TAG tag
**         UINT32 commandSize
**         TPM_CC commandCode
**         =======================
**         TPML_PCR_SELECTION pcrSelectionIn
**
**     Wire response:
**         TPM_ST tag
**         UINT32 responseSize
**         TPM_RC responseCode
**         =======================
**         UINT32 pcrUpdateCounter
**         TPML_PCR_SELECTION pcrSelectionOut
**         TPML_DIGEST pcrValues
**
**==============================================================================
*/

TPM_RC TPM2_PCR_Read(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN const TPMS_AUTH_COMMAND *authCommand,
    IN TPML_PCR_SELECTION  *pcrSelectionIn,
    OUT UINT32 *pcrUpdateCounter,
    OUT TPML_PCR_SELECTION *pcrSelectionOut,
    OUT TPML_DIGEST *pcrValues,
    OUT TPMS_AUTH_RESPONSE *authResponse)
{
    TPMBuf in;
    TPMBuf out;
    UINT32 pcrUpdateCounterDummy;
    TPML_PCR_SELECTION pcrSelectionOutDummy;
    TPML_DIGEST pcrValuesDummy;
    TPMS_AUTH_RESPONSE authResponseDummy;
    TPMI_ST_COMMAND_TAG tag = authCommand ? TPM_ST_SESSIONS:TPM_ST_NO_SESSIONS;

    /* Check input parameters */
    if (!protocol || !pcrSelectionIn)
        RETURN(TPM_RC_FAILURE);

    /* Set null parameters to refer to dummy variables */
    {
        if (!pcrUpdateCounter)
            pcrUpdateCounter = &pcrUpdateCounterDummy;

        if (!pcrSelectionOut)
            pcrSelectionOut = &pcrSelectionOutDummy;

        if (!pcrValues)
            pcrValues = &pcrValuesDummy;

        if (!authResponse)
            authResponse = &authResponseDummy;
    }

    /* Pack the request */
    {
        TPMBufInit(&in);
        _Pack_TPML_PCR_SELECTION(&in, pcrSelectionIn);

        if (in.error)
            RETURN(TPM_RC_FAILURE);
    }

    /* Submit the request */
    {
        TPM_RC responseCode;
        TPMBufInit(&out);

        TPM_RC rc = _SubmitCommand(
            protocol,
            tag,
            TPM_CC_PCR_Read,
            &responseCode,
            &in,
            &out);

        if (rc != EFI_SUCCESS)
            RETURN(TPM_RC_FAILURE);

        if (responseCode != TPM_RC_SUCCESS)
            RETURN(responseCode);
    }

    /* Unpack the response */
    {
        UINT32 parameterSize;

        if (tag == TPM_ST_SESSIONS)
            TPMBufUnpackU32(&out, &parameterSize);

        TPMBufUnpackU32(&out, pcrUpdateCounter);
        _Unpack_TPML_PCR_SELECTION(&out, pcrSelectionOut);
        _Unpack_TPML_DIGEST(&out, pcrValues);

        if (tag == TPM_ST_SESSIONS)
            _Unpack_TPMS_AUTH_RESPONSE(&out, authResponse);

        if (out.error)
            RETURN(TPM_RC_FAILURE);
    }

    return TPM_RC_SUCCESS;
}

/*
**==============================================================================
**
** TPM2_DictionaryAttackLockReset()
**
**     Wire request:
**         TPMI_ST_COMMAND_TAG tag
**         UINT32 commandSize
**         TPM_CC commandCode
**         =======================
**         TPMI_RH_LOCKOUT lockHandle
**
**     Wire response:
**         TPM_ST tag
**         UINT32 responseSize
**         TPM_RC responseCode
**         =======================
**
**      SEND:
**          00000000 80 02 00 00 00 1B 00 00 01 39 40 00 00 0A 00 00
**          00000016 00 09 40 00 00 09 00 00 00 00 00
**
**      RECV:
**          00000000 80 02 00 00 00 13 00 00 00 00 00 00 00 00 00 00
**          00000016 01 00 00
**
**          00000000 tag: 80 02
**          responseSize: 00 00 00 13 
**          responseCode: 00 00 00 00 
**          sizeField: 00 00 00 00 
**          nonce: 00 00
**          sessionAttributes: 01 
**          hmac: 00 00
**
**==============================================================================
*/

TPM_RC TPM2_DictionaryAttackLockReset(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_RH_LOCKOUT lockHandle,
    IN const TPMS_AUTH_COMMAND *authCommand,
    OUT TPMS_AUTH_RESPONSE *authResponse)
{
    TPMBuf in;
    TPMBuf out;
    TPMS_AUTH_RESPONSE authResponseDummy;
    TPMI_ST_COMMAND_TAG tag = authCommand ? TPM_ST_SESSIONS:TPM_ST_NO_SESSIONS;

    /* Check input parameters */
    if (!protocol)
        RETURN(TPM_RC_FAILURE);

    /* Set null parameters to refer to dummy */
    if (!authResponse)
        authResponse = &authResponseDummy;

    /* Pack the request */
    {
        TPMBufInit(&in);
        TPMBufPackU32(&in, lockHandle);

        if (authCommand)
            _Pack_TPMS_AUTH_COMMAND(&in, authCommand);

        if (in.error)
            RETURN(TPM_RC_FAILURE);
    }

    /* Submit the request */
    {
        TPM_RC responseCode;
        TPMBufInit(&out);

        TPM_RC rc = _SubmitCommand(
            protocol,
            tag,
            TPM_CC_DictionaryAttackLockReset,
            &responseCode,
            &in,
            &out);

        if (rc != EFI_SUCCESS)
            RETURN(TPM_RC_FAILURE);

        if (responseCode != TPM_RC_SUCCESS)
            RETURN(responseCode);
    }

    /* Unpack the response */
    {
        UINT32 parameterSize;

        if (tag == TPM_ST_SESSIONS)
            TPMBufUnpackU32(&out, &parameterSize);

        if (tag == TPM_ST_SESSIONS)
            _Unpack_TPMS_AUTH_RESPONSE(&out, authResponse);

        if (out.error)
            RETURN(TPM_RC_FAILURE);
    }

    return TPM_RC_SUCCESS;
}

/*
**==============================================================================
**
** TPM2_DictionaryAttackLockReset()
**
**     Wire request:
**         TPMI_ST_COMMAND_TAG tag
**         UINT32 commandSize
**         TPM_CC commandCode
**         =======================
**         TPMI_RH_LOCKOUT lockHandle
**         UINT32 newMaxTries
**         UINT32 newRecoveryTime
**         UINT32 lockoutRecovery
**
**     Wire response:
**         TPM_ST tag
**         UINT32 responseSize
**         TPM_RC responseCode
**         =======================
**
**     SEND:
**         00000000 80 02 00 00 00 27 00 00 01 3A 40 00 00 0A 00 00 
**         00000016 00 09 40 00 00 09 00 00 00 00 00 00 0F 42 40 00 
**         00000032 00 00 01 00 00 00 01 
**         
**     RECV:
**         00000000 80 02 00 00 00 13 00 00 00 00 00 00 00 00 00 00 
**         00000016 01 00 00 
**
**==============================================================================
*/

TPM_RC TPM2_DictionaryAttackParameters(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_RH_LOCKOUT lockHandle,
    IN const TPMS_AUTH_COMMAND *authCommand,
    IN UINT32 newMaxTries,
    IN UINT32 newRecoveryTime,
    IN UINT32 lockoutRecovery,
    OUT TPMS_AUTH_RESPONSE *authResponse)
{
    TPMBuf in;
    TPMBuf out;
    TPMS_AUTH_RESPONSE authResponseDummy;
    TPMI_ST_COMMAND_TAG tag = authCommand ? TPM_ST_SESSIONS:TPM_ST_NO_SESSIONS;

    /* Check input parameters */
    if (!protocol)
        RETURN(TPM_RC_FAILURE);

    /* Set null parameters to refer to dummy */
    if (!authResponse)
        authResponse = &authResponseDummy;

    /* Pack the request */
    {
        TPMBufInit(&in);
        TPMBufPackU32(&in, lockHandle);

        if (authCommand)
            _Pack_TPMS_AUTH_COMMAND(&in, authCommand);

        TPMBufPackU32(&in, newMaxTries);
        TPMBufPackU32(&in, newRecoveryTime);
        TPMBufPackU32(&in, lockoutRecovery);

        if (in.error)
            RETURN(TPM_RC_FAILURE);
    }

    /* Submit the request */
    {
        TPM_RC responseCode;
        TPMBufInit(&out);

        TPM_RC rc = _SubmitCommand(
            protocol,
            tag,
            TPM_CC_DictionaryAttackParameters,
            &responseCode,
            &in,
            &out);

        if (rc != EFI_SUCCESS)
            RETURN(TPM_RC_FAILURE);

        if (responseCode != TPM_RC_SUCCESS)
            RETURN(responseCode);
    }

    /* Unpack the response */
    {
        UINT32 parameterSize;

        if (tag == TPM_ST_SESSIONS)
            TPMBufUnpackU32(&out, &parameterSize);

        if (tag == TPM_ST_SESSIONS)
            _Unpack_TPMS_AUTH_RESPONSE(&out, authResponse);

        if (out.error)
            RETURN(TPM_RC_FAILURE);
    }

    return TPM_RC_SUCCESS;
}

/*
**==============================================================================
**
** TPM2_CreatePrimary()
**
**     Wire request:
**         TPMI_ST_COMMAND_TAG tag
**         UINT32 commandSize
**         TPM_CC commandCode
**         --------------------------------------
**         TPMI_RH_HIERARCHY primaryHandle
**         ======================================
**         TPM2B_SENSITIVE_CREATE inSensitive
**         TPM2B_PUBLIC inPublic
**         TPM2B_DATA outsideInfo
**         TPML_PCR_SELECTION creationPCR
**
**     Wire response:
**         TPM_ST tag
**         UINT32 responseSize
**         TPM_RC responseCode
**         --------------------------------------
**         TPM_HANDLE objectHandle
**         ======================================
**         TPM2B_PUBLIC outPublic
**         TPM2B_CREATION_DATA creationData
**         TPM2B_DIGEST creationHash
**         TPMT_TK_CREATION creationTicket
**         TPM2B_NAME name
**
**==============================================================================
*/

TPM_RC TPM2_CreatePrimary(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_RH_HIERARCHY primaryHandle,
    IN const TPMS_AUTH_COMMAND *authCommand,
    IN TPM2B_SENSITIVE_CREATE *inSensitive,
    IN TPM2B_PUBLIC *inPublic,
    IN TPM2B_DATA *outsideInfo,
    IN TPML_PCR_SELECTION *creationPCR,
    OUT TPM_HANDLE *objectHandle,
    OUT TPM2B_PUBLIC *outPublic,
    OUT TPM2B_CREATION_DATA *creationData,
    OUT TPM2B_DIGEST *creationHash,
    OUT TPMT_TK_CREATION *creationTicket,
    OUT TPM2B_NAME *name,
    OUT TPMS_AUTH_RESPONSE *authResponse)
{
    TPMBuf in;
    TPMBuf out;
    TPM_HANDLE objectHandleDummy;
    TPM2B_PUBLIC outPublicDummy;
    TPM2B_CREATION_DATA creationDataDummy;
    TPM2B_DIGEST creationHashDummy;
    TPMT_TK_CREATION creationTicketDummy;
    TPM2B_NAME nameDummy;
    TPMS_AUTH_RESPONSE authResponseDummy;
    TPMI_ST_COMMAND_TAG tag = authCommand ? TPM_ST_SESSIONS:TPM_ST_NO_SESSIONS;

    /* Check input parameters */
    if (!protocol)
        RETURN(TPM_RC_FAILURE);

    /* Set null parameters to refer to dummy */
    {
        if (!objectHandle)
            objectHandle = &objectHandleDummy;

        if (!outPublic)
            outPublic = &outPublicDummy;

        if (!creationData)
            creationData = &creationDataDummy;

        if (!creationHash)
            creationHash = &creationHashDummy;

        if (!creationTicket)
            creationTicket = &creationTicketDummy;

        if (!name)
            name = &nameDummy;

        if (!authResponse)
            authResponse = &authResponseDummy;
    }

    /* Clear the output buffers */
    Memset(outPublic, 0, sizeof(*outPublic));
    Memset(creationData, 0, sizeof(*creationData));
    Memset(creationHash, 0, sizeof(*creationHash));
    Memset(creationTicket, 0, sizeof(*creationTicket));
    Memset(name, 0, sizeof(*name));
    Memset(authResponse, 0, sizeof(*authResponse));

    /* Pack the request */
    {
        TPMBufInit(&in);
        TPMBufPackU32(&in, primaryHandle);

        if (authCommand)
            _Pack_TPMS_AUTH_COMMAND(&in, authCommand);

        _Pack_TPM2B_SENSITIVE_CREATE(&in, inSensitive);
        _Pack_TPM2B_PUBLIC(&in, inPublic);
        _Pack_TPM2B(&in, outsideInfo->size, outsideInfo->buffer);
        _Pack_TPML_PCR_SELECTION(&in, creationPCR);

        if (in.error)
            RETURN(TPM_RC_FAILURE);
    }

    /* Submit the request */
    {
        TPM_RC responseCode;
        TPMBufInit(&out);

        TPM_RC rc = _SubmitCommand(
            protocol,
            tag,
            TPM_CC_CreatePrimary,
            &responseCode,
            &in,
            &out);

        if (rc != EFI_SUCCESS)
            RETURN(TPM_RC_FAILURE);

        if (responseCode != TPM_RC_SUCCESS)
            RETURN(responseCode);
    }

    /* Unpack the response */
    {
        UINT32 parameterSize;

        /* Unpack handles */
        TPMBufUnpackU32(&out, objectHandle);

        /* Unpack parameters */
        {
            if (tag == TPM_ST_SESSIONS)
                TPMBufUnpackU32(&out, &parameterSize);

            _Unpack_TPM2B_PUBLIC(&out, outPublic);
            _Unpack_TPM2B(&out, (TPM2B*)creationData);
            _Unpack_TPM2B(&out, (TPM2B*)creationHash);
            _Unpack_TPMT_TK_CREATION(&out, creationTicket);
            _Unpack_TPM2B(&out, (TPM2B*)name);

            if (tag == TPM_ST_SESSIONS)
                _Unpack_TPMS_AUTH_RESPONSE(&out, authResponse);

            if (out.error)
                RETURN(TPM_RC_FAILURE);
        }
    }

    return TPM_RC_SUCCESS;
}

/*
**==============================================================================
**
** TPM2_FlushContext()
**
**     Wire request:
**         TPMI_ST_COMMAND_TAG tag
**         UINT32 commandSize
**         TPM_CC commandCode
**         ======================================
**         TPMI_DH_CONTEXT flushHandle
**
**     Wire response:
**         TPM_ST tag
**         UINT32 responseSize
**         TPM_RC responseCode
**         ======================================
**
**==============================================================================
*/

TPM_RC TPM2_FlushContext(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_DH_CONTEXT flushHandle)
{
    TPMBuf in;
    TPMBuf out;

    /* Check input parameters */
    if (!protocol)
        RETURN(TPM_RC_FAILURE);

    /* Pack the request */
    {
        TPMBufInit(&in);
        TPMBufPackU32(&in, flushHandle);

        if (in.error)
            RETURN(TPM_RC_FAILURE);
    }

    /* Submit the request */
    {
        TPM_RC responseCode;
        TPMBufInit(&out);

        TPM_RC rc = _SubmitCommand(
            protocol,
            TPM_ST_NO_SESSIONS,
            TPM_CC_FlushContext,
            &responseCode,
            &in,
            &out);

        if (rc != EFI_SUCCESS)
            RETURN(TPM_RC_FAILURE);

        if (responseCode != TPM_RC_SUCCESS)
            RETURN(responseCode);
    }

    /* Unpack the response */
    {
        if (out.error)
            RETURN(TPM_RC_FAILURE);
    }

    return TPM_RC_SUCCESS;
}

/*
**==============================================================================
**
** TPM2_StartAuthSession()
**
**==============================================================================
*/

TPM_RC TPM2_StartAuthSession(
    IN EFI_TCG2_PROTOCOL *protocol,
    /* request handles */
    IN TPMI_DH_OBJECT tpmKey,
    IN TPMI_DH_ENTITY bind,
    /* request parameters */
    IN const TPMS_AUTH_COMMAND *authCommand,
    IN TPM2B_NONCE *nonceCaller,
    IN TPM2B_ENCRYPTED_SECRET *encryptedSalt,
    IN TPM_SE sessionType,
    IN TPMT_SYM_DEF *symmetric,
    IN TPMI_ALG_HASH authHash,
    /* response handles */
    OUT TPMI_SH_AUTH_SESSION *sessionHandle,
    /* response parameters */
    OUT TPM2B_NONCE *nonceTPM,
    OUT TPMS_AUTH_RESPONSE *authResponse)
{
    TPMBuf in;
    TPMBuf out;
    TPMI_SH_AUTH_SESSION sessionHandleDummy;
    TPM2B_NONCE nonceTPMDummy;
    TPMS_AUTH_RESPONSE authResponseDummy;
    TPMI_ST_COMMAND_TAG tag = authCommand ? TPM_ST_SESSIONS:TPM_ST_NO_SESSIONS;

    /* Check input parameters */
    if (!protocol)
        RETURN(TPM_RC_FAILURE);

    /* Set null parameters to refer to dummy */
    {
        if (!sessionHandle)
            sessionHandle = &sessionHandleDummy;

        if (!nonceTPM)
            nonceTPM = &nonceTPMDummy;

        if (!authResponse)
            authResponse = &authResponseDummy;
    }

    /* Clear the output buffers */
    Memset(sessionHandle, 0, sizeof(*sessionHandle));
    Memset(nonceTPM, 0, sizeof(*nonceTPM));
    Memset(authResponse, 0, sizeof(*authResponse));

    /* Pack the request */
    {
        TPMBufInit(&in);

        TPMBufPackU32(&in, tpmKey);
        TPMBufPackU32(&in, bind);

        if (tag == TPM_ST_SESSIONS)
            _Pack_TPMS_AUTH_COMMAND(&in, authCommand);

        _Pack_TPM2B(&in, nonceCaller->size, nonceCaller->buffer);
        _Pack_TPM2B(&in, encryptedSalt->size, encryptedSalt->secret);
        TPMBufPackU8(&in, sessionType);
        _Pack_TPMT_SYM_DEF(&in, symmetric);
        TPMBufPackU16(&in, authHash);

        if (in.error)
            RETURN(TPM_RC_FAILURE);
    }

    /* Submit the request */
    {
        TPM_RC responseCode;
        TPMBufInit(&out);

        TPM_RC rc = _SubmitCommand(
            protocol,
            tag,
            TPM_CC_StartAuthSession,
            &responseCode,
            &in,
            &out);

        if (rc != EFI_SUCCESS)
            RETURN(TPM_RC_FAILURE);

        if (responseCode != TPM_RC_SUCCESS)
            RETURN(responseCode);
    }

    /* Unpack the response */
    {
        UINT32 parameterSize;

        TPMBufUnpackU32(&out, sessionHandle);

        if (tag == TPM_ST_SESSIONS)
            TPMBufUnpackU32(&out, &parameterSize);

        _Unpack_TPM2B(&out, (TPM2B*)nonceTPM);

        if (tag == TPM_ST_SESSIONS)
            _Unpack_TPMS_AUTH_RESPONSE(&out, authResponse);

        if (out.error)
            RETURN(TPM_RC_FAILURE);
    }

    return TPM_RC_SUCCESS;
}

/*
**==============================================================================
**
** TPM2_PolicyPassword()
**
**==============================================================================
*/

TPM_RC TPM2_PolicyPassword(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_SH_POLICY policySession,
    IN const TPMS_AUTH_COMMAND *authCommand,
    OUT TPMS_AUTH_RESPONSE *authResponse)
{
    TPMBuf in;
    TPMBuf out;
    TPMS_AUTH_RESPONSE authResponseDummy;
    TPMI_ST_COMMAND_TAG tag = authCommand ? TPM_ST_SESSIONS:TPM_ST_NO_SESSIONS;

    /* Check input parameters */
    if (!protocol)
        RETURN(TPM_RC_FAILURE);

    /* Set null parameters to refer to dummy */
    {
        if (!authResponse)
            authResponse = &authResponseDummy;
    }

    /* Clear the output buffers */
    Memset(authResponse, 0, sizeof(*authResponse));

    /* Pack the request */
    {
        TPMBufInit(&in);

        TPMBufPackU32(&in, policySession);

        if (tag == TPM_ST_SESSIONS)
            _Pack_TPMS_AUTH_COMMAND(&in, authCommand);

        if (in.error)
            RETURN(TPM_RC_FAILURE);
    }

    /* Submit the request */
    {
        TPM_RC responseCode;
        TPMBufInit(&out);

        TPM_RC rc = _SubmitCommand(
            protocol,
            tag,
            TPM_CC_PolicyPassword,
            &responseCode,
            &in,
            &out);

        if (rc != EFI_SUCCESS)
            RETURN(TPM_RC_FAILURE);

        if (responseCode != TPM_RC_SUCCESS)
            RETURN(responseCode);
    }

    /* Unpack the response */
    {
        UINT32 parameterSize;

        if (tag == TPM_ST_SESSIONS)
            TPMBufUnpackU32(&out, &parameterSize);

        if (tag == TPM_ST_SESSIONS)
            _Unpack_TPMS_AUTH_RESPONSE(&out, authResponse);

        if (out.error)
            RETURN(TPM_RC_FAILURE);
    }

    return TPM_RC_SUCCESS;
}

/*
**==============================================================================
**
** TPM2_PolicyPCR()
**
**==============================================================================
*/

TPM_RC TPM2_PolicyPCR(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_SH_POLICY policySession,
    IN const TPMS_AUTH_COMMAND *authCommand,
    IN TPM2B_DIGEST *pcrDigest,
    IN TPML_PCR_SELECTION *pcrs,
    OUT TPMS_AUTH_RESPONSE *authResponse)
{
    TPMBuf in;
    TPMBuf out;
    TPMS_AUTH_RESPONSE authResponseDummy;
    TPMI_ST_COMMAND_TAG tag = authCommand ? TPM_ST_SESSIONS:TPM_ST_NO_SESSIONS;

    /* Check input parameters */
    if (!protocol)
        RETURN(TPM_RC_FAILURE);

    /* Set null parameters to refer to dummy */
    {
        if (!authResponse)
            authResponse = &authResponseDummy;
    }

    /* Clear the output buffers */
    Memset(authResponse, 0, sizeof(*authResponse));

    /* Pack the request */
    {
        TPMBufInit(&in);

        TPMBufPackU32(&in, policySession);

        if (tag == TPM_ST_SESSIONS)
            _Pack_TPMS_AUTH_COMMAND(&in, authCommand);

        _Pack_TPM2B(&in, pcrDigest->size, pcrDigest->buffer);
        _Pack_TPML_PCR_SELECTION(&in, pcrs);

        if (in.error)
            RETURN(TPM_RC_FAILURE);
    }

    /* Submit the request */
    {
        TPM_RC responseCode;
        TPMBufInit(&out);

        TPM_RC rc = _SubmitCommand(
            protocol,
            tag,
            TPM_CC_PolicyPCR,
            &responseCode,
            &in,
            &out);

        if (rc != EFI_SUCCESS)
            RETURN(TPM_RC_FAILURE);

        if (responseCode != TPM_RC_SUCCESS)
            RETURN(responseCode);
    }

    /* Unpack the response */
    {
        UINT32 parameterSize;

        if (tag == TPM_ST_SESSIONS)
            TPMBufUnpackU32(&out, &parameterSize);

        if (tag == TPM_ST_SESSIONS)
            _Unpack_TPMS_AUTH_RESPONSE(&out, authResponse);

        if (out.error)
            RETURN(TPM_RC_FAILURE);
    }

    return TPM_RC_SUCCESS;
}

/*
**==============================================================================
**
** TPM2_Hash()
**
**==============================================================================
*/

TPM_RC TPM2_Hash(
    IN EFI_TCG2_PROTOCOL *protocol,
    const TPMS_AUTH_COMMAND* authCommand,
    IN TPM2B_MAX_BUFFER *data,
    IN TPMI_ALG_HASH hashAlg,
    IN TPMI_RH_HIERARCHY hierarchy,
    OUT TPM2B_DIGEST *outHash,
    OUT TPMT_TK_HASHCHECK *validation,
    OUT TPMS_AUTH_RESPONSE *authResponse)
{
    TPMBuf in;
    TPMBuf out;
    OUT TPM2B_DIGEST outHashDummy;
    OUT TPMT_TK_HASHCHECK validationDummy;
    TPMS_AUTH_RESPONSE authResponseDummy;
    TPMI_ST_COMMAND_TAG tag = authCommand ? TPM_ST_SESSIONS:TPM_ST_NO_SESSIONS;

    /* Check input parameters */
    if (!protocol)
        RETURN(TPM_RC_FAILURE);

    /* Set null parameters to refer to dummy */
    {
        if (!authResponse)
            authResponse = &authResponseDummy;

        if (!outHash)
            outHash = &outHashDummy;

        if (!validation)
            validation = &validationDummy;
    }

    /* Clear the output buffers */
    Memset(authResponse, 0, sizeof(*authResponse));
    Memset(outHash, 0, sizeof(*outHash));
    Memset(validation, 0, sizeof(*validation));

    /* Pack the request */
    {
        TPMBufInit(&in);

        if (tag == TPM_ST_SESSIONS)
            _Pack_TPMS_AUTH_COMMAND(&in, authCommand);

        _Pack_TPM2B(&in, data->size, data->buffer);
        TPMBufPackU16(&in, hashAlg);
        TPMBufPackU32(&in, hierarchy);

        if (in.error)
            RETURN(TPM_RC_FAILURE);
    }

    /* Submit the request */
    {
        TPM_RC responseCode;
        TPMBufInit(&out);

        TPM_RC rc = _SubmitCommand(
            protocol,
            tag,
            TPM_CC_Hash,
            &responseCode,
            &in,
            &out);

        if (rc != EFI_SUCCESS)
            RETURN(TPM_RC_FAILURE);

        if (responseCode != TPM_RC_SUCCESS)
            RETURN(responseCode);
    }

    /* Unpack the response */
    {
        UINT32 parameterSize;

        if (tag == TPM_ST_SESSIONS)
            TPMBufUnpackU32(&out, &parameterSize);

        _Unpack_TPM2B(&out, (TPM2B*)outHash);
        _Unpack_TPMT_TK_HASHCHECK(&out, validation);

        if (tag == TPM_ST_SESSIONS)
            _Unpack_TPMS_AUTH_RESPONSE(&out, authResponse);

        if (out.error)
            RETURN(TPM_RC_FAILURE);
    }

    return TPM_RC_SUCCESS;
}

/*
**==============================================================================
**
** TPM2_PolicyGetDigest()
**
**==============================================================================
*/

TPM_RC TPM2_PolicyGetDigest(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_SH_POLICY policySession,
    IN const TPMS_AUTH_COMMAND *authCommand,
    OUT TPM2B_DIGEST *policyDigest,
    OUT TPMS_AUTH_RESPONSE *authResponse)
{
    TPMBuf in;
    TPMBuf out;
    TPMS_AUTH_RESPONSE authResponseDummy;
    TPM2B_DIGEST policyDigestDummy;
    TPMI_ST_COMMAND_TAG tag = authCommand ? TPM_ST_SESSIONS:TPM_ST_NO_SESSIONS;

    /* Check input parameters */
    if (!protocol)
        RETURN(TPM_RC_FAILURE);

    /* Set null parameters to refer to dummy */
    {
        if (!authResponse)
            authResponse = &authResponseDummy;

        if (!policyDigest)
            policyDigest = &policyDigestDummy;
    }

    /* Clear the output buffers */
    Memset(authResponse, 0, sizeof(*authResponse));
    Memset(policyDigest, 0, sizeof(*policyDigest));

    /* Pack the request */
    {
        TPMBufInit(&in);

        TPMBufPackU32(&in, policySession);

        if (tag == TPM_ST_SESSIONS)
            _Pack_TPMS_AUTH_COMMAND(&in, authCommand);

        if (in.error)
            RETURN(TPM_RC_FAILURE);
    }

    /* Submit the request */
    {
        TPM_RC responseCode;
        TPMBufInit(&out);

        TPM_RC rc = _SubmitCommand(
            protocol,
            tag,
            TPM_CC_PolicyGetDigest,
            &responseCode,
            &in,
            &out);

        if (rc != EFI_SUCCESS)
            RETURN(TPM_RC_FAILURE);

        if (responseCode != TPM_RC_SUCCESS)
            RETURN(responseCode);
    }

    /* Unpack the response */
    {
        UINT32 parameterSize;

        if (tag == TPM_ST_SESSIONS)
            TPMBufUnpackU32(&out, &parameterSize);

        _Unpack_TPM2B(&out, (TPM2B*)policyDigest);

        if (tag == TPM_ST_SESSIONS)
            _Unpack_TPMS_AUTH_RESPONSE(&out, authResponse);

        if (out.error)
            RETURN(TPM_RC_FAILURE);
    }

    return TPM_RC_SUCCESS;
}

/*
**==============================================================================
**
** TPM2_Create()
**
**==============================================================================
*/

TPM_RC TPM2_Create(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_DH_OBJECT parentHandle,
    IN const TPMS_AUTH_COMMAND *authCommand,
    IN TPM2B_SENSITIVE_CREATE *inSensitive,
    IN TPM2B_PUBLIC *inPublic,
    IN TPM2B_DATA *outsideInfo,
    IN TPML_PCR_SELECTION *creationPCR,
    OUT TPM2B_PRIVATE *outPrivate,
    OUT TPM2B_PUBLIC *outPublic,
    OUT TPM2B_CREATION_DATA *creationData,
    OUT TPM2B_DIGEST *creationHash,
    OUT TPMT_TK_CREATION *creationTicket,
    OUT TPMS_AUTH_RESPONSE *authResponse)
{
    TPMBuf in;
    TPMBuf out;
    TPM2B_PUBLIC outPublicDummy;
    TPM2B_PRIVATE outPrivateDummy;
    TPM2B_CREATION_DATA creationDataDummy;
    TPM2B_DIGEST creationHashDummy;
    TPMT_TK_CREATION creationTicketDummy;
    TPMS_AUTH_RESPONSE authResponseDummy;
    TPMI_ST_COMMAND_TAG tag = authCommand ? TPM_ST_SESSIONS:TPM_ST_NO_SESSIONS;

    /* Check input parameters */
    if (!protocol)
        RETURN(TPM_RC_FAILURE);

    /* Set null parameters to refer to dummy */
    {
        if (!outPrivate)
            outPrivate = &outPrivateDummy;

        if (!outPublic)
            outPublic = &outPublicDummy;

        if (!creationData)
            creationData = &creationDataDummy;

        if (!creationHash)
            creationHash = &creationHashDummy;

        if (!creationTicket)
            creationTicket = &creationTicketDummy;

        if (!authResponse)
            authResponse = &authResponseDummy;
    }

    /* Clear the output buffers */
    Memset(outPrivate, 0, sizeof(*outPrivate));
    Memset(outPublic, 0, sizeof(*outPublic));
    Memset(creationData, 0, sizeof(*creationData));
    Memset(creationHash, 0, sizeof(*creationHash));
    Memset(creationTicket, 0, sizeof(*creationTicket));
    Memset(authResponse, 0, sizeof(*authResponse));

    /* Pack the request */
    {
        TPMBufInit(&in);

        TPMBufPackU32(&in, parentHandle);

        if (authCommand)
            _Pack_TPMS_AUTH_COMMAND(&in, authCommand);

        _Pack_TPM2B_SENSITIVE_CREATE(&in, inSensitive);
        _Pack_TPM2B_PUBLIC(&in, inPublic);
        _Pack_TPM2B(&in, outsideInfo->size, outsideInfo->buffer);
        _Pack_TPML_PCR_SELECTION(&in, creationPCR);

        if (in.error)
            RETURN(TPM_RC_FAILURE);
    }

    /* Submit the request */
    {
        TPM_RC responseCode;
        TPMBufInit(&out);

        TPM_RC rc = _SubmitCommand(
            protocol,
            tag,
            TPM_CC_Create,
            &responseCode,
            &in,
            &out);

        if (rc != EFI_SUCCESS)
            RETURN(TPM_RC_FAILURE);

        if (responseCode != TPM_RC_SUCCESS)
            RETURN(responseCode);
    }

    /* Unpack the response */
    {
        UINT32 parameterSize;

        /* Unpack handles */

        /* Unpack parameters */
        {
            if (tag == TPM_ST_SESSIONS)
                TPMBufUnpackU32(&out, &parameterSize);

            _Unpack_TPM2B(&out, (TPM2B*)outPrivate);
            _Unpack_TPM2B_PUBLIC(&out, outPublic);
            _Unpack_TPM2B(&out, (TPM2B*)creationData);
            _Unpack_TPM2B(&out, (TPM2B*)creationHash);
            _Unpack_TPMT_TK_CREATION(&out, creationTicket);

            if (tag == TPM_ST_SESSIONS)
                _Unpack_TPMS_AUTH_RESPONSE(&out, authResponse);

            if (out.error)
                RETURN(TPM_RC_FAILURE);
        }
    }

    return TPM_RC_SUCCESS;
}

/*
**==============================================================================
**
** TPM2_Load()
**
**==============================================================================
*/

TPM_RC TPM2_Load(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_DH_OBJECT parentHandle,
    IN TPMS_AUTH_COMMAND const *authCommand,
    IN TPM2B_PRIVATE *inPrivate,
    IN TPM2B_PUBLIC *inPublic,
    OUT TPM_HANDLE *objectHandle,
    OUT TPM2B_NAME *name,
    OUT TPMS_AUTH_RESPONSE *authResponse)
{
    TPMBuf in;
    TPMBuf out;
    TPM_HANDLE objectHandleDummy;
    TPM2B_NAME nameDummy;
    TPMS_AUTH_RESPONSE authResponseDummy;
    TPMI_ST_COMMAND_TAG tag = authCommand ? TPM_ST_SESSIONS:TPM_ST_NO_SESSIONS;

    /* Check input parameters */
    if (!protocol)
        RETURN(TPM_RC_FAILURE);

    /* Set null parameters to refer to dummy */
    {
        if (!objectHandle)
            objectHandle = &objectHandleDummy;

        if (!name)
            name = &nameDummy;

        if (!authResponse)
            authResponse = &authResponseDummy;
    }

    /* Clear the output buffers */
    Memset(objectHandle, 0, sizeof(*objectHandle));
    Memset(name, 0, sizeof(*name));
    Memset(authResponse, 0, sizeof(*authResponse));

    /* Pack the request */
    {
        TPMBufInit(&in);

        TPMBufPackU32(&in, parentHandle);

        if (authCommand)
            _Pack_TPMS_AUTH_COMMAND(&in, authCommand);

        _Pack_TPM2B(&in, inPrivate->size, inPrivate->buffer);
        _Pack_TPM2B_PUBLIC(&in, inPublic);

        if (in.error)
            RETURN(TPM_RC_FAILURE);
    }

    /* Submit the request */
    {
        TPM_RC responseCode;
        TPMBufInit(&out);

        TPM_RC rc = _SubmitCommand(
            protocol,
            tag,
            TPM_CC_Load,
            &responseCode,
            &in,
            &out);

        if (rc != EFI_SUCCESS)
            RETURN(TPM_RC_FAILURE);

        if (responseCode != TPM_RC_SUCCESS)
            RETURN(responseCode);
    }

    /* Unpack the response */
    {
        UINT32 parameterSize;

        /* Unpack handles */
        TPMBufUnpackU32(&out, objectHandle);

        /* Unpack parameters */
        {
            if (tag == TPM_ST_SESSIONS)
                TPMBufUnpackU32(&out, &parameterSize);

            _Unpack_TPM2B(&out, (TPM2B*)name);

            if (tag == TPM_ST_SESSIONS)
                _Unpack_TPMS_AUTH_RESPONSE(&out, authResponse);

            if (out.error)
                RETURN(TPM_RC_FAILURE);
        }
    }

    return TPM_RC_SUCCESS;
}

/*
**==============================================================================
**
** TPM2_Unseal()
**
**==============================================================================
*/

TPM_RC TPM2_Unseal(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_DH_OBJECT itemHandle,
    IN const TPMS_AUTH_COMMAND *authCommand,
    OUT TPM2B_SENSITIVE_DATA *outData,
    OUT TPMS_AUTH_RESPONSE *authResponse)
{
    TPMBuf in;
    TPMBuf out;
    TPM2B_SENSITIVE_DATA outDataDummy;
    TPMS_AUTH_RESPONSE authResponseDummy;
    TPMI_ST_COMMAND_TAG tag = authCommand ? TPM_ST_SESSIONS:TPM_ST_NO_SESSIONS;

    /* Check input parameters */
    if (!protocol)
        RETURN(TPM_RC_FAILURE);

    /* Set null parameters to refer to dummy */
    {
        if (!outData)
            outData = &outDataDummy;

        if (!authResponse)
            authResponse = &authResponseDummy;
    }

    /* Clear the output buffers */
    Memset(outData, 0, sizeof(*outData));
    Memset(authResponse, 0, sizeof(*authResponse));

    /* Pack the request */
    {
        TPMBufInit(&in);

        TPMBufPackU32(&in, itemHandle);

        if (authCommand)
            _Pack_TPMS_AUTH_COMMAND(&in, authCommand);

        if (in.error)
            RETURN(TPM_RC_FAILURE);
    }

    /* Submit the request */
    {
        TPM_RC responseCode;
        TPMBufInit(&out);

        TPM_RC rc = _SubmitCommand(
            protocol,
            tag,
            TPM_CC_Unseal,
            &responseCode,
            &in,
            &out);

        if (rc != EFI_SUCCESS)
            RETURN(TPM_RC_FAILURE);

        if (responseCode != TPM_RC_SUCCESS)
            RETURN(responseCode);
    }

    /* Unpack the response */
    {
        UINT32 parameterSize;

        /* Unpack handles */

        /* Unpack parameters */
        {
            if (tag == TPM_ST_SESSIONS)
                TPMBufUnpackU32(&out, &parameterSize);

            _Unpack_TPM2B(&out, (TPM2B*)outData);

            if (tag == TPM_ST_SESSIONS)
                _Unpack_TPMS_AUTH_RESPONSE(&out, authResponse);

            if (out.error)
                RETURN(TPM_RC_FAILURE);
        }
    }

    return TPM_RC_SUCCESS;
}

/*
**==============================================================================
**
** TPM2_PCR_Extend()
**
**==============================================================================
*/

TPM_RC TPM2_PCR_Extend(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_DH_PCR pcrHandle,
    IN const TPMS_AUTH_COMMAND *authCommand,
    IN TPML_DIGEST_VALUES *digests,
    OUT TPMS_AUTH_RESPONSE *authResponse)
{
    TPMBuf in;
    TPMBuf out;
    TPMS_AUTH_RESPONSE authResponseDummy;
    TPMI_ST_COMMAND_TAG tag = authCommand ? TPM_ST_SESSIONS:TPM_ST_NO_SESSIONS;

    /* Check input parameters */
    if (!protocol)
        RETURN(TPM_RC_FAILURE);

    /* Set null parameters to refer to dummy */
    {
        if (!authResponse)
            authResponse = &authResponseDummy;
    }

    /* Clear the output buffers */
    Memset(authResponse, 0, sizeof(*authResponse));

    /* Pack the request */
    {
        TPMBufInit(&in);

        TPMBufPackU32(&in, pcrHandle);

        if (authCommand)
            _Pack_TPMS_AUTH_COMMAND(&in, authCommand);

        _Pack_TPML_DIGEST_VALUES(&in, digests);

        if (in.error)
            RETURN(TPM_RC_FAILURE);
    }

    /* Submit the request */
    {
        TPM_RC responseCode;
        TPMBufInit(&out);

        TPM_RC rc = _SubmitCommand(
            protocol,
            tag,
            TPM_CC_PCR_Extend,
            &responseCode,
            &in,
            &out);

        if (rc != EFI_SUCCESS)
            RETURN(TPM_RC_FAILURE);

        if (responseCode != TPM_RC_SUCCESS)
            RETURN(responseCode);
    }

    /* Unpack the response */
    {
        UINT32 parameterSize;

        /* Unpack handles */

        /* Unpack parameters */
        {
            if (tag == TPM_ST_SESSIONS)
                TPMBufUnpackU32(&out, &parameterSize);

            if (tag == TPM_ST_SESSIONS)
                _Unpack_TPMS_AUTH_RESPONSE(&out, authResponse);

            if (out.error)
                RETURN(TPM_RC_FAILURE);
        }
    }

    return TPM_RC_SUCCESS;
}

/*
**==============================================================================
**
** TPM2_EncryptDecrypt()
**
**==============================================================================
*/

TPM_RC TPM2_EncryptDecrypt(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_DH_OBJECT keyHandle,
    IN const TPMS_AUTH_COMMAND *authCommand,
    IN TPMI_YES_NO decrypt,
    IN TPMI_ALG_SYM_MODE mode,
    IN TPM2B_IV *ivIn,
    IN TPM2B_MAX_BUFFER *inData,
    OUT TPM2B_MAX_BUFFER *outData,
    OUT TPM2B_IV *ivOut,
    OUT TPMS_AUTH_RESPONSE *authResponse)
{
    TPMBuf in;
    TPMBuf out;
    TPMS_AUTH_RESPONSE authResponseDummy;
    TPMI_ST_COMMAND_TAG tag = authCommand ? TPM_ST_SESSIONS:TPM_ST_NO_SESSIONS;

    /* Check input parameters */
    if (!protocol)
        RETURN(TPM_RC_FAILURE);

    /* Set null parameters to refer to dummy */
    {
        if (!authResponse)
            authResponse = &authResponseDummy;
    }

    /* Clear the output buffers */
    Memset(authResponse, 0, sizeof(*authResponse));

    /* Pack the request */
    {
        TPMBufInit(&in);

        TPMBufPackU32(&in, keyHandle);

        if (authCommand)
            _Pack_TPMS_AUTH_COMMAND(&in, authCommand);

        TPMBufPackU8(&in, decrypt);
        TPMBufPackU16(&in, mode);
        _Pack_TPM2B(&in, ivIn->size, ivIn->buffer);
        _Pack_TPM2B(&in, inData->size, inData->buffer);

        if (in.error)
            RETURN(TPM_RC_FAILURE);
    }

    /* Submit the request */
    {
        TPM_RC responseCode;
        TPMBufInit(&out);

        TPM_RC rc = _SubmitCommand(
            protocol,
            tag,
            TPM_CC_EncryptDecrypt,
            &responseCode,
            &in,
            &out);

        if (rc != EFI_SUCCESS)
            RETURN(TPM_RC_FAILURE);

        if (responseCode != TPM_RC_SUCCESS)
            RETURN(responseCode);
    }

    /* Unpack the response */
    {
        UINT32 parameterSize;

        /* Unpack handles */

        /* Unpack parameters */
        {
            if (tag == TPM_ST_SESSIONS)
                TPMBufUnpackU32(&out, &parameterSize);

            _Unpack_TPM2B(&out, (TPM2B*)outData);
            _Unpack_TPM2B(&out, (TPM2B*)ivOut);

            if (tag == TPM_ST_SESSIONS)
                _Unpack_TPMS_AUTH_RESPONSE(&out, authResponse);

            if (out.error)
                RETURN(TPM_RC_FAILURE);
        }
    }

    return TPM_RC_SUCCESS;
}

/*
**==============================================================================
**
** TPM2_LoadExternal()
**
**==============================================================================
*/

TPM_RC TPM2_LoadExternal(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN const TPMS_AUTH_COMMAND *authCommand,
    IN TPM2B_SENSITIVE *inPrivate,
    IN TPM2B_PUBLIC *inPublic,
    IN TPMI_RH_HIERARCHY hierarchy,
    OUT TPM_HANDLE *objectHandle,
    OUT TPM2B_NAME *name,
    OUT TPMS_AUTH_RESPONSE *authResponse)
{
    TPMBuf in;
    TPMBuf out;
    TPM_HANDLE objectHandleDummy;
    TPM2B_NAME nameDummy;
    TPMS_AUTH_RESPONSE authResponseDummy;
    TPMI_ST_COMMAND_TAG tag = authCommand ? TPM_ST_SESSIONS:TPM_ST_NO_SESSIONS;

    /* Check input parameters */
    if (!protocol)
        RETURN(TPM_RC_FAILURE);

    /* Set null parameters to refer to dummy */
    {
        if (!objectHandle)
            objectHandle = &objectHandleDummy;

        if (!name)
            name = &nameDummy;

        if (!authResponse)
            authResponse = &authResponseDummy;
    }

    /* Clear the output buffers */
    Memset(objectHandle, 0, sizeof(*objectHandle));
    Memset(name, 0, sizeof(*name));
    Memset(authResponse, 0, sizeof(*authResponse));

    /* Pack the request */
    {
        TPMBufInit(&in);

        if (authCommand)
            _Pack_TPMS_AUTH_COMMAND(&in, authCommand);

        _Pack_TPM2B_SENSITIVE(&in, inPrivate);
        _Pack_TPM2B_PUBLIC(&in, inPublic);
        TPMBufPackU32(&in, hierarchy);

        if (in.error)
            RETURN(TPM_RC_FAILURE);
    }

    /* Submit the request */
    {
        TPM_RC responseCode;
        TPMBufInit(&out);

        TPM_RC rc = _SubmitCommand(
            protocol,
            tag,
            TPM_CC_LoadExternal,
            &responseCode,
            &in,
            &out);

        if (rc != EFI_SUCCESS)
            RETURN(TPM_RC_FAILURE);

        if (responseCode != TPM_RC_SUCCESS)
            RETURN(responseCode);
    }

    /* Unpack the response */
    {
        UINT32 parameterSize;

        /* Unpack handles */
        TPMBufUnpackU32(&out, objectHandle);

        /* Unpack parameters */
        {
            if (tag == TPM_ST_SESSIONS)
                TPMBufUnpackU32(&out, &parameterSize);

            _Unpack_TPM2B(&out, (TPM2B*)name);

            if (tag == TPM_ST_SESSIONS)
                _Unpack_TPMS_AUTH_RESPONSE(&out, authResponse);

            if (out.error)
                RETURN(TPM_RC_FAILURE);
        }
    }

    return TPM_RC_SUCCESS;
}

/*
**==============================================================================
**
** TPM2_HashSeuqneceStart()
**
**==============================================================================
*/

TPM_RC TPM2_HashSequenceStart(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN const TPMS_AUTH_COMMAND* authCommand,
    IN TPM2B_AUTH *auth,
    IN TPMI_ALG_HASH hashAlg,
    OUT TPMI_DH_OBJECT *sequenceHandle,
    OUT TPMS_AUTH_RESPONSE *authResponse)
{
    TPMBuf in;
    TPMBuf out;
    OUT TPMI_DH_OBJECT sequenceHandleDummy;
    TPMS_AUTH_RESPONSE authResponseDummy;
    TPMI_ST_COMMAND_TAG tag = authCommand ? TPM_ST_SESSIONS:TPM_ST_NO_SESSIONS;

    /* Check input parameters */
    if (!protocol)
        RETURN(TPM_RC_FAILURE);

    /* Set null parameters to refer to dummy */
    {
        if (!sequenceHandle)
            sequenceHandle = &sequenceHandleDummy;

        if (!authResponse)
            authResponse = &authResponseDummy;
    }

    /* Clear the output buffers */
    Memset(authResponse, 0, sizeof(*authResponse));
    Memset(sequenceHandle, 0, sizeof(*sequenceHandle));

    /* Pack the request */
    {
        TPMBufInit(&in);

        if (tag == TPM_ST_SESSIONS)
            _Pack_TPMS_AUTH_COMMAND(&in, authCommand);

        _Pack_TPM2B(&in, auth->size, auth->buffer);
        TPMBufPackU16(&in, hashAlg);

        if (in.error)
            RETURN(TPM_RC_FAILURE);
    }

    /* Submit the request */
    {
        TPM_RC responseCode;
        TPMBufInit(&out);

        TPM_RC rc = _SubmitCommand(
            protocol,
            tag,
            TPM_CC_HashSequenceStart,
            &responseCode,
            &in,
            &out);

        if (rc != EFI_SUCCESS)
            RETURN(TPM_RC_FAILURE);

        if (responseCode != TPM_RC_SUCCESS)
            RETURN(responseCode);
    }

    /* Unpack the response */
    {
        UINT32 parameterSize;

        if (tag == TPM_ST_SESSIONS)
            TPMBufUnpackU32(&out, &parameterSize);

        TPMBufUnpackU32(&out, sequenceHandle);

        if (tag == TPM_ST_SESSIONS)
            _Unpack_TPMS_AUTH_RESPONSE(&out, authResponse);

        if (out.error)
            RETURN(TPM_RC_FAILURE);
    }

    return TPM_RC_SUCCESS;
}

/*
**==============================================================================
**
** TPM2_SequenceUpdate()
**
**==============================================================================
*/

TPM_RC TPM2_SequenceUpdate(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_DH_OBJECT sequenceHandle,
    IN const TPMS_AUTH_COMMAND* authCommand,
    IN TPM2B_MAX_BUFFER *buffer,
    OUT TPMS_AUTH_RESPONSE *authResponse)
{
    TPMBuf in;
    TPMBuf out;
    TPMS_AUTH_RESPONSE authResponseDummy;
    TPMI_ST_COMMAND_TAG tag = authCommand ? TPM_ST_SESSIONS:TPM_ST_NO_SESSIONS;


    /* Check input parameters */
    if (!protocol)
        RETURN(TPM_RC_FAILURE);

    /* Set null parameters to refer to dummy */
    {
        if (!authResponse)
            authResponse = &authResponseDummy;
    }

    /* Pack the request */
    {
        TPMBufInit(&in);

        TPMBufPackU32(&in, sequenceHandle);

        if (authCommand)
            _Pack_TPMS_AUTH_COMMAND(&in, authCommand);

        _Pack_TPM2B(&in, buffer->size, buffer->buffer);

        if (in.error)
            RETURN(TPM_RC_FAILURE);
    }

    /* Submit the request */
    {
        TPM_RC responseCode;
        TPMBufInit(&out);

        TPM_RC rc = _SubmitCommand(
            protocol,
            tag,
            TPM_CC_SequenceUpdate,
            &responseCode,
            &in,
            &out);

        if (rc != EFI_SUCCESS)
            RETURN(TPM_RC_FAILURE);

        if (responseCode != TPM_RC_SUCCESS)
            RETURN(responseCode);
    }

    /* Unpack the response */
    {
        UINT32 parameterSize;

        /* Unpack parameters */
        {
            if (tag == TPM_ST_SESSIONS)
                TPMBufUnpackU32(&out, &parameterSize);

            if (tag == TPM_ST_SESSIONS)
                _Unpack_TPMS_AUTH_RESPONSE(&out, authResponse);

            if (out.error)
                RETURN(TPM_RC_FAILURE);
        }
    }

    return TPM_RC_SUCCESS;
}

/*
**==============================================================================
**
** TPM2_SequenceComplete()
**
**==============================================================================
*/

TPM_RC TPM2_SequenceComplete(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_DH_OBJECT sequenceHandle,
    IN const TPMS_AUTH_COMMAND* authCommand,
    IN TPM2B_MAX_BUFFER *buffer,
    IN TPMI_RH_HIERARCHY hierarchy,
    OUT TPM2B_DIGEST *result,
    OUT TPMT_TK_HASHCHECK *validation,
    OUT TPMS_AUTH_RESPONSE *authResponse)
{
    TPMBuf in;
    TPMBuf out;
    TPM2B_DIGEST resultDummy;
    TPMT_TK_HASHCHECK validationDummy;
    TPMS_AUTH_RESPONSE authResponseDummy;
    TPMI_ST_COMMAND_TAG tag = authCommand ? TPM_ST_SESSIONS:TPM_ST_NO_SESSIONS;

    /* Check input parameters */
    if (!protocol)
        RETURN(TPM_RC_FAILURE);

    /* Set null parameters to refer to dummy */
    {
        if (!result)
            result = &resultDummy;

        if (!validation)
            validation = &validationDummy;

        if (!authResponse)
            authResponse = &authResponseDummy;
    }

    /* Clear the output buffers */
    Memset(result, 0, sizeof(*result));
    Memset(validation, 0, sizeof(*validation));
    Memset(authResponse, 0, sizeof(*authResponse));


    /* Pack the request */
    {
        TPMBufInit(&in);

        TPMBufPackU32(&in, sequenceHandle);

        if (authCommand)
            _Pack_TPMS_AUTH_COMMAND(&in, authCommand);

        _Pack_TPM2B(&in, buffer->size, buffer->buffer);
        TPMBufPackU32(&in, hierarchy);

        if (in.error)
            RETURN(TPM_RC_FAILURE);
    }

    /* Submit the request */
    {
        TPM_RC responseCode;
        TPMBufInit(&out);

        TPM_RC rc = _SubmitCommand(
            protocol,
            tag,
            TPM_CC_SequenceComplete,
            &responseCode,
            &in,
            &out);

        if (rc != EFI_SUCCESS)
            RETURN(TPM_RC_FAILURE);

        if (responseCode != TPM_RC_SUCCESS)
            RETURN(responseCode);
    }

    /* Unpack the response */
    {
        UINT32 parameterSize;

        /* Unpack parameters */
        {
            if (tag == TPM_ST_SESSIONS)
                TPMBufUnpackU32(&out, &parameterSize);

            _Unpack_TPM2B_DIGEST(&out, result);

            _Unpack_TPMT_TK_HASHCHECK(&out, validation);

            if (tag == TPM_ST_SESSIONS)
                _Unpack_TPMS_AUTH_RESPONSE(&out, authResponse);

            if (out.error)
                RETURN(TPM_RC_FAILURE);
        }
    }

    return TPM_RC_SUCCESS;
}



/*
**==============================================================================
**
** TPM2_ReadPublic()
**
**==============================================================================
*/
TPM_RC TPM2_ReadPublic(
    IN EFI_TCG2_PROTOCOL *protocol, 
    IN TPMI_DH_OBJECT objectHandle,
    IN const TPMS_AUTH_COMMAND* authCommand,
    OUT TPM2B_PUBLIC *outPublic)
{
    TPMBuf in;
    TPMBuf out;
    TPMI_ST_COMMAND_TAG tag = authCommand ? TPM_ST_SESSIONS : TPM_ST_NO_SESSIONS;

    /* Pack request. */
    {
        TPMBufInit(&in);
        TPMBufPackU32(&in, objectHandle);
    }

    /* Submit Request. */
    {
        TPM_RC responseCode;
        TPMBufInit(&out);
        
        TPM_RC rc = _SubmitCommand(
                        protocol,
                        tag,
                        TPM_CC_ReadPublic,
                        &responseCode,
                        &in,
                        &out);

        if (rc != EFI_SUCCESS)
            RETURN(TPM_RC_FAILURE);
        
        if (responseCode != TPM_RC_SUCCESS)
            RETURN(responseCode);
    }

    /* Unpack the response. */
    {
        UINT32 parameterSize;

        if (tag == TPM_ST_SESSIONS)
            TPMBufUnpackU32(&out, &parameterSize);


        _Unpack_TPM2B_PUBLIC(&out, outPublic);
        
        if (out.error)
            RETURN(TPM_RC_FAILURE);
    }

    return TPM_RC_SUCCESS;
}

/*
**==============================================================================
**
** TPM2 Extras:
**
**==============================================================================
*/

TPM_RC TPM2X_SetLockoutParams(
    EFI_TCG2_PROTOCOL *protocol)
{
    TPMS_AUTH_COMMAND authCommand;

    Memset(&authCommand, 0, sizeof(authCommand));
    authCommand.sessionHandle = TPM_RS_PW;

    TPM_RC rc = TPM2_DictionaryAttackParameters(
        protocol,
        TPM_RH_LOCKOUT,
        &authCommand,
        1000000, /* newMaxTries */
        1, /* newRecoveryTime */
        1, /* lockoutRecovery */
        NULL);

    return rc;
}

TPM_RC TPM2X_SetDictionaryAttackLockReset(
    EFI_TCG2_PROTOCOL *protocol)
{
    TPMS_AUTH_COMMAND authCommand;
    TPM_RC rc;

    Memset(&authCommand, 0, sizeof(authCommand));
    authCommand.sessionHandle = TPM_RS_PW;

    rc = TPM2_DictionaryAttackLockReset(
        protocol,
        TPM_RH_LOCKOUT,
        &authCommand,
        NULL);

    return rc;
}

TPM_RC TPM2X_GetTPMRevision(
    IN EFI_TCG2_PROTOCOL *protocol,
    OUT UINT32 *revision)
{
    TPMS_CAPABILITY_DATA data;
    TPMS_TAGGED_PROPERTY property;
    TPM_RC rc;

    if ((rc = TPM2_GetCapability(
        protocol,
        NULL,
        TPM_CAP_TPM_PROPERTIES,
        TPM_PT_REVISION,
        1,
        NULL,
        &data,
        NULL)) != TPM_RC_SUCCESS)
    {
        return rc;
    }

    if (data.data.tpmProperties.count != 1)
        return TPM_RC_FAILURE;

    property = data.data.tpmProperties.tpmProperty[0];

    if (property.property != TPM_PT_REVISION)
        return TPM_RC_FAILURE;

    *revision = property.value;
    return TPM_RC_SUCCESS;
}

TPM_RC TPM2X_GetPCRCount(
    IN EFI_TCG2_PROTOCOL *protocol,
    OUT UINT32* pcrCount)
{
    TPMS_CAPABILITY_DATA data;
    TPMS_TAGGED_PROPERTY property;
    TPM_RC rc;

    if ((rc = TPM2_GetCapability(
        protocol,
        NULL,
        TPM_CAP_TPM_PROPERTIES,
        TPM_PT_PCR_COUNT,
        1,
        NULL,
        &data,
        NULL)) != TPM_RC_SUCCESS)
    {
        return rc;
    }

    if (data.data.tpmProperties.count != 1)
        return TPM_RC_FAILURE;

    property = data.data.tpmProperties.tpmProperty[0];

    if (property.property != TPM_PT_PCR_COUNT)
        return TPM_RC_FAILURE;

    *pcrCount = property.value;
    return TPM_RC_SUCCESS;
}

TPM_RC TPM2X_Hash(
    EFI_TCG2_PROTOCOL *protocol,
    TPMI_ALG_HASH hashAlg,
    const BYTE *data,
    UINT16 size,
    TPM2B_DIGEST *result)
{
    TPM2B_MAX_BUFFER buf;
    TPM_RC rc;

    if (size > sizeof(buf.buffer))
        return TPM_RC_SIZE;

    buf.size = size;
    Memcpy(buf.buffer, (void*)data, size);

    rc = TPM2_Hash(
        protocol,
        0,
        &buf,
        hashAlg,
        TPM_RH_NULL,
        result,
        0,
        0);

    return rc;
}

TPM_RC TPM2X_StartAuthSession( 
    EFI_TCG2_PROTOCOL *protocol,
    BOOLEAN trial,
    TPMI_ALG_HASH alg,
    TPMI_SH_AUTH_SESSION *policySession)
{
    TPM_RC rc;
    TPM2B_NONCE nonceCaller;
    TPM2B_ENCRYPTED_SECRET salt;
    TPMT_SYM_DEF symmetric;
    TPM2B_NONCE nonceTPM;

    Memset(&nonceCaller, 0, sizeof(nonceCaller));
    if (alg == TPM_ALG_SHA1)
    {
        nonceCaller.size = sizeof(SHA1Hash);
    }
    else if (alg == TPM_ALG_SHA256)
    {
        nonceCaller.size = sizeof(SHA256Hash);
    }
    else
    {
        /* Unknown hash. */
        return TPM_RC_FAILURE;
    }


    Memset(&salt, 0, sizeof(salt));

    Memset(&symmetric, 0, sizeof(symmetric));
    symmetric.algorithm = TPM_ALG_NULL;

    Memset(&nonceTPM, 0, sizeof(nonceTPM));

    rc = TPM2_StartAuthSession( 
        protocol,
        TPM_RH_NULL, 
        TPM_RH_NULL, 
        0, 
        &nonceCaller,
        &salt,
        trial ? TPM_SE_TRIAL : TPM_SE_POLICY,
        &symmetric,
        alg,
        policySession,
        &nonceTPM,
        0);

    return rc;
}

TPM_RC TPM2X_BuildPolicy(
    EFI_TCG2_PROTOCOL *protocol,
    TPMI_SH_AUTH_SESSION policySession,
    BOOLEAN predictive,
    UINT32 pcrMask,
    const SHA256Hash pcrs[MAX_PCRS],
    TPMI_ALG_HASH hashAlg,
    Error* err)
{
    TPM_RC rc;
    TPML_PCR_SELECTION pcrSelectionIn;
    UINT32 pcrUpdateCounter;
    TPML_PCR_SELECTION pcrSelectionOut;
    TPML_DIGEST pcrValues;
    UINT32 i = 0;
    UINT32 npcrs = 0;

    ClearErr(err);

    /* Check the hashing algorithm, should be SHA1 or SHA256. */
    if (hashAlg != TPM_ALG_SHA1 && hashAlg != TPM_ALG_SHA256)
    {
        SetErr(err, TCS("Unsupported hash function"));
        return TPM_RC_FAILURE;
    }


    /* Set NULL password for this policy */
    if ((rc = TPM2_PolicyPassword(
        protocol, 
        policySession,
        NULL, 
        NULL)) != TPM_RC_SUCCESS)
    {
        SetErr(err, TCS("failed to set the policy password"));
        return rc;
    }

    Memset(&pcrSelectionIn, 0, sizeof(pcrSelectionIn));
    pcrSelectionIn.count = 1;
    /* Note: this is not the number of PCRs */
    pcrSelectionIn.pcrSelections[0].sizeofSelect = 3;
    pcrSelectionIn.pcrSelections[0].hash = hashAlg;

    for (i = 0; i < MAX_PCRS; i++)
    {
        if (pcrMask & (1 << i))
        {
            TPMS_PCR_SELECTION_SelectPCR(&pcrSelectionIn.pcrSelections[0], i);
            npcrs++;
        }
    }

    Memset(&pcrUpdateCounter, 0, sizeof(pcrUpdateCounter));
    Memset(&pcrSelectionOut, 0, sizeof(pcrSelectionOut));
    Memset(&pcrValues, 0, sizeof(pcrValues));

    if (predictive)
    {
        UINT32 n;

        pcrValues.count = npcrs;

        for (i = 0, n = 0; i < MAX_PCRS; i++)
        {
            if (pcrMask & (1 << i))
            {
                Memcpy(pcrValues.digests[n].buffer, &pcrs[i], sizeof(pcrs[i]));
                pcrValues.digests[n].size = sizeof(pcrs[i]);
                n++;
            }
        }
    }
    else
    {
        if ((rc = TPM2_PCR_Read(
            protocol,
            NULL, /* input */
            &pcrSelectionIn, /* input */
            &pcrUpdateCounter, /* not used */
            &pcrSelectionOut, /* not used */
            &pcrValues,
            NULL)) != TPM_RC_SUCCESS)
        {
            SetErr(err, TCS("failed to read PCRs when building policy"));
            return rc;
        }
    }

    if (pcrValues.count != npcrs)
    {
        SetErr(err, TCS("unexpected error when building policy"));
        return TPM_RC_FAILURE;
    }

    /* Hash the PCRs together */
    TPM2B_DIGEST pcrDigest;
    {
        unsigned char data[sizeof(pcrValues.digests[0].buffer) * MAX_PCRS];
        unsigned char* end = data;

        Memset(&pcrDigest, 0, sizeof(pcrDigest));

        for (i = 0; i < npcrs; i++)
        {
            Memcpy(end, pcrValues.digests[i].buffer, pcrValues.digests[i].size);
            end += pcrValues.digests[i].size;
        }

        if (hashAlg == TPM_ALG_SHA1)
        {
            if (!ComputeSHA1(data, end - data, (SHA1Hash*)pcrDigest.buffer))
            {
                SetErr(err, TCS("hash operation failed while building policy"));
                return rc;
            }
            pcrDigest.size = sizeof(SHA1Hash);
        }
        else /* SHA256 */
        {
            if (!ComputeSHA256(data, end - data, (SHA256Hash*)pcrDigest.buffer))
            {
                SetErr(err, TCS("hash operation failed while building policy"));
                return rc;
            }
            pcrDigest.size = sizeof(SHA256Hash);
        }
    }

    /* Create the PCR policy */
    if ((rc = TPM2_PolicyPCR(
        protocol,
        policySession,
        NULL,
        &pcrDigest,
        &pcrSelectionIn,
        NULL)) != TPM_RC_SUCCESS)
    {
        SetErr(err, TCS("failed to create PCR policy"));
        return rc;
    }

    return TPM_RC_SUCCESS;
}

TPM_RC TPM2X_CreateSRKKey(
    EFI_TCG2_PROTOCOL *protocol,
    TPM_HANDLE *srkHandle)
{
    TPMS_AUTH_COMMAND authCommand;
    TPM2B_SENSITIVE_CREATE inSensitive;
    TPM2B_PUBLIC inPublic;
    TPM2B_DATA outsideInfo;
    TPML_PCR_SELECTION creationPcr;
    TPM2B_PUBLIC outPublic;
    TPM2B_CREATION_DATA creationData;
    TPM2B_DIGEST creationHash;
    TPMT_TK_CREATION creationTicket;
    TPM2B_NAME srkName;

    Memset(&authCommand, 0, sizeof(authCommand));
    Memset(&inSensitive, 0, sizeof(inSensitive));
    Memset(&inPublic, 0, sizeof(inPublic));
    Memset(&outsideInfo, 0, sizeof(outsideInfo));
    Memset(&creationPcr, 0, sizeof(creationPcr));
    Memset(&outPublic, 0, sizeof(outPublic));
    Memset(&creationData, 0, sizeof(creationData));
    Memset(&creationHash, 0, sizeof(creationHash));
    Memset(&creationTicket, 0, sizeof(creationTicket));
    Memset(&srkName, 0, sizeof(srkName));

    authCommand.sessionHandle = TPM_RS_PW;
    {
        {
            TPMA_OBJECT objectAttributes;
            Memset(&objectAttributes, 0, sizeof(objectAttributes));
            objectAttributes.restricted = 1;
            objectAttributes.userWithAuth = 1;
            objectAttributes.decrypt = 1;
            objectAttributes.fixedTPM = 1;
            objectAttributes.fixedParent = 1;
            objectAttributes.sensitiveDataOrigin = 1;
            objectAttributes.noDA = 1;       
            inPublic.publicArea.objectAttributes = objectAttributes;
        }

        {
            TPMS_RSA_PARMS rsaDetail;
            Memset(&rsaDetail, 0, sizeof(rsaDetail));
            rsaDetail.symmetric.algorithm = TPM_ALG_AES;
            rsaDetail.symmetric.keyBits.aes = 128;
            rsaDetail.symmetric.mode.aes = TPM_ALG_CFB;
            rsaDetail.scheme.scheme = TPM_ALG_NULL;
            rsaDetail.keyBits = 2048;
            rsaDetail.exponent = 0;
            inPublic.publicArea.parameters.rsaDetail = rsaDetail;
        }

        inPublic.publicArea.type = TPM_ALG_RSA;
        inPublic.publicArea.nameAlg = TPM_ALG_SHA256;
    }

    TPM_RC rc = TPM2_CreatePrimary( 
        protocol,
        TPM_RH_OWNER, 
        &authCommand,
        &inSensitive,
        &inPublic,
        &outsideInfo,
        &creationPcr,
        srkHandle,
        &outPublic,
        &creationData,
        &creationHash,
        &creationTicket,
        &srkName,
        NULL); /* TSS2_SYS_RSP_AUTHS */

    return rc;
}

TPM_RC TPM2X_Seal(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPM_HANDLE parentKey,
    BOOLEAN predictive,
    UINT32 pcrMask,
    const SHA256Hash pcrs[MAX_PCRS],
    IN const BYTE data[TPM2X_MAX_DATA_SIZE],
    IN UINT16 size,
    OUT TPM2X_BLOB *blob,
    Error* err)
{
    TPM_RC rc = TPM_RC_SUCCESS;
    TPMI_SH_AUTH_SESSION policyHandle;
    BOOLEAN policyHandleNull = TRUE;
    TPM2B_DIGEST policyDigest;
    TPM2B_PRIVATE outPrivate;
    TPM2B_PUBLIC outPublic;

    ClearErr(err);

    if (!protocol)
    {
        SetErr(err, TCS("null protocol parameter"));
        rc = TPM_RC_FAILURE;
        goto done;
    }

    if (!data)
    {
        SetErr(err, TCS("null data parameter"));
        rc = TPM_RC_FAILURE;
        goto done;
    }

    if (size > TPM2X_MAX_DATA_SIZE)
    {
        SetErr(err, TCS("size parameter too big"));
        rc = TPM_RC_FAILURE;
        goto done;
    }

    if (!blob)
    {
        SetErr(err, TCS("null blob parameter"));
        rc = TPM_RC_FAILURE;
        goto done;
    }

    if ((rc = TPM2X_StartAuthSession(
        protocol, 
        TRUE,
        TPM_ALG_SHA256,
        &policyHandle)) != TPM_RC_SUCCESS)
    {
        SetErr(err, TCS("failed to start authentication session"));
        return rc;
        goto done;
    }

    policyHandleNull = FALSE;

    if (TPM2X_BuildPolicy(
        protocol, 
        policyHandle, 
        predictive,
        pcrMask,
        pcrs,
        TPM_ALG_SHA256,
        err) != TPM_RC_SUCCESS)
    {
        goto done;
    }

    if ((rc = TPM2_PolicyGetDigest(
        protocol, 
        policyHandle,
        NULL,
        &policyDigest,
        NULL)) != TPM_RC_SUCCESS)
    {
        SetErr(err, TCS("failed to get the policy digest"));
        goto done;
    }

    /* Create */
    {
        TPMS_AUTH_COMMAND authCommand;
        TPM2B_SENSITIVE_CREATE inSensitive;
        TPM2B_PUBLIC inPublic;
        TPM2B_DATA outsideInfo;
        TPML_PCR_SELECTION creationPCR;
        TPM2B_CREATION_DATA creationData;
        TPM2B_DIGEST creationHash;
        TPMT_TK_CREATION creationTicket;
        TPMS_AUTH_RESPONSE authResponse;
        UINT32 i;

        /* authCommand */
        Memset(&authCommand, 0, sizeof(authCommand));
        authCommand.sessionHandle = TPM_RS_PW;

        /* inSensitive */
        Memset(&inSensitive, 0, sizeof(inSensitive));
        inSensitive.sensitive.data.size = size;
        Memcpy(inSensitive.sensitive.data.buffer, (void*)data, size);

        /* inPublic */
        Memset(&inPublic, 0, sizeof(inPublic));
        inPublic.publicArea.type = TPM_ALG_KEYEDHASH;
        inPublic.publicArea.nameAlg = TPM_ALG_SHA256;
        inPublic.publicArea.parameters.keyedHashDetail.scheme.scheme = 
            TPM_ALG_NULL;
        inPublic.publicArea.authPolicy = policyDigest;

        /* outsideInfo */
        Memset(&outsideInfo, 0, sizeof(outsideInfo));

        /* creationPCR: Select PCRs */
        Memset(&creationPCR, 0, sizeof(creationPCR));
        creationPCR.count = 1;
        creationPCR.pcrSelections[0].sizeofSelect = 3;
        creationPCR.pcrSelections[0].hash = TPM_ALG_SHA256;

        for (i = 0; i < MAX_PCRS; i++)
        {
            if (pcrMask & (1 << i))
                TPMS_PCR_SELECTION_SelectPCR(&creationPCR.pcrSelections[0], i);
        }

        if ((rc = TPM2_Create(
            protocol,
            parentKey,
            &authCommand,
            &inSensitive,
            &inPublic,
            &outsideInfo,
            &creationPCR,
            &outPrivate,
            &outPublic,
            &creationData,
            &creationHash,
            &creationTicket,
            &authResponse)) != TPM_RC_SUCCESS)
        {
            SetErr(err, TCS("failed to create sealed object"));
            goto done;
        }

        blob->private = outPrivate;
        blob->public = outPublic;
        blob->hash = creationHash;
        blob->pcr.sizeofSelect = creationPCR.pcrSelections[0].sizeofSelect;
        Memcpy(blob->pcr.pcrSelect,
               creationPCR.pcrSelections[0].pcrSelect,
               sizeof(blob->pcr.pcrSelect));
    }

done:

    if (!policyHandleNull)
        TPM2_FlushContext(protocol, policyHandle);

    return rc;
}

TPM_RC TPM2X_Unseal(
    IN EFI_TCG2_PROTOCOL *protocol,
    UINT32 pcrMask,
    IN TPM_HANDLE parentKey,
    IN const TPM2X_BLOB *blob,
    OUT BYTE data[TPM2X_MAX_DATA_SIZE],
    OUT UINT16 *size,
    Error* err)
{
    TPM_RC rc = TPM_RC_SUCCESS;
    TPMI_SH_AUTH_SESSION sessionHandle = 0;
    BOOLEAN sessionHandleNull = TRUE;
    TPM_HANDLE sealedBlobHandle;
    BOOLEAN sealedBlobHandleNull = TRUE;
    TPM2B_PRIVATE inPrivate;
    TPM2B_PUBLIC inPublic;
    TPMI_ALG_HASH hashAlg;

    ClearErr(err);
    if (!blob)
    {
        SetErr(err, TCS("null blob parameter"));
        rc = TPM_RC_FAILURE;
        goto done;
    }


    /* First, check the hash algo. Only support SHA1 and SHA256. */
    hashAlg = blob->public.publicArea.nameAlg;
    if (hashAlg != TPM_ALG_SHA1 && hashAlg != TPM_ALG_SHA256)
    {
        SetErr(err, TCS("unsupported hashing algorithm"));
        rc = TPM_RC_FAILURE;
        goto done;
    }


    if ((rc = TPM2X_StartAuthSession(
        protocol, 
        FALSE,
        hashAlg,
        &sessionHandle)) != TPM_RC_SUCCESS)
    {
        SetErr(err, TCS("failed to start authentication session"));
        goto done;
    }

    sessionHandleNull = FALSE;

    if ((rc = TPM2X_BuildPolicy(
        protocol, 
        sessionHandle,
        FALSE,
        pcrMask,
        NULL,
        hashAlg,
        err)) != TPM_RC_SUCCESS)
    {
        goto done;
    }

    {
        TPMS_AUTH_COMMAND authCommand;
        TPM2B_NAME name;
        TPMS_AUTH_RESPONSE authResponse;

        /* authCommand */
        Memset(&authCommand, 0, sizeof(authCommand));
        authCommand.sessionHandle = TPM_RS_PW;

        /* sealedBlobHandle */
        Memset(&sealedBlobHandle, 0, sizeof(sealedBlobHandle));

        inPrivate = blob->private;
        inPublic = blob->public;

        rc = TPM2_Load( 
            protocol,
            parentKey,
            &authCommand,
            &inPrivate,
            &inPublic,
            &sealedBlobHandle,
            &name,
            &authResponse);

        if (rc != TPM_RC_SUCCESS)
        {
            SetErr(err, TCS("failed to load sealed blob"));
            goto done;
        }

        sealedBlobHandleNull = FALSE;
    }

    /* Unseal the sealed blob */
    {
        TPMS_AUTH_COMMAND authCommand;
        TPM2B_SENSITIVE_DATA outData;
        TPMS_AUTH_RESPONSE authResponse;

        /* authCommand */
        Memset(&authCommand, 0, sizeof(authCommand));
        authCommand.sessionHandle = sessionHandle;

        rc = TPM2_Unseal(
            protocol,
            sealedBlobHandle,
            &authCommand,
            &outData,
            &authResponse);

        if (rc != TPM_RC_SUCCESS)
        {
            SetErr(err, TCS("TPM_Unseal Failed %u"), rc);
            goto done;
        }

        /* TPM2_Unseal() seems to implicitly release 'sessionHandle' */
        sessionHandleNull = TRUE;

        Memcpy(data, outData.buffer, outData.size);
        *size = outData.size;
    }

done:

    if (!sessionHandleNull)
        TPM2_FlushContext(protocol, sessionHandle);

    if (!sealedBlobHandleNull)
        TPM2_FlushContext(protocol, sealedBlobHandle);

    return rc;
}

TPM_RC TPM2X_Cap(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_DH_PCR pcrHandle)
{
    TPM_RC rc;
    TPMS_AUTH_COMMAND authCommand;
    TPML_DIGEST_VALUES digests;
    UINT8 zeros[20];

    /* authCommand */
    Memset(&authCommand, 0, sizeof(authCommand));
    authCommand.sessionHandle = TPM_RS_PW;

    /* Clear the zeros buffer */
    Memset(zeros, 0, sizeof(zeros));

    /* SHA1 Capping */
    {
        SHA1Hash sha1;

        ComputeSHA1(zeros, sizeof(zeros), &sha1);
        Memset(&digests, 0, sizeof(digests));
        digests.count = 1;
        digests.digests[0].hashAlg = TPM_ALG_SHA1;
        Memcpy(digests.digests[0].digest.sha1, &sha1, sizeof(sha1));

        if ((rc = TPM2_PCR_Extend(
            protocol,
            pcrHandle,
            &authCommand,
            &digests,
            0)) != TPM_RC_SUCCESS)
        {
            return rc;
        }
    }

    /* SHA256 Capping */
    {
        SHA256Hash sha256;

        ComputeSHA256(zeros, sizeof(zeros), &sha256);
        Memset(&digests, 0, sizeof(digests));
        digests.count = 1;
        digests.digests[0].hashAlg = TPM_ALG_SHA256;
        Memcpy(digests.digests[0].digest.sha256, &sha256, sizeof(sha256));

        if ((rc = TPM2_PCR_Extend(
            protocol,
            pcrHandle,
            &authCommand,
            &digests,
            0)) != TPM_RC_SUCCESS)
        {
            return rc;
        }
    }

    return TPM_RC_SUCCESS;
}

TPM_RC TPM2X_CapAll(
    EFI_TCG2_PROTOCOL* protocol)
{
    TPM_RC rc = TPM_RC_SUCCESS;
    UINT32 i;
    const UINT32 PCR_COUNT = 16;
    SHA1Hash sha1s[16];
    SHA256Hash sha256s[16];

    /* Save the values of all the PCRs */
    for (i = 0; i < PCR_COUNT; i++)
    {
        SHA1Hash sha1;
        SHA256Hash sha256;

        if ((rc = TPM2X_ReadPCRSHA1(protocol, 0, &sha1)) != TPM_RC_SUCCESS)
        {
            goto done;
        }

        if ((rc = TPM2X_ReadPCRSHA256(protocol, 0, &sha256)) != TPM_RC_SUCCESS)
        {
            goto done;
        }

        sha1s[i] = sha1;
        sha256s[i] = sha256;
    }

    /* Cap each PCR */
    for (i = 0; i < PCR_COUNT; i++)
    {
        if ((rc = TPM2X_Cap(protocol, i)) != TPM_RC_SUCCESS)
        {
            goto done;
        }
    }

    /* Check that each PCR has actually changed value */
    for (i = 0; i < PCR_COUNT; i++)
    {
        SHA1Hash sha1;
        SHA256Hash sha256;

        if ((rc = TPM2X_ReadPCRSHA1(protocol, 0, &sha1)) != TPM_RC_SUCCESS)
        {
            goto done;
        }

        if (SHA1Equal(&sha1, &sha1s[i]))
        {
            rc = TPM_RC_FAILURE;
            goto done;
        }

        if ((rc = TPM2X_ReadPCRSHA256(protocol, 0, &sha256)) != TPM_RC_SUCCESS)
        {
            goto done;
        }

        if (SHA256Equal(&sha256, &sha256s[i]))
        {
            rc = TPM_RC_FAILURE;
            goto done;
        }
    }

done:

    return rc;
}

static unsigned char _CharToHex(char x)
{
    if (x >= '0' && x <= '9')
        return x - '0';
    if (x >= 'A' && x <= 'F')
        return 10 + x - 'A';
    if (x >= 'a' && x <= 'f')
        return 10 + x - 'a';
    else 
        return 0xFF;
}

int TPM2X_HexStrToBinary(
    const char* str,
    UINT32 len,
    BYTE* binary,
    UINT32* binarySize)
{
    UINTN i;
    UINTN j;

    if (len % 2)
        return -1;

    *binarySize = len / 2;

    for (i = 0, j = 0; i < len; i += 2, j++)
    {
        if (_CharToHex(str[i]) == 0xFF || _CharToHex(str[i+1]) == 0xFF)
            return -1;

        binary[j] = (_CharToHex(str[i]) << 4) | _CharToHex(str[i+1]);
    }

    return 0;
}

int TPM2X_HexStrToKey(
    const char* str,
    BYTE* key,
    UINT32* keyBytes)
{
    UINTN n = Strlen(str);

    if (n != 32 && n != 64)
        return -1;

    return TPM2X_HexStrToBinary(str, n, key, keyBytes);
}

static char _HexNibbleToChar(
    BYTE x)
{
    if (x >= 0 && x <= 9)
        return '0' + x;
    if (x >= 0x0A && x <= 0x0F)
        return 'A' + (x - 10);

    return -1;
}

static void _HexByteToStr(
    BYTE x,
    char str[2])
{
    BYTE hi = (x & 0xF0) >> 4;
    BYTE lo = 0x0F & x;

    str[0] = _HexNibbleToChar(hi);
    str[1] = _HexNibbleToChar(lo);
}

void TPM2X_BinaryToHexStr(
    const BYTE* binary,
    UINT32 binarySize,
    char* hexstr)
{
    UINT32 i;
    UINT32 j;

    for (i = 0, j = 0; i < binarySize; i++)
    {
        char chars[2];
        _HexByteToStr(binary[i], chars);
        hexstr[j++] = chars[0];
        hexstr[j++] = chars[1];
    }

    hexstr[j] = '\0';
}

TPM_RC TPM2X_ReadPCRSHA1(
    EFI_TCG2_PROTOCOL* protocol,
    UINT32 pcrNumber,
    SHA1Hash* sha1)
{
    TPM_RC rc;
    TPML_PCR_SELECTION pcrSelectionIn;
    UINT32 pcrUpdateCounter;
    TPML_PCR_SELECTION prcSelectionOut;
    TPML_DIGEST pcrValues;

    /* Select PCR[pcrNumber] */
    Memset(&pcrSelectionIn, 0, sizeof(pcrSelectionIn));
    pcrSelectionIn.count = 1;
    pcrSelectionIn.pcrSelections[0].sizeofSelect = 3;
    pcrSelectionIn.pcrSelections[0].hash = TPM_ALG_SHA1;
    TPMS_PCR_SELECTION_SelectPCR(&pcrSelectionIn.pcrSelections[0], pcrNumber);

    /* Read the PCRs */
    if ((rc = TPM2_PCR_Read(
        protocol,
        NULL, /* cmdAuthsArray */
        &pcrSelectionIn,
        &pcrUpdateCounter,
        &prcSelectionOut,
        &pcrValues,
        NULL)) != TPM_RC_SUCCESS)
    {
        return rc;
    }

    if (pcrValues.digests[0].size != sizeof(SHA1Hash))
    {
        return TPM_RC_FAILURE;
    }

    Memcpy(sha1, pcrValues.digests[0].buffer, sizeof(SHA1Hash));
    return TPM_RC_SUCCESS;
}

TPM_RC TPM2X_ReadPCRSHA256(
    EFI_TCG2_PROTOCOL* protocol,
    UINT32 pcrNumber,
    SHA256Hash* sha256)
{
    TPM_RC rc;
    TPML_PCR_SELECTION pcrSelectionIn;
    UINT32 pcrUpdateCounter;
    TPML_PCR_SELECTION prcSelectionOut;
    TPML_DIGEST pcrValues;

    /* Select PCR256[pcrNumber] */
    Memset(&pcrSelectionIn, 0, sizeof(pcrSelectionIn));
    pcrSelectionIn.count = 1;
    pcrSelectionIn.pcrSelections[0].sizeofSelect = 3;
    pcrSelectionIn.pcrSelections[0].hash = TPM_ALG_SHA256;
    TPMS_PCR_SELECTION_SelectPCR(&pcrSelectionIn.pcrSelections[0], pcrNumber);

    /* Read the PCR256s */
    if ((rc = TPM2_PCR_Read(
        protocol,
        NULL, /* cmdAuthsArray */
        &pcrSelectionIn,
        &pcrUpdateCounter,
        &prcSelectionOut,
        &pcrValues,
        NULL)) != TPM_RC_SUCCESS)
    {
        return rc;
    }

    if (pcrValues.digests[0].size != sizeof(SHA256Hash))
    {
        return TPM_RC_FAILURE;
    }

    Memcpy(sha256, pcrValues.digests[0].buffer, sizeof(SHA256Hash));
    return TPM_RC_SUCCESS;
}

TPM_RC TPM2X_ExtendPCR_SHA1(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_DH_PCR pcrHandle,
    IN const SHA1Hash* sha1In)
{
    TPM_RC rc;
    TPMS_AUTH_COMMAND authCommand;
    TPML_DIGEST_VALUES digests;

    /* authCommand */
    Memset(&authCommand, 0, sizeof(authCommand));
    authCommand.sessionHandle = TPM_RS_PW;

    /* digests */
    Memset(&digests, 0, sizeof(digests));
    digests.count = 1;
    digests.digests[0].hashAlg = TPM_ALG_SHA1;
    Memcpy(digests.digests[0].digest.sha1, (BYTE*)sha1In, sizeof(SHA1Hash));

    if ((rc = TPM2_PCR_Extend(
        protocol,
        pcrHandle,
        &authCommand,
        &digests,
        0)) != TPM_RC_SUCCESS)
    {
        return rc;
    }

    return TPM_RC_SUCCESS;
}

TPM_RC TPM2X_ExtendPCR_SHA256(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_DH_PCR pcrHandle,
    IN const SHA256Hash* sha256In)
{
    TPM_RC rc;
    TPMS_AUTH_COMMAND authCommand;
    TPML_DIGEST_VALUES digests;

    /* authCommand */
    Memset(&authCommand, 0, sizeof(authCommand));
    authCommand.sessionHandle = TPM_RS_PW;

    /* digests */
    Memset(&digests, 0, sizeof(digests));
    digests.count = 1;
    digests.digests[0].hashAlg = TPM_ALG_SHA256;
    Memcpy(digests.digests[0].digest.sha256, sha256In,sizeof(SHA256Hash));

    if ((rc = TPM2_PCR_Extend(
        protocol,
        pcrHandle,
        &authCommand,
        &digests,
        0)) != TPM_RC_SUCCESS)
    {
        return rc;
    }

    return TPM_RC_SUCCESS;
}

EFI_STATUS TPM2X_CheckTPMPresentFlag(
    EFI_TCG2_PROTOCOL *protocol)
{
    EFI_STATUS status = EFI_SUCCESS;
    EFI_TCG2_BOOT_SERVICE_CAPABILITY capability;

    /* Clear the capability structure */
    Memset(&capability, sizeof(capability), 0x00);

    /* Get the TCG2 capability structure */
    if ((status = TCG2_GetCapability(protocol, &capability)) != EFI_SUCCESS)
    {
        goto done;
    }

    /* Print error if TPM is not enabled */
    if (!capability.TPMPresentFlag)
    {
        status = EFI_NOT_FOUND;
        goto done;
    }

done:
    return status;
}

int TPM2X_SerializeBlob(
    IN const TPM2X_BLOB* blob,
    IN OUT BYTE* blobBuffer,
    IN UINT32 blobBufferSize,
    OUT UINT32 *newBlobBufferSize)
{
    TPMBuf buf;

    if (!blob || ! blobBuffer || !newBlobBufferSize || blobBufferSize < sizeof(TPM2X_BLOB))
    {
        return -1;
    }

    TPMBufInit(&buf);
    _Pack_TPM2B_PRIVATE(&buf, &blob->private);
    _Pack_TPM2B_PUBLIC(&buf, (TPM2B_PUBLIC*) &blob->public);
    _Pack_TPM2B_DIGEST(&buf, &blob->hash);
    _Pack_TPMS_PCR_SELECT(&buf, &blob->pcr);
    
    if (buf.error != 0)
    {
        return -1;
    }


    Memcpy(blobBuffer, buf.data, buf.size);
    *newBlobBufferSize = buf.size;
    return 0;
}


int TPM2X_DeserializeBlob(
    IN const BYTE* blobBuffer,
    IN UINT32 blobBufferSize,
    OUT TPM2X_BLOB* blob)
{
    TPMBuf buf;


    if (!blobBuffer || !blob || blobBufferSize > sizeof(TPM2X_BLOB))
    {
        return -1;
    }

    /* Create the buffer. */
    TPMBufInit(&buf);
    TPMBufPack(&buf, blobBuffer, blobBufferSize);
    if (buf.error != 0)
    {
        return -1;
    }

    _Unpack_TPM2B_PRIVATE(&buf, &blob->private);
    _Unpack_TPM2B_PUBLIC(&buf, &blob->public);
    _Unpack_TPM2B_DIGEST(&buf, &blob->hash);
    _Unpack_TPMS_PCR_SELECT(&buf, &blob->pcr);
   
    /* Check for buffer overflow. */
    return buf.error == 0 ? 0: -1;
}

int TPM2X_DumpBlob(
    IN const BYTE data[TPM2X_MAX_DATA_SIZE],
    IN UINT16 size)
{
    TPM_RC rc = TPM_RC_FAILURE;
    TPMBuf buf;
    OUT TPM2X_BLOB blob;

    if (!data)
        goto done;


    /* Create the buffer. */
    {
        TPMBufInit(&buf);

        TPMBufPack(&buf, data, size);

        if (buf.error != 0)
            goto done;
    }

    /* Dump TPM2B_PRIVATE */
    {
        _Unpack_TPM2B_PRIVATE(&buf, &blob.private);

        if (buf.error != 0)
            goto done;

        PRINTF0("TPM2B_PRIVATE.size: ");
        PRINTF("%ld\n", (long)blob.private.size);
        PRINTF0("TPM2B_PRIVATE.buffer:\n");
        HexDump(blob.private.buffer, blob.private.size);
    }

    /* Dump TPM2B_PUBLIC */
    {
        _Unpack_TPM2B_PUBLIC(&buf, &blob.public);

        if (buf.error != 0)
            goto done;

        PRINTF0("TPM2B_PUBLIC.size: ");
        PRINTF("%ld\n", (long)blob.public.size);
        PRINTF0("TPM2B_PUBLIC.publicArea:\n");
        HexDump(&blob.public.publicArea, blob.public.size);
    }

    /* Dump TPM2B_DIGEST */
    {
        _Unpack_TPM2B_DIGEST(&buf, &blob.hash);

        if (buf.error != 0)
            goto done;

        PRINTF0("TPM2B_DIGEST.size: ");
        PRINTF("%ld\n", (long)blob.hash.size);
        PRINTF0("TPM2B_DIGEST.buffer:\n");
        HexDump(blob.hash.buffer, blob.hash.size);
    }

    /* Dump TPMS_PCR_SELECT */
    {
        _Unpack_TPMS_PCR_SELECT(&buf, &blob.pcr);

        if (buf.error != 0)
            goto done;

        PRINTF0("TPMS_PCR_SELECT.size: ");
        PRINTF("%ld\n", (long)blob.pcr.sizeofSelect);
        PRINTF0("TPMS_PCR_SELECT.buffer:\n");
        HexDump(blob.pcr.pcrSelect, blob.pcr.sizeofSelect);
    }

    if (buf.error != 0)
        goto done;

    rc = TPM_RC_SUCCESS;

done:
    return rc;
}

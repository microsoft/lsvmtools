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
#ifndef _tpm2_h
#define _tpm2_h

#include "config.h"
#include "tcg2.h"
#include "inline.h"
#include "sha.h"
#include "error.h"
#include "byteorder.h"

/*
**==============================================================================
**
** TPM2_RC_*
**
**==============================================================================
*/

typedef UINT32 TPM_RC;

#define TPM_RC_1 0x100
#define TPM_RC_2 0x200
#define TPM_RC_3 0x300
#define TPM_RC_4 0x400
#define TPM_RC_5 0x500
#define TPM_RC_6 0x600
#define TPM_RC_7 0x700
#define TPM_RC_8 0x800
#define TPM_RC_9 0x900
#define TPM_RC_A 0xA00
#define TPM_RC_ASYMMETRIC 0x081
#define TPM_RC_ATTRIBUTES 0x082
#define TPM_RC_AUTH_CONTEXT 0x145
#define TPM_RC_AUTH_FAIL 0x08E
#define TPM_RC_AUTH_MISSING 0x125
#define TPM_RC_AUTHSIZE 0x144
#define TPM_RC_AUTH_TYPE 0x124
#define TPM_RC_AUTH_UNAVAILABLE 0x12F
#define TPM_RC_B 0xB00
#define TPM_RC_BAD_AUTH 0x0A2
#define TPM_RC_BAD_CONTEXT 0x150
#define TPM_RC_BAD_TAG 0x01E
#define TPM_RC_BINDING 0x0A5
#define TPM_RC_C 0xC00
#define TPM_RC_CANCELED 0x909
#define TPM_RC_COMMAND_CODE 0x143
#define TPM_RC_COMMAND_SIZE 0x142
#define TPM_RC_CONTEXT_GAP 0x901
#define TPM_RC_CPHASH 0x151
#define TPM_RC_CURVE 0x0A6
#define TPM_RC_D 0xD00
#define TPM_RC_DISABLED 0x120
#define TPM_RC_E 0xE00
#define TPM_RC_ECC_POINT 0x0A7
#define TPM_RC_EXCLUSIVE 0x121
#define TPM_RC_EXPIRED 0x0A3
#define TPM_RC_F 0xF00
#define TPM_RC_FAILURE 0x101
#define TPM_RC_H 0x000
#define TPM_RC_HANDLE 0x08B
#define TPM_RC_HASH 0x083
#define TPM_RC_HIERARCHY 0x085
#define TPM_RC_HMAC 0x119
#define TPM_RC_INITIALIZE 0x100
#define TPM_RC_INSUFFICIENT 0x09A
#define TPM_RC_INTEGRITY 0x09F
#define TPM_RC_KDF 0x08C
#define TPM_RC_KEY 0x09C
#define TPM_RC_KEY_SIZE 0x087
#define TPM_RC_LOCALITY 0x907
#define TPM_RC_LOCKOUT 0x921
#define TPM_RC_MEMORY 0x904
#define TPM_RC_MGF 0x088
#define TPM_RC_MODE 0x089
#define TPM_RC_NEEDS_TEST 0x153
#define TPM_RC_N_MASK 0xF00
#define TPM_RC_NONCE 0x08F
#define TPM_RC_NO_RESULT 0x154
#define TPM_RC_NOT_USED 0x97F
#define TPM_RC_NV_AUTHORIZATION 0x149
#define TPM_RC_NV_DEFINED 0x14C
#define TPM_RC_NV_LOCKED 0x148
#define TPM_RC_NV_RANGE 0x146
#define TPM_RC_NV_RATE 0x920
#define TPM_RC_NV_SIZE 0x147
#define TPM_RC_NV_SPACE 0x14B
#define TPM_RC_NV_UNAVAILABLE 0x923
#define TPM_RC_NV_UNINITIALIZED 0x14A
#define TPM_RC_OBJECT_HANDLES 0x906
#define TPM_RC_OBJECT_MEMORY 0x902
#define TPM_RC_P 0x040
#define TPM_RC_PARENT 0x152
#define TPM_RC_PCR 0x127
#define TPM_RC_PCR_CHANGED 0x128
#define TPM_RC_POLICY 0x126
#define TPM_RC_POLICY_CC 0x0A4
#define TPM_RC_POLICY_FAIL 0x09D
#define TPM_RC_PP 0x090
#define TPM_RC_PRIVATE 0x10B
#define TPM_RC_RANGE 0x08D
#define TPM_RC_REBOOT 0x130
#define TPM_RC_REFERENCE_H0 0x910
#define TPM_RC_REFERENCE_H1 0x911
#define TPM_RC_REFERENCE_H2 0x912
#define TPM_RC_REFERENCE_H3 0x913
#define TPM_RC_REFERENCE_H4 0x914
#define TPM_RC_REFERENCE_H5 0x915
#define TPM_RC_REFERENCE_H6 0x916
#define TPM_RC_REFERENCE_S0 0x918
#define TPM_RC_REFERENCE_S1 0x919
#define TPM_RC_REFERENCE_S2 0x91A
#define TPM_RC_REFERENCE_S3 0x91B
#define TPM_RC_REFERENCE_S4 0x91C
#define TPM_RC_REFERENCE_S5 0x91D
#define TPM_RC_REFERENCE_S6 0x91E
#define TPM_RC_RESERVED_BITS 0x0A1
#define TPM_RC_RETRY 0x922
#define TPM_RC_S 0x800
#define TPM_RC_SCHEME 0x092
#define TPM_RC_SELECTOR 0x098
#define TPM_RC_SENSITIVE 0x155
#define TPM_RC_SEQUENCE 0x103
#define TPM_RC_SESSION_HANDLES 0x905
#define TPM_RC_SESSION_MEMORY 0x903
#define TPM_RC_SIGNATURE 0x09B
#define TPM_RC_SIZE 0x095
#define TPM_RC_SUCCESS 0x000
#define TPM_RC_SYMMETRIC 0x096
#define TPM_RC_TAG 0x097
#define TPM_RC_TESTING 0x90A
#define TPM_RC_TICKET 0x0A0
#define TPM_RC_TOO_MANY_CONTEXTS 0x12E
#define TPM_RC_TYPE 0x08A
#define TPM_RC_UNBALANCED 0x131
#define TPM_RC_UPGRADE 0x12D
#define TPM_RC_VALUE 0x084
#define TPM_RC_YIELDED 0x908

/*
**==============================================================================
**
** BYTE
**
**==============================================================================
*/

typedef unsigned char BYTE;

/*
**==============================================================================
**
** TPM_HANDLE
**
**==============================================================================
*/

typedef UINT32 TPM_HANDLE;
typedef TPM_HANDLE TPMI_RH_HIERARCHY;
typedef TPM_HANDLE TPMI_RH_LOCKOUT;
typedef TPM_HANDLE TPMI_SH_AUTH_SESSION;
typedef TPM_HANDLE TPMI_DH_CONTEXT;
typedef TPM_HANDLE TPMI_DH_OBJECT;
typedef TPM_HANDLE TPMI_DH_ENTITY;
typedef TPM_HANDLE TPMI_SH_POLICY;
typedef TPM_HANDLE TPMI_DH_PCR;

/*
**==============================================================================
**
** TPM_CC (command code)
**
**==============================================================================
*/

typedef UINT32 TPM_CC;

/*
**==============================================================================
**
** TPM_CAP
**
**==============================================================================
*/

typedef UINT32 TPM_CAP;

#define TPM_CAP_FIRST 0x00000000
#define TPM_CAP_ALGS 0x00000000
#define TPM_CAP_HANDLES 0x00000001
#define TPM_CAP_COMMANDS 0x00000002
#define TPM_CAP_PP_COMMANDS 0x00000003
#define TPM_CAP_AUDIT_COMMANDS 0x00000004
#define TPM_CAP_PCRS 0x00000005
#define TPM_CAP_TPM_PROPERTIES 0x00000006
#define TPM_CAP_PCR_PROPERTIES 0x00000007
#define TPM_CAP_ECC_CURVES 0x00000008
#define TPM_CAP_LAST 0x00000008
#define TPM_CAP_VENDOR_PROPERTY 0x00000100
#define TPM_ALG_CBC 0x0042
#define TPM_ALG_MGF1 0x0007
#define TPM_ALG_KDF1_SP800_56A 0x0020
#define TPM_ALG_KDF2 0x0021
#define TPM_ALG_KDF1_SP800_108 0x0022
#define TPM_ALG_CFB 0x0043


/*
**==============================================================================
**
** TPMI_YES_NO
**
**==============================================================================
*/

typedef BYTE TPMI_YES_NO;
#define TPM_NO 0
#define TPM_YES 1

/*
**==============================================================================
**
** TPM_PT
**
**==============================================================================
*/

typedef UINT32 TPM_PT;

#define TPM_PT_NONE 0x00000000
#define PT_GROUP 0x00000100
#define PT_FIXED (PT_GROUP * 1)
#define TPM_PT_FAMILY_INDICATOR (PT_FIXED + 0)
#define TPM_PT_LEVEL (PT_FIXED + 1)
#define TPM_PT_REVISION (PT_FIXED + 2)
#define TPM_PT_DAY_OF_YEAR (PT_FIXED + 3)
#define TPM_PT_YEAR (PT_FIXED + 4)
#define TPM_PT_PCR_COUNT (PT_FIXED + 18)

/*
**==============================================================================
**
** TPMS_TAGGED_PROPERTY
**
**==============================================================================
*/

typedef struct _TPMS_TAGGED_PROPERTY TPMS_TAGGED_PROPERTY;

struct _TPMS_TAGGED_PROPERTY
{
    TPM_PT property;
    UINT32 value;
};

/*
**==============================================================================
**
** TPML_TAGGED_TPM_PROPERTY
**
**==============================================================================
*/

#define MAX_TPM_PROPERTIES 8

typedef struct _TPML_TAGGED_TPM_PROPERTY TPML_TAGGED_TPM_PROPERTY;

struct _TPML_TAGGED_TPM_PROPERTY
{
    UINT32 count;
    TPMS_TAGGED_PROPERTY tpmProperty[MAX_TPM_PROPERTIES];
};

/*
**==============================================================================
**
** TPMU_CAPABILITIES
**
**==============================================================================
*/

typedef union _TPMU_CAPABILITIES TPMU_CAPABILITIES;

union _TPMU_CAPABILITIES
{
    TPML_TAGGED_TPM_PROPERTY tpmProperties;
};

/*
**==============================================================================
**
** TPMS_CAPABILITY_DATA
**
**==============================================================================
*/

typedef struct _TPMS_CAPABILITY_DATA TPMS_CAPABILITY_DATA;

struct _TPMS_CAPABILITY_DATA
{
    TPM_CAP capability;
    TPMU_CAPABILITIES data;
};

/*
**==============================================================================
**
** TPM_ST
**
**==============================================================================
*/

typedef UINT16 TPM_ST;
typedef TPM_ST TPMI_ST_COMMAND_TAG;
#define TPM_ST_NO_SESSIONS 0x8001
#define TPM_ST_SESSIONS 0x8002

/*
**==============================================================================
**
** IMPLEMENTATION_PCR
**
**==============================================================================
*/

#define IMPLEMENTATION_PCR 24

/*
**==============================================================================
**
** TPM_GENERATED_VALUE
**
**==============================================================================
*/

#define TPM_GENERATED_VALUE 0xff544347

/*
**==============================================================================
**
** PCR_SELECT_MAX
**
**==============================================================================
*/

#define PCR_SELECT_MAX ((IMPLEMENTATION_PCR + 7) / 8)

/* We never use more than 16 PCRs */
#define MAX_PCRS 16

/*
**==============================================================================
**
** TPM_ALG_ID
**
**==============================================================================
*/

typedef UINT16 TPM_ALG_ID;
typedef TPM_ALG_ID TPMI_ALG_PUBLIC;
typedef TPM_ALG_ID TPMI_ALG_HASH;
typedef TPM_ALG_ID TPMI_ALG_KEYEDHASH_SCHEME;
typedef TPM_ALG_ID TPMI_ALG_KDF;
typedef TPM_ALG_ID TPMI_ALG_SYM_OBJECT;
typedef TPM_ALG_ID TPMI_ALG_SYM_MODE;
typedef TPM_ALG_ID TPMI_ALG_RSA_DECRYPT;
typedef TPM_ALG_ID TPMI_ALG_ECC_SCHEME;
typedef TPM_ALG_ID TPMI_ALG_ASYM_SCHEME;
typedef TPM_ALG_ID TPMI_ALG_RSA_SCHEME;
typedef TPM_ALG_ID TPMI_ALG_SYM;

/*
**==============================================================================
**
** TPM_KEY_BITS
**
**==============================================================================
*/

typedef UINT16 TPM_KEY_BITS;

/*
**==============================================================================
**
** TPMS_PCR_SELECT
**
**==============================================================================
*/

typedef struct _TPMS_PCR_SELECT TPMS_PCR_SELECT;

struct _TPMS_PCR_SELECT
{
    UINT8 sizeofSelect;
    BYTE pcrSelect[PCR_SELECT_MAX];
};

/*
**==============================================================================
**
** TPMS_PCR_SELECTION
**
**==============================================================================
*/

typedef struct _TPMS_PCR_SELECTION TPMS_PCR_SELECTION;

struct _TPMS_PCR_SELECTION
{
    TPMI_ALG_HASH hash;
    UINT8 sizeofSelect;
    BYTE pcrSelect[PCR_SELECT_MAX];
};

INLINE void TPMS_PCR_SELECTION_SelectPCR(
    TPMS_PCR_SELECTION* self,
    UINT32 n)
{
    self->pcrSelect[(n / 8)] |= (1 << (n % 8));
}

/*
**==============================================================================
**
** HASH_COUNT - the number of supported hashes
**
**==============================================================================
*/

#define HASH_COUNT 8

/*
**==============================================================================
**
** TPML_PCR_SELECTION
**
**==============================================================================
*/


typedef struct _TPML_PCR_SELECTION TPML_PCR_SELECTION;

struct _TPML_PCR_SELECTION
{
    UINT32 count;
    TPMS_PCR_SELECTION pcrSelections[HASH_COUNT];
};

/*
**==============================================================================
**
** TPM_ALG_*
**
**==============================================================================
*/

#define TPM_ALG_ERROR 0x0000
#define TPM_ALG_RSA 0x0001
#define TPM_ALG_SHA1 0x0004
#define TPM_ALG_HMAC 0x0005
#define TPM_ALG_KEYEDHASH 0x0008
#define TPM_ALG_NULL 0x0010
#define TPM_ALG_XOR 0x000A
#define TPM_ALG_AES 0x0006
#define TPM_ALG_CAMELLIA 0x0026
#define TPM_ALG_SM4 0x0013
#define TPM_ALG_SYMCIPHER 0x0025
#define TPM_ALG_ECC 0x0023
#define TPM_ALG_SHA256 0x000B
#define TPM_ALG_SHA384 0x000C
#define TPM_ALG_SHA512 0x000D
#define TPM_ALG_SM3_256 0x0012
#define TPM_ALG_ECB 0x0044

/*
**==============================================================================
**
** TPM_*_DIGEST_SIZE
**
**==============================================================================
*/

#define TPM_SHA1_DIGEST_SIZE 20
#define TPM_SHA256_DIGEST_SIZE 32
#define TPM_SM3_256_DIGEST_SIZE 32
#define TPM_SHA384_DIGEST_SIZE 48
#define TPM_SHA512_DIGEST_SIZE 64

/*
**==============================================================================
**
** TPMU_HA
**
**==============================================================================
*/

typedef union _TPMU_HA TPMU_HA;

union _TPMU_HA
{
#if 0
    BYTE sha[TPM_SHA_DIGEST_SIZE];
#endif
    BYTE sha1[TPM_SHA1_DIGEST_SIZE];
    BYTE sha256[TPM_SHA256_DIGEST_SIZE];
    BYTE sha384[TPM_SHA384_DIGEST_SIZE];
    BYTE sha512[TPM_SHA512_DIGEST_SIZE];
    BYTE sm3_256[TPM_SM3_256_DIGEST_SIZE];
};

/*
**==============================================================================
**
** TPM2B
**
**==============================================================================
*/

typedef struct _TPM2B TPM2B;

struct _TPM2B
{
    UINT16 size;
    BYTE buffer[1];
};

/*
**==============================================================================
**
** TPM2B_DIGEST
**
**==============================================================================
*/

typedef struct _TPM2B_DIGEST TPM2B_DIGEST;

struct _TPM2B_DIGEST
{
    UINT16 size;
    BYTE buffer[sizeof(TPMU_HA)];
};

/*
**==============================================================================
**
** TPML_DIGEST
**
**==============================================================================
*/

typedef struct _TPML_DIGEST TPML_DIGEST;

struct _TPML_DIGEST
{
    UINT32 count;
    TPM2B_DIGEST digests[8];
};

/*
**==============================================================================
**
** TPM2B_NONCE
**
**==============================================================================
*/

typedef TPM2B_DIGEST TPM2B_NONCE;

/*
**==============================================================================
**
** TPM_RH
**
**==============================================================================
*/

typedef TPM_HANDLE RPM_RH;

#define TPM_RH_FIRST 0x40000000
#define TPM_RH_SRK 0x40000000
#define TPM_RH_OWNER 0x40000001
#define TPM_RH_REVOKE 0x40000002
#define TPM_RH_TRANSPORT 0x40000003
#define TPM_RH_OPERATOR 0x40000004
#define TPM_RH_ADMIN 0x40000005
#define TPM_RH_EK 0x40000006
#define TPM_RH_NULL 0x40000007
#define TPM_RH_UNASSIGNED 0x40000008
#define TPM_RS_PW 0x40000009
#define TPM_RH_LOCKOUT 0x4000000A
#define TPM_RH_ENDORSEMENT 0x4000000B
#define TPM_RH_PLATFORM 0x4000000C
#define TPM_RH_PLATFORM_NV 0x4000000D
#define TPM_RH_AUTH_00 0x40000010
#define TPM_RH_AUTH_FF 0x4000010F
#define TPM_RH_LAST 0x4000010F

/*
**==============================================================================
**
** TPM_SE
**
**==============================================================================
*/

typedef UINT8 TPM_SE;
#define TPM_SE_HMAC 0x00
#define TPM_SE_POLICY 0x01
#define TPM_SE_TRIAL 0x03

/*
**==============================================================================
**
** TPMA_SESSION
**
**==============================================================================
*/

typedef struct _TPMA_SESSION TPMA_SESSION;

struct _TPMA_SESSION
{
    unsigned int continueSession:1;
    unsigned int auditExclusive:1;
    unsigned int auditReset:1;
    unsigned int reserved1:2;
    unsigned int decrypt:1;
    unsigned int encrypt:1;
    unsigned int audit:1;
    unsigned int reserved:24;
};

/*
**==============================================================================
**
** TPM2B_AUTH
**
**==============================================================================
*/

typedef TPM2B_DIGEST TPM2B_AUTH;

/*
**==============================================================================
**
** TPMS_AUTH_COMMAND
**
**==============================================================================
*/

typedef struct _TPMS_AUTH_COMMAND TPMS_AUTH_COMMAND;

struct _TPMS_AUTH_COMMAND
{
    TPMI_SH_AUTH_SESSION sessionHandle;
    TPM2B_NONCE nonce;
    TPMA_SESSION sessionAttributes;
    TPM2B_AUTH hmac;
};

/*
**==============================================================================
**
** TPMS_AUTH_RESPONSE
**
**==============================================================================
*/

typedef struct _TPMS_AUTH_RESPONSE TPMS_AUTH_RESPONSE;

struct _TPMS_AUTH_RESPONSE
{
    TPM2B_NONCE nonce;
    TPMA_SESSION sessionAttributes;
    TPM2B_AUTH hmac;
};

/*
**==============================================================================
**
** TPM2B_SENSITIVE_DATA
**
**==============================================================================
*/

#define MAX_SYM_DATA 128

typedef struct _TPM2B_SENSITIVE_DATA TPM2B_SENSITIVE_DATA;

struct _TPM2B_SENSITIVE_DATA
{
    UINT16 size;
    BYTE buffer[MAX_SYM_DATA];
};

/*
**==============================================================================
**
** TPMS_SENSITIVE_CREATE
**
**==============================================================================
*/

typedef struct _TPMS_SENSITIVE_CREATE TPMS_SENSITIVE_CREATE;

struct _TPMS_SENSITIVE_CREATE
{
    TPM2B_AUTH userAuth;
    TPM2B_SENSITIVE_DATA data;
};

/*
**==============================================================================
**
** TPM2B_SENSITIVE_CREATE
**
**==============================================================================
*/

typedef struct _TPM2B_SENSITIVE_CREATE TPM2B_SENSITIVE_CREATE;

struct _TPM2B_SENSITIVE_CREATE
{
    UINT16 size;
    TPMS_SENSITIVE_CREATE sensitive;
};

/*
**==============================================================================
**
** TPMA_OBJECT
**
**==============================================================================
*/

typedef struct _TPMA_OBJECT TPMA_OBJECT;

struct _TPMA_OBJECT
{
    unsigned int reserved1:1;
    unsigned int fixedTPM:1;
    unsigned int stClear:1;
    unsigned int reserved2:1;
    unsigned int fixedParent:1;
    unsigned int sensitiveDataOrigin:1;
    unsigned int userWithAuth:1;
    unsigned int adminWithPolicy:1;
    unsigned int reserved3:2;
    unsigned int noDA:1;
    unsigned int encryptedDuplication:1;
    unsigned int reserved4:4;
    unsigned int restricted:1;
    unsigned int decrypt:1;
    unsigned int sign:1;
    unsigned int reserved5:13;
};

/*
**==============================================================================
**
** TPMS_SCHEME_HASH
**
**==============================================================================
*/

typedef struct _TPMS_SCHEME_HASH TPMS_SCHEME_HASH;

struct _TPMS_SCHEME_HASH
{
    TPMI_ALG_HASH hashAlg;
};

typedef TPMS_SCHEME_HASH TPMS_KEY_SCHEME_ECDH;
typedef TPMS_SCHEME_HASH TPMS_KEY_SCHEME_ECMQV;
typedef TPMS_SCHEME_HASH TPMS_SIG_SCHEME_RSASSA;
typedef TPMS_SCHEME_HASH TPMS_SIG_SCHEME_RSAPSS;
typedef TPMS_SCHEME_HASH TPMS_SIG_SCHEME_ECDSA;
typedef TPMS_SCHEME_HASH TPMS_SIG_SCHEME_ECDAA;
typedef TPMS_SCHEME_HASH TPMS_SIG_SCHEME_SM2;
typedef TPMS_SCHEME_HASH TPMS_SIG_SCHEME_ECSCHNORR;
typedef TPMS_SCHEME_HASH TPMS_ENC_SCHEME_RSAES;
typedef TPMS_SCHEME_HASH TPMS_ENC_SCHEME_OAEP;
typedef TPMS_SCHEME_HASH TPMS_SCHEME_KDF2;
typedef TPMS_SCHEME_HASH TPMS_SCHEME_MGF1;
typedef TPMS_SCHEME_HASH TPMS_SCHEME_KDF1_SP800_56A;
typedef TPMS_SCHEME_HASH TPMS_SCHEME_KDF2;
typedef TPMS_SCHEME_HASH TPMS_SCHEME_KDF1_SP800_108;

/*
**==============================================================================
**
** TPMS_SCHEME_HMAC
**
**==============================================================================
*/

typedef TPMS_SCHEME_HASH TPMS_SCHEME_HMAC;

/*
**==============================================================================
**
** TPMS_SCHEME_XOR
**
**==============================================================================
*/

typedef struct _TPMS_SCHEME_XOR TPMS_SCHEME_XOR;

struct _TPMS_SCHEME_XOR
{
    TPMI_ALG_HASH hashAlg;
    TPMI_ALG_KDF kdf;
};

/*
**==============================================================================
**
** TPMU_SCHEME_KEYEDHASH
**
**==============================================================================
*/

typedef union _TPMU_SCHEME_KEYEDHASH TPMU_SCHEME_KEYEDHASH;

union _TPMU_SCHEME_KEYEDHASH
{
    TPMS_SCHEME_HMAC hmac;
    TPMS_SCHEME_XOR exclusiveOr;
};

/*
**==============================================================================
**
** TPMT_KEYEDHASH_SCHEME
**
**==============================================================================
*/

typedef struct _TPMT_KEYEDHASH_SCHEME TPMT_KEYEDHASH_SCHEME;

struct _TPMT_KEYEDHASH_SCHEME
{
    TPMI_ALG_KEYEDHASH_SCHEME scheme;
    TPMU_SCHEME_KEYEDHASH details;
};

/*
**==============================================================================
**
** TPMS_KEYEDHASH_PARMS
**
**==============================================================================
*/

typedef struct _TPMS_KEYEDHASH_PARMS TPMS_KEYEDHASH_PARMS;

struct _TPMS_KEYEDHASH_PARMS
{
    TPMT_KEYEDHASH_SCHEME scheme;
};

/*
**==============================================================================
**
** TPMU_SYM_KEY_BITS
**
**==============================================================================
*/

typedef union _TPMU_SYM_KEY_BITS TPMU_SYM_KEY_BITS;

union _TPMU_SYM_KEY_BITS
{
    TPM_KEY_BITS aes;
    TPM_KEY_BITS exclusiveOr;
    TPM_KEY_BITS sm4;
    TPM_KEY_BITS camellia;
};

/*
**==============================================================================
**
** TPMU_SYM_MODE
**
**==============================================================================
*/

typedef union _TPMU_SYM_MODE TPMU_SYM_MODE;

union _TPMU_SYM_MODE
{
    TPMI_ALG_SYM_MODE aes;
    TPMI_ALG_SYM_MODE sm4;
    TPMI_ALG_SYM_MODE camellia;
    TPMI_ALG_SYM_MODE sym;
};

/*
**==============================================================================
**
** TPMT_SYM_DEF_OBJECT
**
**==============================================================================
*/

typedef struct _TPMT_SYM_DEF_OBJECT TPMT_SYM_DEF_OBJECT;

struct _TPMT_SYM_DEF_OBJECT
{
    TPMI_ALG_SYM_OBJECT algorithm;
    TPMU_SYM_KEY_BITS keyBits;
    TPMU_SYM_MODE mode;
};

/*
**==============================================================================
**
** TPMS_SYMCIPHER_PARMS
**
**==============================================================================
*/

typedef struct _TPMS_SYMCIPHER_PARMS TPMS_SYMCIPHER_PARMS;

struct _TPMS_SYMCIPHER_PARMS
{
    TPMT_SYM_DEF_OBJECT sym;
};

/*
**==============================================================================
**
** TPMU_ASYM_SCHEME
**
**==============================================================================
*/

typedef union _TPMU_ASYM_SCHEME TPMU_ASYM_SCHEME;

union _TPMU_ASYM_SCHEME
{
    TPMS_KEY_SCHEME_ECDH ecdh;
    TPMS_KEY_SCHEME_ECMQV ecmqv;
    TPMS_SIG_SCHEME_RSASSA rsassa;
    TPMS_SIG_SCHEME_RSAPSS rsapss;
    TPMS_SIG_SCHEME_ECDSA ecdsa;
    TPMS_SIG_SCHEME_ECDAA ecdaa;
    TPMS_SIG_SCHEME_SM2 sm2;
    TPMS_SIG_SCHEME_ECSCHNORR ecschnorr;
    TPMS_ENC_SCHEME_RSAES rsaes;
    TPMS_ENC_SCHEME_OAEP oaep;
    TPMS_SCHEME_HASH anySig;
    unsigned char padding[4];
};

/*
**==============================================================================
**
** TPMT_RSA_SCHEME
**
**==============================================================================
*/

typedef struct _TPMT_RSA_SCHEME TPMT_RSA_SCHEME;

struct _TPMT_RSA_SCHEME
{
    TPMI_ALG_RSA_SCHEME scheme;
    TPMU_ASYM_SCHEME details;
};

/*
**==============================================================================
**
** TPMS_RSA_PARMS
**
**==============================================================================
*/

typedef struct _TPMS_RSA_PARMS TPMS_RSA_PARMS;

struct _TPMS_RSA_PARMS
{
    TPMT_SYM_DEF_OBJECT symmetric;
    TPMT_RSA_SCHEME scheme;
    TPM_KEY_BITS keyBits;
    UINT32 exponent;
};

/*
**==============================================================================
**
** TPMT_ECC_SCHEME
**
**==============================================================================
*/

typedef struct _TPMT_ECC_SCHEME TPMT_ECC_SCHEME;

struct _TPMT_ECC_SCHEME
{
    TPMI_ALG_ECC_SCHEME scheme;
    TPMU_ASYM_SCHEME details;
};

/*
**==============================================================================
**
** TPM_ECC_CURVE
**
**==============================================================================
*/
typedef  UINT16 TPM_ECC_CURVE;
typedef TPM_ECC_CURVE TPMI_ECC_CURVE;

/*
**==============================================================================
**
** TPMU_KDF_SCHEME
**
**==============================================================================
*/

typedef union _TPMU_KDF_SCHEME TPMU_KDF_SCHEME;

union _TPMU_KDF_SCHEME
{
    TPMS_SCHEME_MGF1 mgf1;
    TPMS_SCHEME_KDF1_SP800_56A kdf1_sp800_56a;
    TPMS_SCHEME_KDF2 kdf2;
    TPMS_SCHEME_KDF1_SP800_108 kdf1_sp800_108;
};

/*
**==============================================================================
**
** TPMT_KDF_SCHEME
**
**==============================================================================
*/

typedef struct _TPMT_KDF_SCHEME TPMT_KDF_SCHEME;

struct _TPMT_KDF_SCHEME
{
    TPMI_ALG_KDF scheme;
    TPMU_KDF_SCHEME details;
};

/*
**==============================================================================
**
** TPMS_ECC_PARMS
**
**==============================================================================
*/

typedef struct _TPMS_ECC_PARMS TPMS_ECC_PARMS;

struct _TPMS_ECC_PARMS
{
    TPMT_SYM_DEF_OBJECT symmetric;
    TPMT_ECC_SCHEME scheme;
    TPMI_ECC_CURVE curveID;
    TPMT_KDF_SCHEME kdf;
};

/*
**==============================================================================
**
** TPMT_ASYM_SCHEME
**
**==============================================================================
*/

typedef struct _TPMT_ASYM_SCHEME TPMT_ASYM_SCHEME;

struct _TPMT_ASYM_SCHEME
{
    TPMI_ALG_ASYM_SCHEME scheme;
    TPMU_ASYM_SCHEME details;
};

/*
**==============================================================================
**
** TPMS_ASYM_PARMS
**
**==============================================================================
*/

typedef struct _TPMS_ASYM_PARMS TPMS_ASYM_PARMS;

struct _TPMS_ASYM_PARMS
{
    TPMT_SYM_DEF_OBJECT symmetric;
    TPMT_ASYM_SCHEME scheme;
};

/*
**==============================================================================
**
** TPMU_PUBLIC_PARMS
**
**==============================================================================
*/

typedef union _TPMU_PUBLIC_PARMS TPMU_PUBLIC_PARMS;

union _TPMU_PUBLIC_PARMS
{
    TPMS_KEYEDHASH_PARMS keyedHashDetail;
    TPMS_SYMCIPHER_PARMS symDetail;
    TPMS_RSA_PARMS rsaDetail;
    TPMS_ECC_PARMS eccDetail;
    TPMS_ASYM_PARMS asymDetail;
};

/*
**==============================================================================
**
** TPM2B_PUBLIC_KEY_RSA
**
**==============================================================================
*/

#define MAX_RSA_KEY_BYTES 256

typedef struct _TPM2B_PUBLIC_KEY_RSA TPM2B_PUBLIC_KEY_RSA;

struct _TPM2B_PUBLIC_KEY_RSA
{
    UINT16 size;
    BYTE buffer[MAX_RSA_KEY_BYTES];
};

/*
**==============================================================================
**
** TPM2B_ECC_PARAMETER
**
**==============================================================================
*/

#define MAX_ECC_KEY_BYTES 48

typedef struct _TPM2B_ECC_PARAMETER TPM2B_ECC_PARAMETER;

struct _TPM2B_ECC_PARAMETER
{
    UINT16 size;
    BYTE buffer[MAX_ECC_KEY_BYTES];
};

/*
**==============================================================================
**
** TPMS_ECC_POINT
**
**==============================================================================
*/

typedef struct _TPMS_ECC_POINT TPMS_ECC_POINT;

struct _TPMS_ECC_POINT
{
    TPM2B_ECC_PARAMETER x;
    TPM2B_ECC_PARAMETER y;
};

/*
**==============================================================================
**
** TPMU_ENCRYPTED_SECRET
**
**==============================================================================
*/

typedef union _TPMU_ENCRYPTED_SECRET TPMU_ENCRYPTED_SECRET;

union _TPMU_ENCRYPTED_SECRET
{
    BYTE ecc[sizeof(TPMS_ECC_POINT)];
    BYTE rsa[MAX_RSA_KEY_BYTES];
    BYTE symmetric[sizeof(TPM2B_DIGEST)];
    BYTE keyedHash[sizeof(TPM2B_DIGEST)];
};

/*
**==============================================================================
**
** TPM2B_ENCRYPTED_SECRET
**
**==============================================================================
*/

typedef struct _TPM2B_ENCRYPTED_SECRET TPM2B_ENCRYPTED_SECRET;

struct _TPM2B_ENCRYPTED_SECRET
{
    UINT16 size;
    BYTE secret[sizeof(TPMU_ENCRYPTED_SECRET)];
};

/*
**==============================================================================
**
** TPMU_PUBLIC_ID
**
**==============================================================================
*/

typedef union _TPMU_PUBLIC_ID TPMU_PUBLIC_ID;

union _TPMU_PUBLIC_ID
{
    TPM2B_DIGEST keyedHash;
    TPM2B_DIGEST sym;
    TPM2B_PUBLIC_KEY_RSA rsa;
    TPMS_ECC_POINT ecc;
};

/*
**==============================================================================
**
** TPMT_PUBLIC
**
**==============================================================================
*/

typedef struct _TPMT_PUBLIC TPMT_PUBLIC;

struct _TPMT_PUBLIC 
{
    TPMI_ALG_PUBLIC type;
    TPMI_ALG_HASH nameAlg;
    TPMA_OBJECT objectAttributes;
    TPM2B_DIGEST authPolicy;
    TPMU_PUBLIC_PARMS parameters;
    TPMU_PUBLIC_ID unique;
};

/*
**==============================================================================
**
** TPM2B_PUBLIC
**
**==============================================================================
*/

typedef struct _TPM2B_PUBLIC TPM2B_PUBLIC;

struct _TPM2B_PUBLIC
{
    UINT16 size;
    TPMT_PUBLIC publicArea;
};

/*
**==============================================================================
**
** TPMT_HA
**
**==============================================================================
*/

typedef struct _TPMT_HA TPMT_HA;

struct _TPMT_HA
{
    TPMI_ALG_HASH hashAlg;
    TPMU_HA digest;
};

/*
**==============================================================================
**
** TPM2B_DATA
**
**==============================================================================
*/

typedef struct _TPM2B_DATA TPM2B_DATA;

struct _TPM2B_DATA
{
    UINT16 size;
    BYTE buffer[sizeof(TPMT_HA)];
};

/*
**==============================================================================
**
** TPMA_LOCALITY
**
**==============================================================================
*/

typedef struct _TPMA_LOCALITY TPMA_LOCALITY;

struct _TPMA_LOCALITY
{
    unsigned char TPM_LOC_ZERO:1;
    unsigned char TPM_LOC_ONE:1;
    unsigned char TPM_LOC_TWO:1;
    unsigned char TPM_LOC_THREE:1;
    unsigned char TPM_LOC_FOUR:1;
    unsigned char Extended:3;
};

/*
**==============================================================================
**
** TPMU_NAME
**
**==============================================================================
*/

typedef union _TPMU_NAME TPMU_NAME;

union _TPMU_NAME
{
    TPMT_HA digest;
    TPM_HANDLE handle;
};

/*
**==============================================================================
**
** TPM2B_NAME
**
**==============================================================================
*/

typedef struct _TPM2B_NAME TPM2B_NAME;

struct _TPM2B_NAME
{
    UINT16 size;
    BYTE name[sizeof(TPMU_NAME)];
};

/*
**==============================================================================
**
** TPMS_CREATION_DATA
**
**==============================================================================
*/

typedef struct _TPMS_CREATION_DATA TPMS_CREATION_DATA;

struct _TPMS_CREATION_DATA
{
    TPML_PCR_SELECTION pcrSelect;
    TPM2B_DIGEST pcrDigest;
    TPMA_LOCALITY locality;
    TPM_ALG_ID parentNameAlg;
    TPM2B_NAME parentName;
    TPM2B_NAME parentQualifiedName;
    TPM2B_DATA outsideInfo;
};

/*
**==============================================================================
**
** TPM2B_CREATION_DATA
**
**==============================================================================
*/

typedef struct _TPM2B_CREATION_DATA TPM2B_CREATION_DATA;

struct _TPM2B_CREATION_DATA
{
    UINT16 size;
    TPMS_CREATION_DATA creationData;
};

/*
**==============================================================================
**
** TPMT_SYM_DEF
**
**==============================================================================
*/

typedef struct _TPMT_SYM_DEF TPMT_SYM_DEF;

struct _TPMT_SYM_DEF
{
    TPMI_ALG_SYM algorithm;
    TPMU_SYM_KEY_BITS keyBits;
    TPMU_SYM_MODE mode;
};

/*
**==============================================================================
**
** TPM2B_MAX_BUFFER
**
**==============================================================================
*/

#define MAX_DIGEST_BUFFER 1024

typedef struct _TPM2B_MAX_BUFFER TPM2B_MAX_BUFFER;

struct _TPM2B_MAX_BUFFER
{
    UINT16 size;
    BYTE buffer[MAX_DIGEST_BUFFER];
};

/*
**==============================================================================
**
** TPMT_TK_HASHCHECK
**
**==============================================================================
*/

typedef struct _TPMT_TK_HASHCHECK TPMT_TK_HASHCHECK;

struct _TPMT_TK_HASHCHECK
{
    TPM_ST tag;
    TPMI_RH_HIERARCHY hierarchy;
    TPM2B_DIGEST digest;
};

/*
**==============================================================================
**
** TPM2B_SYM_KEY
**
**==============================================================================
*/

#define MAX_SYM_KEY_BYTES 32

typedef struct _TPM2B_SYM_KEY TPM2B_SYM_KEY;

struct _TPM2B_SYM_KEY
{
    UINT16 size;
    BYTE buffer[MAX_SYM_KEY_BYTES];
};

/*
**==============================================================================
**
** TPM2B_PRIVATE_KEY_RSA
**
**==============================================================================
*/

#define MAX_RSA_KEY_BYTES 256

typedef struct _TPM2B_PRIVATE_KEY_RSA TPM2B_PRIVATE_KEY_RSA;

struct _TPM2B_PRIVATE_KEY_RSA
{
    UINT16 size;
    BYTE buffer[MAX_RSA_KEY_BYTES/2];
};

/*
**==============================================================================
**
** TPM2B_PRIVATE_VENDOR_SPECIFIC
**
**==============================================================================
*/

#define PRIVATE_VENDOR_SPECIFIC_BYTES 640

typedef struct _TPM2B_PRIVATE_VENDOR_SPECIFIC TPM2B_PRIVATE_VENDOR_SPECIFIC;

struct _TPM2B_PRIVATE_VENDOR_SPECIFIC
{
    UINT16 size;
    BYTE buffer[PRIVATE_VENDOR_SPECIFIC_BYTES];
};

/*
**==============================================================================
**
** TPMU_SENSITIVE_COMPOSITE
**
**==============================================================================
*/

typedef union _TPMU_SENSITIVE_COMPOSITE TPMU_SENSITIVE_COMPOSITE;

union _TPMU_SENSITIVE_COMPOSITE
{
    TPM2B_PRIVATE_KEY_RSA rsa;
    TPM2B_ECC_PARAMETER ecc;
    TPM2B_SENSITIVE_DATA bits;
    TPM2B_SYM_KEY sym;
    TPM2B_PRIVATE_VENDOR_SPECIFIC any;
};

/*
**==============================================================================
**
** TPMT_SENSITIVE
**
**==============================================================================
*/

typedef struct _TPMT_SENSITIVE TPMT_SENSITIVE;

struct _TPMT_SENSITIVE
{
    TPMI_ALG_PUBLIC sensitiveType;
    TPM2B_AUTH authValue;
    TPM2B_DIGEST seedValue;
    TPMU_SENSITIVE_COMPOSITE sensitive;
};

/*
**==============================================================================
**
** TPM2B_SENSITIVE
**
**==============================================================================
*/

typedef struct _TPM2B_SENSITIVE TPM2B_SENSITIVE;

struct _TPM2B_SENSITIVE
{
    UINT16 size;
    TPMT_SENSITIVE sensitiveArea;
};

/*
**==============================================================================
**
** _PRIVATE
**
**==============================================================================
*/

typedef struct __PRIVATE _PRIVATE;

struct __PRIVATE
{
    TPM2B_DIGEST integrityOuter;
    TPM2B_DIGEST integrityInner;
    TPM2B_SENSITIVE sensitive;
};

/*
**==============================================================================
**
** TPM2B_PRIVATE
**
**==============================================================================
*/

typedef struct _TPM2B_PRIVATE TPM2B_PRIVATE;

struct _TPM2B_PRIVATE
{
    UINT16 size;
    BYTE buffer[sizeof(_PRIVATE)];
};

/*
**==============================================================================
**
** TPML_DIGEST_VALUES
**
**==============================================================================
*/

typedef struct _TPML_DIGEST_VALUES TPML_DIGEST_VALUES;

struct _TPML_DIGEST_VALUES
{
    UINT16 count;
    TPMT_HA digests[HASH_COUNT];
};

/*
**==============================================================================
**
** TPM2_GetCapability()
**
**==============================================================================
*/

#define TPM_CC_GetCapability 0x0000017A

TPM_RC TPM2_GetCapability(
    EFI_TCG2_PROTOCOL *protocol,
    IN const TPMS_AUTH_COMMAND *authCommand, /* Ignored */
    IN TPM_CAP capability, /* Only TPM_CAP_TPM_PROPERTIES supported */
    IN UINT32 property,
    IN UINT32 propertyCount,
    OUT TPMI_YES_NO *moreData,
    OUT TPMS_CAPABILITY_DATA *capabilityData,
    OUT TPMS_AUTH_RESPONSE *authResponse); /* Ignored */

/*
**==============================================================================
**
** TPM2_PCR_Read()
**
**==============================================================================
*/

#define  TPM_CC_PCR_Read 0x0000017e

TPM_RC TPM2_PCR_Read(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN const TPMS_AUTH_COMMAND *authCommand, /* Ignored */
    IN TPML_PCR_SELECTION  *pcrSelectionIn,
    OUT UINT32 *pcrUpdateCounter,
    OUT TPML_PCR_SELECTION *pcrSelectionOut,
    OUT TPML_DIGEST *pcrValues,
    OUT TPMS_AUTH_RESPONSE *authResponse); /* Ignored */

/*
**==============================================================================
**
** TPM2_DictionaryAttackLockReset()
**
**==============================================================================
*/

#define TPM_CC_DictionaryAttackLockReset 0x00000139

TPM_RC TPM2_DictionaryAttackLockReset(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_RH_LOCKOUT lockHandle,
    IN const TPMS_AUTH_COMMAND *authCommand,
    OUT TPMS_AUTH_RESPONSE *authResponse);

/*
**==============================================================================
**
** Tss2_Sys_DictionaryAttackParameters()
**
**==============================================================================
*/

#define TPM_CC_DictionaryAttackParameters 0x0000013a

TPM_RC TPM2_DictionaryAttackParameters(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_RH_LOCKOUT lockHandle,
    IN const TPMS_AUTH_COMMAND *authCommand,
    IN UINT32 newMaxTries,
    IN UINT32 newRecoveryTime,
    IN UINT32 lockoutRecovery,
    OUT TPMS_AUTH_RESPONSE *authResponse);

/*
**==============================================================================
**
** TPMT_TK_CREATION
**
**==============================================================================
*/

typedef struct _TPMT_TK_CREATION TPMT_TK_CREATION;

struct _TPMT_TK_CREATION
{
    TPM_ST tag;
    TPMI_RH_HIERARCHY hierarchy;
    TPM2B_DIGEST digest;
};

/*
**==============================================================================
**
** TPM2B_IV
**
**==============================================================================
*/

#define MAX_SYM_BLOCK_SIZE 16

typedef struct _TPM2B_IV TPM2B_IV;

struct _TPM2B_IV
{
    UINT16 size;
    BYTE buffer[MAX_SYM_BLOCK_SIZE];
};

/*
**==============================================================================
**
** TPM2_CreatePrimary()
**
**==============================================================================
*/

#define  TPM_CC_CreatePrimary 0x00000131

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
    OUT TPMS_AUTH_RESPONSE *authResponse);

/*
**==============================================================================
**
** TPM2_FlushContext()
**
**==============================================================================
*/

#define TPM_CC_FlushContext 0x00000165

TPM_RC TPM2_FlushContext(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_DH_CONTEXT flushHandle);

/*
**==============================================================================
**
** TPM2_StartAuthSession()
**
**==============================================================================
*/

#define TPM_CC_StartAuthSession 0x00000176

TPM_RC TPM2_StartAuthSession(
    IN EFI_TCG2_PROTOCOL *protocol,
    /* request handles */
    IN TPMI_DH_OBJECT tpmKey,
    IN TPMI_DH_ENTITY bind,
    /* request paraqmeters */
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
    OUT TPMS_AUTH_RESPONSE *authResponse);

/*
**==============================================================================
**
** TPM2_PolicyPassword()
**
**==============================================================================
*/

#define TPM_CC_PolicyPassword 0x0000018c

TPM_RC TPM2_PolicyPassword(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_SH_POLICY policySession,
    IN const TPMS_AUTH_COMMAND *authCommand,
    OUT TPMS_AUTH_RESPONSE *authResponse);

/*
**==============================================================================
**
** TPM2_PolicyPCR()
**
**==============================================================================
*/

#define TPM_CC_PolicyPCR 0x0000017f

TPM_RC TPM2_PolicyPCR(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_SH_POLICY policySession,
    IN const TPMS_AUTH_COMMAND *authCommand,
    IN TPM2B_DIGEST *pcrDigest,
    IN TPML_PCR_SELECTION *pcrs,
    OUT TPMS_AUTH_RESPONSE *authResponse);

/*
**==============================================================================
**
** TPM2_Hash()
**
**==============================================================================
*/

#define TPM_CC_Hash 0x0000017d

TPM_RC TPM2_Hash(
    IN EFI_TCG2_PROTOCOL *protocol,
    const TPMS_AUTH_COMMAND* authCommand,
    IN TPM2B_MAX_BUFFER *data,
    IN TPMI_ALG_HASH hashAlg,
    IN TPMI_RH_HIERARCHY hierarchy,
    OUT TPM2B_DIGEST *outHash,
    OUT TPMT_TK_HASHCHECK *validation,
    OUT TPMS_AUTH_RESPONSE *authResponse);

/*
**==============================================================================
**
** TPM2_PolicyGetDigest()
**
**==============================================================================
*/

#define TPM_CC_PolicyGetDigest 0x00000189

TPM_RC TPM2_PolicyGetDigest(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_SH_POLICY policySession,
    IN const TPMS_AUTH_COMMAND *authCommand,
    OUT TPM2B_DIGEST *policyDigest,
    OUT TPMS_AUTH_RESPONSE *authResponse);

/*
**==============================================================================
**
** TPM2_Create()
**
**==============================================================================
*/

#define TPM_CC_Create 0x00000153

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
    OUT TPMS_AUTH_RESPONSE *authResponse);

/*
**==============================================================================
**
** TPM2_Load()
**
**==============================================================================
*/

#define TPM_CC_Load 0x00000157

TPM_RC TPM2_Load(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_DH_OBJECT parentHandle,
    IN TPMS_AUTH_COMMAND const *authCommand,
    IN TPM2B_PRIVATE *inPrivate,
    IN TPM2B_PUBLIC *inPublic,
    OUT TPM_HANDLE *objectHandle,
    OUT TPM2B_NAME *name,
    OUT TPMS_AUTH_RESPONSE *authResponse);

/*
**==============================================================================
**
** TPM2_Unseal()
**
**==============================================================================
*/

#define TPM_CC_Unseal 0x0000015e

TPM_RC TPM2_Unseal(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_DH_OBJECT itemHandle,
    IN const TPMS_AUTH_COMMAND *authCommand,
    OUT TPM2B_SENSITIVE_DATA *outData,
    OUT TPMS_AUTH_RESPONSE *authResponse);

/*
**==============================================================================
**
** TPM2_PCR_Extend()
**
**==============================================================================
*/

#define TPM_CC_PCR_Extend 0x00000182

TPM_RC TPM2_PCR_Extend(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_DH_PCR pcrHandle,
    IN const TPMS_AUTH_COMMAND *authCommand,
    IN TPML_DIGEST_VALUES *digests,
    OUT TPMS_AUTH_RESPONSE *authResponse);

/*
**==============================================================================
**
** TPM2_EncryptDecrypt()
**
**==============================================================================
*/

#define TPM_CC_EncryptDecrypt 0x00000164

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
    OUT TPMS_AUTH_RESPONSE *authResponse);

/*
**==============================================================================
**
** TPM2_LoadExternal()
**
**==============================================================================
*/

#define TPM_CC_LoadExternal 0x00000167

TPM_RC TPM2_LoadExternal(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN const TPMS_AUTH_COMMAND *authCommand,
    IN TPM2B_SENSITIVE *inPrivate,
    IN TPM2B_PUBLIC *inPublic,
    IN TPMI_RH_HIERARCHY hierarchy,
    OUT TPM_HANDLE *objectHandle,
    OUT TPM2B_NAME *name,
    OUT TPMS_AUTH_RESPONSE *authResponse);

/*
**==============================================================================
**
** TPM2_HashSequenceStart()
**
**==============================================================================
*/

#define TPM_CC_HashSequenceStart 0x00000186

TPM_RC TPM2_HashSequenceStart(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN const TPMS_AUTH_COMMAND* authCommand,
    IN TPM2B_AUTH *auth,
    IN TPMI_ALG_HASH hashAlg,
    OUT TPMI_DH_OBJECT *sequenceHandle,
    OUT TPMS_AUTH_RESPONSE *authResponse);

/*
**==============================================================================
**
** TPM2_SequenceUpdate()
**
**==============================================================================
*/

#define TPM_CC_SequenceUpdate 0x0000015c

TPM_RC TPM2_SequenceUpdate(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_DH_OBJECT sequenceHandle,
    IN const TPMS_AUTH_COMMAND* authCommand,
    IN TPM2B_MAX_BUFFER *buffer,
    OUT TPMS_AUTH_RESPONSE *authResponse);

/*
**==============================================================================
**
** TPM2_SequenceComplete()
**
**==============================================================================
*/

#define TPM_CC_SequenceComplete 0x0000013e

TPM_RC TPM2_SequenceComplete(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_DH_OBJECT sequenceHandle,
    IN const TPMS_AUTH_COMMAND* authCommand,
    IN TPM2B_MAX_BUFFER *buffer,
    IN TPMI_RH_HIERARCHY hierarchy,
    OUT TPM2B_DIGEST *result,
    OUT TPMT_TK_HASHCHECK *validation,
    OUT TPMS_AUTH_RESPONSE *authResponse);


/*
**==============================================================================
**
** TPM2_ReadPublic()
**
**==============================================================================
*/

#define TPM_CC_ReadPublic 0x00000173

TPM_RC TPM2_ReadPublic(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_DH_OBJECT objectHandle,
    IN const TPMS_AUTH_COMMAND* authCommand,
    OUT TPM2B_PUBLIC *outPublic);

/*
**==============================================================================
**
** TPM2 Extras:
**
**==============================================================================
*/

typedef struct _TPM2X_BLOB {
    TPM2B_PRIVATE private;
    TPM2B_PUBLIC  public;
    TPM2B_DIGEST  hash;
    TPMS_PCR_SELECT pcr;
} TPM2X_BLOB;

#define TPM2X_BLOB_SIZE sizeof(TPM2X_BLOB)

#define TPM2X_MAX_DATA_SIZE \
    sizeof(((TPM2B_SENSITIVE_CREATE*)0)->sensitive.data.buffer)

#define TPM2X_ERROR_SIZE 1024

/* Well known SRK handle from Windows. */
#define TPM2X_SRK_HANDLE 0x81000001

TPM_RC TPM2X_SetDictionaryAttackLockReset(
    EFI_TCG2_PROTOCOL *protocol);

TPM_RC TPM2X_SetLockoutParams(
    EFI_TCG2_PROTOCOL *protocol);

TPM_RC TPM2X_GetTPMRevision(
    IN EFI_TCG2_PROTOCOL *protocol,
    OUT UINT32 *revision);

TPM_RC TPM2X_GetPCRCount(
    IN EFI_TCG2_PROTOCOL *protocol,
    OUT UINT32* pcrCount);

TPM_RC TPM2X_CreateSRKKey(
    EFI_TCG2_PROTOCOL *protocol,
    TPM_HANDLE *srkHandle);

TPM_RC TPM2X_SetLockoutParams(
    EFI_TCG2_PROTOCOL *protocol);

TPM_RC TPM2X_Hash(
    EFI_TCG2_PROTOCOL *protocol,
    TPMI_ALG_HASH hashAlg,
    const BYTE *data,
    UINT16 size,
    TPM2B_DIGEST *result);

TPM_RC TPM2X_StartAuthSession( 
    EFI_TCG2_PROTOCOL *protocol,
    BOOLEAN trial,
    TPMI_ALG_HASH hashAlg,
    TPMI_SH_AUTH_SESSION *policyHandle);

TPM_RC TPM2X_BuildPolicy(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_SH_AUTH_SESSION policySession,
    IN BOOLEAN predictive,
    IN UINT32 pcrMask,
    IN const SHA256Hash pcrs[MAX_PCRS], /* Unused if !predictive */
    TPMI_ALG_HASH hashAlg,
    Error* err);

TPM_RC TPM2X_Seal(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPM_HANDLE parentKey,
    IN BOOLEAN predictive,
    IN UINT32 pcrMask,
    IN const SHA256Hash pcrs[MAX_PCRS],
    IN const BYTE data[TPM2X_MAX_DATA_SIZE],
    IN UINT16 size,
    OUT TPM2X_BLOB *blob,
    Error* err);

TPM_RC TPM2X_Unseal(
    IN EFI_TCG2_PROTOCOL *protocol,
    UINT32 pcrMask,
    IN TPM_HANDLE parentKey,
    IN const TPM2X_BLOB *blob,
    OUT BYTE data[TPM2X_MAX_DATA_SIZE],
    OUT UINT16 *size,
    Error* err);

TPM_RC TPM2X_Cap(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_DH_PCR pcrHandle);

TPM_RC TPM2X_CapAll(
    EFI_TCG2_PROTOCOL* protocol);

/* No longer produces something TPM2X_AES_CBC_Decrypt() can decxrypt */
int TPM2X_AES_CBC_Encrypt(
    IN const unsigned char *in,
    IN unsigned long inSize,
    IN const unsigned char *key,
    IN unsigned int keyBits, /* 128 or 256 */
    OUT unsigned char **out,
    IN OUT unsigned long *outSize);

int TPM2X_AES_CBC_Decrypt(
    IN const unsigned char *in,
    IN unsigned long inSize,
    IN const unsigned char *key,
    IN unsigned int keyBits, /* 128 or 192 or 256 */
    IN const unsigned char *iv, /* 16 */
    IN unsigned int ivSize,
    OUT unsigned char **out,
    IN OUT unsigned long *outSize);

int TPM2X_HexStrToKey(
    const char* str,
    BYTE* key,
    UINT32* keyBytes);

int TPM2X_HexStrToBinary(
    const char* str,
    UINT32 len,
    BYTE* binary,
    UINT32* binarySize);

void TPM2X_BinaryToHexStr(
    const BYTE* binary,
    UINT32 binarySize,
    char* hexstr); /* size >= (binarySize * 2 + 1) */

TPM_RC TPM2X_ReadPCRSHA1(
    EFI_TCG2_PROTOCOL* protocol,
    UINT32 pcrNumber,
    SHA1Hash* sha1);

TPM_RC TPM2X_ReadPCRSHA256(
    EFI_TCG2_PROTOCOL* protocol,
    UINT32 pcrNumber,
    SHA256Hash* sha256);

TPM_RC TPM2X_ExtendPCR_SHA1(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_DH_PCR pcrHandle,
    IN const SHA1Hash* sha1In);

TPM_RC TPM2X_ExtendPCR_SHA256(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN TPMI_DH_PCR pcrHandle,
    IN const SHA256Hash* sha256In);

EFI_STATUS TPM2X_CheckTPMPresentFlag(
    EFI_TCG2_PROTOCOL *protocol);

#define TPM2X_CODETOSTR_BUFSIZE 128

const char* TPM2X_CodeToStr(
    char buf[TPM2X_CODETOSTR_BUFSIZE], 
    UINT32 rc);

int TPM2X_SerializeBlob(
    IN const TPM2X_BLOB* blob,
    IN OUT BYTE* blobBuffer,
    IN UINT32 blobBufferSize,
    OUT UINT32* newBlobBufferSize);

int TPM2X_DeserializeBlob(
    IN const BYTE* blobBuffer,
    IN const UINT32 blobBufferSize,
    OUT TPM2X_BLOB* blob);

int TPM2X_DumpBlob(
    IN const BYTE data[TPM2X_MAX_DATA_SIZE],
    IN UINT16 size);



#endif /* _tpm2_h */

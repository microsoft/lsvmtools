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
#ifndef _sha_h
#define _sha_h

#include "config.h"
#include "inline.h"
#include "strings.h"

#if !defined(HAVE_OPENSSL) && !defined(_WIN32)
# error "SHA-Hashes unsupported"
#endif

#if defined(HAVE_OPENSSL)
# include <openssl/sha.h> 
#endif

#if defined(_WIN32)
# include <bcrypt.h>
#endif

/*
**==============================================================================
**
** SHA Interface:
**
**==============================================================================
*/

#define SHA_MAGIC 0X8712A95F

#define __SHA1_SIZE 20
#define __SHA256_SIZE 32
#define __SHA384_SIZE 48
#define __SHA512_SIZE 64
#define __SHA_MAX_SIZE __SHA512_SIZE

typedef enum _SHAAlgorithm
{
    SHA1_ALG = 0,
    SHA256_ALG,
    SHA384_ALG,
    SHA512_ALG,

    /* Just indicates the number of SHA algorithms/ */
    NUMBER_OF_SHA_ALGS
}
SHAAlgorithm;

#define IS_SUPPORTED_SHA_ALG(x) ((x) >= SHA1_ALG && (x) < NUMBER_OF_SHA_ALGS)

typedef struct _SHA1Hash
{
    unsigned char buf[__SHA1_SIZE];
}
SHA1Hash;

typedef struct _SHA256Hash
{
    unsigned char buf[__SHA256_SIZE];
}
SHA256Hash;

typedef struct _SHA384Hash
{
    unsigned char buf[__SHA384_SIZE];
}
SHA384Hash;

typedef struct __SHA512Hash
{
    unsigned char buf[__SHA512_SIZE];
}
SHA512Hash;

typedef union ___SHAHash
{
    unsigned char buf[__SHA_MAX_SIZE];
}
__SHAHash;

typedef struct ___SHAContext
{
    SHAAlgorithm alg;
#if defined(HAVE_OPENSSL)
    union
    {
        SHA_CTX sha1ctx;
        SHA256_CTX sha256ctx;
        SHA512_CTX sha512ctx;
    }
    u;
#elif defined(_WIN32)
    BCRYPT_HASH_HANDLE handle;
#endif /* defined(_WIN32) */
}
__SHAContext;

typedef struct _SHA1Context
{
    __SHAContext shaContext;
}
SHA1Context;

typedef struct _SHA256Context
{
    __SHAContext shaContext;
}
SHA256Context;

typedef struct _SHA1Str
{
    char buf[sizeof(SHA1Hash) * 2 + 1];
}
SHA1Str;

typedef struct _SHA256Str
{
    char buf[sizeof(SHA256Hash) * 2 + 1];
}
SHA256Str;

BOOLEAN __SHAInit(
    __SHAContext* context,
    SHAAlgorithm alg);

BOOLEAN __SHAUpdate(
    __SHAContext* context,
    const void* data,
    UINTN size);

BOOLEAN __SHAFinal(
    __SHAContext* context,
    __SHAHash* hash);

INLINE INT32 SHAHashSize(
    SHAAlgorithm alg)
{
    switch (alg) {
        case SHA1_ALG:
            return __SHA1_SIZE;
        case SHA256_ALG:
            return __SHA256_SIZE;
        case SHA384_ALG:
            return __SHA384_SIZE;
        case SHA512_ALG:
            return __SHA512_SIZE;
        default:
            return -1;
    }
}

/*
**==============================================================================
**
** SHA1Init()
**
**==============================================================================
*/

INLINE BOOLEAN SHA1Init(
    SHA1Context* context)
{
    return __SHAInit(&context->shaContext, SHA1_ALG);
}

INLINE BOOLEAN SHA1Update(
    SHA1Context* context,
    const void* data,
    UINTN size)
{
    return __SHAUpdate(&context->shaContext, data, size);
}

INLINE BOOLEAN SHA1Final(
    SHA1Context* context,
    SHA1Hash* hash)
{
    return __SHAFinal(&context->shaContext, (__SHAHash*)hash);
}

BOOLEAN ComputeSHA1(
    const void* data,
    UINT32 size,
    SHA1Hash* sha1);

SHA1Str SHA1ToStr(const SHA1Hash* sha1);

BOOLEAN SHA1FromStr(
    const char* str, 
    UINTN len, 
    SHA1Hash* sha1);

INLINE BOOLEAN SHA1Equal(
    const SHA1Hash* lhs,
    const SHA1Hash* rhs)
{
    return Memcmp(lhs, rhs, sizeof(SHA1Hash)) == 0 ? TRUE : FALSE;
}

INLINE void SHA1Copy(
    SHA1Hash* dest,
    const SHA1Hash* src)
{
    Memcpy(dest, src, sizeof(SHA1Hash));
}

INLINE void SHA1Clear(
    SHA1Hash* sha1)
{
    Memset(sha1, 0, sizeof(SHA1Hash));
}

/*
**==============================================================================
**
** SHA256Init()
**
**==============================================================================
*/

INLINE BOOLEAN SHA256Init(
    SHA256Context* context)
{
    return __SHAInit(&context->shaContext, SHA256_ALG);
}

INLINE BOOLEAN SHA256Update(
    SHA256Context* context,
    const void* data,
    UINTN size)
{
    return __SHAUpdate(&context->shaContext, data, size);
}

INLINE BOOLEAN SHA256Final(
    SHA256Context* context,
    SHA256Hash* hash)
{
    return __SHAFinal(&context->shaContext, (__SHAHash*)hash);
}

BOOLEAN ComputeSHA256(
    const void* data,
    UINT32 size,
    SHA256Hash* sha256);

SHA256Str SHA256ToStr(const SHA256Hash* sha256);

BOOLEAN SHA256FromStr(
    const char* str, 
    UINTN len, 
    SHA256Hash* sha256);

INLINE BOOLEAN SHA256Equal(
    const SHA256Hash* lhs,
    const SHA256Hash* rhs)
{
    return Memcmp(lhs, rhs, sizeof(SHA256Hash)) == 0 ? TRUE : FALSE;
}

INLINE void SHA256Copy(
    SHA256Hash* dest,
    const SHA256Hash* src)
{
    Memcpy(dest, src, sizeof(SHA256Hash));
}

INLINE void SHA256Clear(
    SHA256Hash* sha256)
{
    Memset(sha256, 0, sizeof(SHA256Hash));
}

BOOLEAN ComputeSHA1Pad32(
    const void* data,
    UINT32 size,
    SHA1Hash* sha1);

BOOLEAN ComputeSHA256Pad32(
    const void* data,
    UINT32 size,
    SHA256Hash* sha256);

#endif /* _sha_h */

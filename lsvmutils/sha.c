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
#include "sha.h"
#include "strings.h"
#include "tpm2.h"

#if defined(HAVE_OPENSSL)
# include <openssl/sha.h> 
#endif

#if defined(_WIN32)
# include "bcryptdefines.h"
#endif

BOOLEAN __SHAInit(
    __SHAContext* context,
    SHAAlgorithm alg)
{
    /* Check arguments */
    if (!context || !IS_SUPPORTED_SHA_ALG(alg))
        return FALSE;

#if defined(HAVE_OPENSSL)
    {
        context->alg = alg;

        if (alg == SHA1_ALG)
            return SHA1_Init(&context->u.sha1ctx);
        else if (alg == SHA256_ALG)
            return SHA256_Init(&context->u.sha256ctx);
        else if (alg == SHA384_ALG)
            return SHA384_Init(&context->u.sha512ctx);
        else
            return SHA512_Init(&context->u.sha512ctx);
    }
#elif defined(_WIN32)
    {
        BCRYPT_ALG_HANDLE bah;

        context->alg = alg;

        if (alg == SHA1_ALG)
            bah = BCRYPT_SHA1_ALG_HANDLE;
        else if (alg == SHA256_ALG)
            bah = BCRYPT_SHA256_ALG_HANDLE;
        else if (alg == SHA384_ALG)
            bah = BCRYPT_SHA384_ALG_HANDLE;
        else
            bah = BCRYPT_SHA512_ALG_HANDLE;

        if (BCryptCreateHash(bah, &context->handle, NULL, 0, NULL, 0, 0) != 0)
        {
            return FALSE;
        }

        return TRUE;
    }
#endif /* defined(_WIN32) */
}

BOOLEAN __SHAUpdate(
    __SHAContext* context,
    const void* data,
    UINTN size)
{
    /* Check arguments */
    if (!context || !IS_SUPPORTED_SHA_ALG(context->alg))
        return FALSE;

#if defined(HAVE_OPENSSL)
    {
        if (context->alg == SHA1_ALG)
            return SHA1_Update(&context->u.sha1ctx, data, size);
        else if (context->alg == SHA256_ALG)
            return SHA256_Update(&context->u.sha256ctx, data, size);
        else if (context->alg == SHA384_ALG)
            return SHA384_Update(&context->u.sha512ctx, data, size);
        else
            return SHA512_Update(&context->u.sha512ctx, data, size);
    }
#elif defined(_WIN32)
    {
        if (BCryptHashData(context->handle, (void*)data, size, 0) != 0)
            return FALSE;

        return TRUE;
    }
#endif /* defined(_WIN32) */
}

BOOLEAN __SHAFinal(
    __SHAContext* context,
    __SHAHash* hash)
{
    /* Check arguments */
    if (!context || !hash || !IS_SUPPORTED_SHA_ALG(context->alg))
        return FALSE;;

#if defined(HAVE_OPENSSL)

    if (context->alg == SHA1_ALG)
        return SHA1_Final(hash->buf, &context->u.sha1ctx);
    else if (context->alg == SHA256_ALG)
        return SHA256_Final(hash->buf, &context->u.sha256ctx);
    else if (context->alg == SHA384_ALG)
        return SHA384_Final(hash->buf, &context->u.sha512ctx);
    else
        return SHA512_Final(hash->buf, &context->u.sha512ctx);

#elif defined(_WIN32)
    {
        UINTN size;

        if (context->alg == SHA1_ALG)
            size = sizeof(SHA1Hash);
        else if (context->alg == SHA256_ALG)
            size = sizeof(SHA256Hash);
        else if (context->alg == SHA384_ALG)
            size = sizeof(SHA384Hash);
        else
            size = sizeof(SHA512Hash);

        if (BCryptFinishHash(context->handle, hash->buf, size, 0) != 0)
        {
            return FALSE;
        }

        return TRUE;
    }
#endif /* defined(_WIN32) */
}

BOOLEAN ComputeSHA1(
    const void* data,
    UINT32 size,
    SHA1Hash* sha1)
{
    SHA1Context context;

    if (!SHA1Init(&context))
        return FALSE;

    if (!SHA1Update(&context, data, size))
        return FALSE;

    if (!SHA1Final(&context, sha1))
        return FALSE;

    return TRUE;
}

BOOLEAN ComputeSHA256(
    const void* data,
    UINT32 size,
    SHA256Hash* sha256)
{
    SHA256Context context;

    if (!SHA256Init(&context))
        return FALSE;

    if (!SHA256Update(&context, data, size))
        return FALSE;

    if (!SHA256Final(&context, sha256))
        return FALSE;

    return TRUE;
}

SHA1Str SHA1ToStr(const SHA1Hash* sha1)
{
    SHA1Str str;
    TPM2X_BinaryToHexStr(sha1->buf, sizeof(SHA1Hash), str.buf);
    return str;
}

SHA256Str SHA256ToStr(const SHA256Hash* sha256)
{
    SHA256Str str;
    TPM2X_BinaryToHexStr(sha256->buf, sizeof(SHA256Hash), str.buf);
    return str;
}

BOOLEAN SHA1FromStr(const char* str, UINTN len, SHA1Hash* sha1)
{
    UINT32 tmp;

    if (len != sizeof(SHA1Hash) * 2)
        return FALSE;

    if (TPM2X_HexStrToBinary(
        str, 
        len, 
        sha1->buf, 
        &tmp) != 0)
    {
        return FALSE;
    }

    return TRUE;
}

BOOLEAN SHA256FromStr(const char* str, UINTN len, SHA256Hash* sha256)
{
    UINT32 tmp;

    if (len != sizeof(SHA256Hash) * 2)
        return FALSE;

    if (TPM2X_HexStrToBinary(
        str, 
        len, 
        sha256->buf, 
        &tmp) != 0)
    {
        return FALSE;
    }

    return TRUE;
}

BOOLEAN ComputeSHA1Pad32(
    const void* data,
    UINT32 size,
    SHA1Hash* sha1)
{
    SHA1Context context;
    UINT32 n = 4; /* 4-byte alignment */
    UINT32 aligned_size = (size + n - 1) / n * n;
    UINT32 delta = aligned_size - size;

    if (!SHA1Init(&context))
        return FALSE;

    if (!SHA1Update(&context, data, size))
        return FALSE;

    if (delta)
    {
        UINT8 bytes[sizeof(UINT32)];
        Memset(bytes, 0, sizeof(bytes));

        if (!SHA1Update(&context, bytes, delta))
            return FALSE;
    }

    if (!SHA1Final(&context, sha1))
        return FALSE;

    return TRUE;
}

BOOLEAN ComputeSHA256Pad32(
    const void* data,
    UINT32 size,
    SHA256Hash* sha1)
{
    SHA256Context context;
    UINT32 n = 4; /* 4-byte alignment */
    UINT32 aligned_size = (size + n - 1) / n * n;
    UINT32 delta = aligned_size - size;

    if (!SHA256Init(&context))
        return FALSE;

    if (!SHA256Update(&context, data, size))
        return FALSE;

    if (delta)
    {
        UINT8 bytes[sizeof(UINT32)];
        Memset(bytes, 0, sizeof(bytes));

        if (!SHA256Update(&context, bytes, delta))
            return FALSE;
    }

    if (!SHA256Final(&context, sha1))
        return FALSE;

    return TRUE;
}

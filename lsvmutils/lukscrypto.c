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
#if defined (_WIN32)
# define WIN32_NO_STATUS
# include <Windows.h>
# undef WIN32_NO_STATUS
# include <bcrypt.h>
# include <ntstatus.h>
# include <intrin.h>
# include "bcryptdefines.h"
#endif

#include "../crypto/utils_crypto.h"
#include "strings.h"
#include "luks.h"
#include "alloc.h"
#include "print.h"

void LUKSInitialize()
{
}

void LUKSShutdown()
{
}

struct _LUKSCipher
{
    crypto_cipher *cipher;
    UINT8* key;
    UINTN keySize;
};

LUKSCipher* LUKSGetAES256ECBCipher(
    const UINT8* key,
    UINTN keySize)
{
    LUKSCipher tmp;
    LUKSCipher* result = NULL;

    Memset(&tmp, 0xDD, sizeof(tmp));

    /* Create crypto_cipher object */
    if (CryptoCipherInit(
        CRYPTO_AES_CBC,
        (UINT8*)key,
        keySize,
        &tmp.cipher) != CRYPTO_ERR_NONE)
    {
        goto done;
    }

    /* Copy the key */
    {
        if (!(tmp.key = (UINT8*)Malloc(keySize)))
            goto done;

        Memcpy(tmp.key, key, keySize);
        tmp.keySize = keySize;
    }

    /* Create and initialize object */
    {
        if (!(result = (LUKSCipher*)Calloc(1, sizeof(LUKSCipher))))
            goto done;

        Memcpy(result, &tmp, sizeof(LUKSCipher));
    }

done:

    if (!result)
    {
        if (tmp.key)
            Free(tmp.key);
    }

    return result;
}

LUKSCipher* LUKSGetCipher(
    const LUKSHeader* header,
    const UINT8* key,
    UINTN keySize)
{
    LUKSCipher tmp;
    LUKSCipher* result = NULL;
    crypto_algo algo;
    UINT32 keyBits;

    Memset(&tmp, 0xDD, sizeof(tmp));

    /* Check for null parameters */
    if (!header)
        goto done;

    /* Create crypto_cipher object */
    {
        /* Only supporting AES cipher for now */
        if (Strcmp(header->cipher_name, "aes") != 0)
            goto done;

        /* Set the key length in bits */
        keyBits = header->key_bytes * 8;

        /* Determine the cipher mode */
        if (Strcmp(header->cipher_mode, "cbc-plain") == 0)
            algo = CRYPTO_AES_CBC;
        else if (Strcmp(header->cipher_mode, "cbc-essiv:") == 0)
            algo = CRYPTO_AES_CBC;
        else if (Strcmp(header->cipher_mode, "xts-plain64") == 0)
        {
            algo = CRYPTO_AES_XTS;
            keyBits /= 2;
        }
        else 
            goto done;

        if (CryptoCipherInit(
            algo,
            (UINT8*)key,
            keySize,
            &tmp.cipher) != CRYPTO_ERR_NONE)
        {
            goto done;
        }
    }

    /* Copy the key */
    {
        if (!(tmp.key = (UINT8*)Malloc(keySize)))
            goto done;

        Memcpy(tmp.key, key, keySize);
        tmp.keySize = keySize;
    }

    /* Create and initialize object */
    {
        if (!(result = (LUKSCipher*)Calloc(1, sizeof(LUKSCipher))))
            goto done;

        Memcpy(result, &tmp, sizeof(LUKSCipher));
    }

done:

    if (!result)
    {
        if (tmp.key)
            Free(tmp.key);
    }

    return result;
}

void LUKSReleaseCipher(
    LUKSCipher* cipher)
{
    if (!cipher)
        return;

    if (cipher->key)
        Free(cipher->key);

    Free(cipher);
}

int LUKSCryptData(
    LUKSCipher* cipher,
    LUKSCryptMode mode,
    UINT8 *iv,
    UINTN ivSize,
    const UINT8 *in,
    UINTN inSize,
    UINT8 *out_)
{
    int rc = -1;

    /* Check parameters */
    if (!cipher || !iv || !in || !out_)
    {
        goto done;
    }

    /* Encrypt/Decrypt data */
    if (mode)
    {
        if (CryptoCipherEncryptNoPad(
            cipher->cipher,
            (UINT8*)in,
            inSize,
            iv,
            ivSize,
            out_,
            inSize) != CRYPTO_ERR_NONE)
        {
            goto done;
        }
    }
    else
    {
        if (CryptoCipherDecryptNoPad(
            cipher->cipher,
            (UINT8*)in,
            inSize,
            iv,
            ivSize,
            out_,
            inSize) != CRYPTO_ERR_NONE)
        {
            goto done;
        }
    }

    rc = 0;
    
done:

    return rc;
}

int LUKSDeriveKey(
    const char *pass, 
    int passlen,
    const unsigned char* salt,
    int saltlen,
    int iter,
    const char* hashspec,
    int keylen,
    unsigned char* out)
{
    int rc = -1;
    crypto_algo algo;

    if (Strcmp(hashspec, "sha1") == 0)
        algo = CRYPTO_HMAC_SHA1;
    else if (Strcmp(hashspec, "sha256") == 0)
        algo = CRYPTO_HMAC_SHA256;
    else
        goto done;

    if (CryptoPBKDF2(
        algo,
        (UINT8*)pass,
        passlen,
        (UINT8*)salt,
        saltlen,
        iter,
        out,
        keylen) != CRYPTO_ERR_NONE)
    {
        goto done;
    }

    rc = 0;

done:
    return rc;
}

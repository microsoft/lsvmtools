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
#include "luks.h"
#include <lsvmutils/strings.h>
#include <lsvmutils/byteorder.h>
#include <lsvmutils/alloc.h>
#include <lsvmutils/print.h>
#include <lsvmutils/sha.h>

#if defined(HAVE_OPENSSL)
# include <openssl/evp.h>
# include <openssl/sha.h>
# include <openssl/err.h>
# include <openssl/conf.h>
#endif /* defined(HAVE_OPENSSL) */

#if defined(BUILD_EFI)
# define VSNPRINTF VSPrint
# define SNPRINTF SPrint
#else
# define VSNPRINTF vsnprintf
# define SNPRINTF snprintf
#endif

struct _LUKSCipher
{
    const EVP_CIPHER* evp;
    UINT8* key;
    UINTN keySize;
};

static int _initialized = 0;

void LUKSInitialize()
{
    if (!_initialized)
    {
        ERR_load_crypto_strings();
        OpenSSL_add_all_algorithms();
        _initialized = 1;
    }
}

void LUKSShutdown()
{
    if (_initialized)
    {
        EVP_cleanup();
        CRYPTO_cleanup_all_ex_data();
        ERR_free_strings();
        _initialized = 0;
    }
}

LUKSCipher* LUKSGetAES256ECBCipher(
    const UINT8* key,
    UINTN keySize)
{
    LUKSCipher tmp;
    LUKSCipher* cipher = NULL;

    /* Obtain cipher from OpenSSL */
    if (!(tmp.evp = EVP_aes_256_ecb()))
        goto done;

    /* Copy the key */
    {
        if (!(tmp.key = (UINT8*)Malloc(keySize)))
            goto done;

        Memcpy(tmp.key, key, keySize);
        tmp.keySize = keySize;
    }

    /* Create and initialize object */
    {
        if (!(cipher = (LUKSCipher*)Calloc(1, sizeof(LUKSCipher))))
            goto done;

        Memcpy(cipher, &tmp, sizeof(LUKSCipher));
    }

done:

    if (!cipher)
    {
        if (tmp.key)
            Free(tmp.key);
    }

    return cipher;
}

LUKSCipher* LUKSGetCipher(
    const LUKSHeader* header,
    const UINT8* key,
    UINTN keySize)
{
    LUKSCipher tmp;
    LUKSCipher* cipher = NULL;
    const char cipherName[] = "AES";
    const char* cipherMode = NULL;
    UINT32 keyBits;

    /* Check for null parameters */
    if (!header)
        goto done;

    /* Obtain cipher from OpenSSL */
    {
        /* Only supporting AES cipher for now */
        if (Strcmp(header->cipher_name, "aes") != 0)
            goto done;

        /* Set the key length in bits */
        keyBits = header->key_bytes * 8;

        /* Determine the cipher mode */
        if (Strcmp(header->cipher_mode, "ecb") == 0) 
            cipherMode = "ECB";
        else if (Strcmp(header->cipher_mode, "cbc-plain") == 0)
            cipherMode = "CBC";
        else if (Strcmp(header->cipher_mode, "cbc-essiv:") == 0)
            cipherMode = "CBC";
        else if (Strcmp(header->cipher_mode, "xts-plain64") == 0)
        {
            cipherMode = "XTS";
            keyBits /= 2;
        }
        else 
            goto done;

        /* Form the string to pass to EVP_get_cipherbyname() */
        {
            TCHAR tcs[128];
            char str[128];
#if defined(BUILD_EFI)
            TCHAR FMT[] = TCS("%a-%d-%a");
#else
            const char FMT[] = "%s-%d-%s";
#endif
            SNPRINTF(tcs, ARRSIZE(tcs), FMT, cipherName, keyBits, cipherMode);
            StrTcscpy(str, tcs);
            tmp.evp = EVP_get_cipherbyname(str);

            if (!tmp.evp)
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
        if (!(cipher = (LUKSCipher*)Calloc(1, sizeof(LUKSCipher))))
            goto done;

        Memcpy(cipher, &tmp, sizeof(LUKSCipher));
    }

done:

    if (!cipher)
    {
        if (tmp.key)
            Free(tmp.key);
    }

    return cipher;
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
    UINT8 *out)
{
    int rc = -1;
    int len;
    EVP_CIPHER_CTX *ctx = NULL;

    if (!(ctx = EVP_CIPHER_CTX_new()))
        goto done;

    EVP_CIPHER_CTX_set_padding(ctx, 0);

    if (!EVP_CipherInit_ex(ctx, cipher->evp, NULL, cipher->key, iv, mode))
        goto done;

    if (!EVP_CipherUpdate(ctx, out, &len, in, inSize)) 
        goto done;

    if (!EVP_CipherFinal_ex(ctx, (UINT8*)in + len, &len))
        goto done;

    rc = len;

done:

    if (ctx)
        EVP_CIPHER_CTX_free(ctx);

    return rc;
}

int LUKSDeriveKey(
    const char *pass, 
    int passlen,
    const unsigned  char* salt,
    int saltlen,
    int iter,
    const char* hashspec,
    int keylen,
    unsigned char* out)
{
    int rc = -1;

    if (!PKCS5_PBKDF2_HMAC(
        pass,
        passlen,
        salt,
        saltlen,
        iter,
        EVP_get_digestbyname(hashspec),
        keylen,
        out))
    {
        goto done;
    }

    rc = 0;

done:
    return rc;
}

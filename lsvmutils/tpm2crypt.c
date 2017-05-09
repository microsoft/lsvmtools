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

#if defined(HAVE_OPENSSL)
# include <openssl/aes.h> 
# include <openssl/sha.h> 
# include <openssl/rand.h> 
# include <openssl/evp.h>
# include <openssl/hmac.h>
#endif /* defined(HAVE_OPENSSL) */

#if defined(HAVE_OPENSSL)
static const EVP_CIPHER *_GetCipher(unsigned int keyBits)
{
    const EVP_CIPHER *ret;
    switch (keyBits)
    {
        case 128:
            ret = EVP_aes_128_cbc();
            break;
        case 192:
            ret = EVP_aes_192_cbc();
            break;
        case 256:
            ret = EVP_aes_256_cbc();
            break;
        default:
            ret = NULL;
    }
    return ret;
}
#endif /* defined(HAVE_OPENSSL) */

static int _Encrypt(
    const unsigned char *inData, 
    int inSize,
    const unsigned char *key, 
    const unsigned char *iv,
    unsigned char *outData,
    unsigned long *outSize)
{
    int result = -1;
    EVP_CIPHER_CTX *ctx;
    int len;

    if (outSize)
        *outSize = 0;

    if(!(ctx = EVP_CIPHER_CTX_new()))
        goto done;

    if (!EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
        goto done;

    if(!EVP_EncryptUpdate(ctx, outData, &len, inData, inSize))
        goto done;

    *outSize = len;

    if(!EVP_EncryptFinal_ex(ctx, outData + len, &len))
        goto done;

    (*outSize) += len;

    result = 0;

done:

    if (ctx)
        EVP_CIPHER_CTX_free(ctx);

    return result;
}

int TPM2X_AES_CBC_Encrypt(
    IN const unsigned char *in,
    IN unsigned long inSize,
    IN const unsigned char *key,
    IN unsigned int keyBits,
    OUT unsigned char **out,
    IN OUT unsigned long *outSize)
{
    unsigned char iv[AES_BLOCK_SIZE];
    unsigned char* data = NULL;
    unsigned char* p = NULL;

    if (out)
        *out = NULL;

    if (outSize)
        *outSize = 0;

    /* Check parameters */
    if (!in || !inSize || !key || !keyBits || !out || !outSize)
        return -1;

    /* Randomize the initialization vector */
    Memset(iv, 0, sizeof(iv));
    RAND_pseudo_bytes(iv, sizeof(iv));

    /* Allocate output buffer [IV + SIZE + DATA + PADDING] */
    {
        size_t n = AES_BLOCK_SIZE + sizeof(ULONG) + inSize + AES_BLOCK_SIZE;

        if (!(data = Malloc(n)))
            return -1;

        p = data;
    }

    /* Write [IV + SIZE] to output buffer */
    {
        ULONG tmp = inSize;

        /* [IV] */
        Memcpy(p, iv, AES_BLOCK_SIZE);
        p += AES_BLOCK_SIZE;

        /* [SIZE] */
        Memcpy(p, &tmp, sizeof(ULONG));
        p += sizeof(ULONG);
    }

    /* Write [DATA] */
    {
        unsigned long n = 0;

        if (_Encrypt(in, inSize, key, iv, p, &n) != 0)
        {
            return -1;
        }

        *out = data;
        *outSize = AES_BLOCK_SIZE + sizeof(ULONG) + n;
    } 

    return 0;
}

int TPM2X_AES_CBC_Decrypt(
    IN const unsigned char *in,
    IN unsigned long inSize,
    IN const unsigned char *key,
    IN unsigned int keyBits,
    IN const unsigned char *iv,
    IN unsigned int ivSize,
    OUT unsigned char **out,
    IN OUT unsigned long *outSize)
{
    EVP_CIPHER_CTX *ctx = NULL;
    const EVP_CIPHER *type = NULL;
    unsigned char *buf = NULL;
    int bufSize1 = 0;
    int bufSize2 = 0;
    int err = -1;

    /* Check params. */
    if (!in || !key || !iv || !out || !outSize)
        return -1;

#if 0
    if (inSize % AES_BLOCK_SIZE != 0 || ivSize != AES_BLOCK_SIZE)
        return -2;
#endif
    
    if ((type = _GetCipher(keyBits)) == NULL)
        return -3;

    /* Start the cipher. */
    ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL)
    {
        err = -4;
        goto Cleanup;
    }

    if (!EVP_DecryptInit_ex(ctx, type, NULL, key, iv))
    {
        err = -5;
        goto Cleanup;
    }

    buf = (unsigned char*) Malloc(inSize);
    if (buf == NULL)
    {
       err = -6;
       goto Cleanup;
    }

    bufSize1 = inSize;

    if (!EVP_DecryptUpdate(ctx, buf, &bufSize1, in, inSize))
    {
       err = -7;
       goto Cleanup;
    }

    if (!EVP_DecryptFinal_ex(ctx, buf + bufSize1, &bufSize2))
    {
        err = -8;
        goto Cleanup;
    }

    *out = buf;
    *outSize = bufSize1 + bufSize2;
    err = 0;
    buf = NULL;

Cleanup:
    if (ctx != NULL)
        EVP_CIPHER_CTX_free(ctx);
    if (buf != NULL)
        Free(buf);

    return err;
}

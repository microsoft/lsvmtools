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
#include "pass.h"
#include "alloc.h"
#include <unistd.h>
#include <openssl/rand.h> 
#include <stdio.h>
#include "strings.h"
#include "sha.h"

#define LSVMPASS_SHA512_PRE "$6$"
#define LSVMPASS_SHA512_PASS_SIZE 86
#define LSVMPASS_ROUNDS_DEFAULT 5000

static const char BASE64_TABLE[] = 
    "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

int GenerateSalt(char salt[SALT_STRING_SIZE])
{
    int rc = -1;
    UINT8 data[SALT_SIZE];
    char* p = salt;
    UINTN i;

    *p++ = '$';
    *p++ = '6';
    *p++ = '$';

    Memset(data, 0, sizeof(data));

    if (!RAND_bytes(data, sizeof(data)))
        goto done;
    
    for (i = 0; i < sizeof(data); i++)
    {
        *p++ = BASE64_TABLE[data[i] % (sizeof(BASE64_TABLE) - 1)]; 
    }

    *p++ = '\0';

    rc = 0;

done:
    return rc;
}

void print_array(
    const char* array_name,
    const unsigned char* buf,
    size_t buflen)
{
    size_t i;
    printf("\n%s\n", array_name);
    for (i = 0; i < buflen; i++)
    {
        printf("%02X ", buf[i]);
    }
    printf("\n");
}

char *SHA512Crypt(
    const char* password,
    const char* salt)
{
    size_t passLen = 0;
    size_t saltLen = 0; 
    __SHAContext a;
    __SHAHash aHash = { { 0 } };
    __SHAContext b;
    __SHAHash bHash = { { 0 } };
    size_t i = 0;
    __SHAContext dp;
    __SHAHash dpHash = { { 0 } };
    __SHAContext ds;
    __SHAHash dsHash = { { 0 } };
    unsigned char* p = NULL;
    unsigned char* s = NULL;
    char* result = NULL;
    char* resultPtr = NULL; 
    size_t resultSize = 0;

    if (!password || !salt)
        goto CleanupErr;

    /* Check prefix */
    if (Strncmp(salt, LSVMPASS_SHA512_PRE, sizeof(LSVMPASS_SHA512_PRE) - 1) != 0)
        goto CleanupErr;

    /* Skip prefix now. since it's not used. */
    salt += sizeof(LSVMPASS_SHA512_PRE) - 1;
    
    /* Read up to 16 characters of the salt. */
    while (salt[saltLen] != 0 && saltLen < SALT_SIZE)
        saltLen++;

    passLen = Strlen(password);

   /* 
    * Now, do the algorithm specified here:
    * https://www.akkadia.org/drepper/SHA-crypt.txt
    */

    /* Start digest A (refer to the link above for details. */
    if (!__SHAInit(&a, SHA512_ALG))
        goto CleanupErr;
       
    if (!__SHAUpdate(&a, password, passLen))
        goto CleanupErr;

    if (!__SHAUpdate(&a, salt, saltLen))
        goto CleanupErr;

    /* Complete digest B. */
    if (!__SHAInit(&b, SHA512_ALG))
        goto CleanupErr;

    if (!__SHAUpdate(&b, password, passLen))
        goto CleanupErr;

    if (!__SHAUpdate(&b, salt, saltLen))
        goto CleanupErr;
    
    if (!__SHAUpdate(&b, password, passLen))
        goto CleanupErr;

     if (!__SHAFinal(&b, &bHash))
        goto CleanupErr;

    /* Finish digest A. */
    for (i = 0; i + __SHA512_SIZE <= passLen; i += __SHA512_SIZE)
    {
        if (!__SHAUpdate(&a, bHash.buf, __SHA512_SIZE))
            goto CleanupErr;
    }

    if (!__SHAUpdate(&a, &bHash, passLen - i))
        goto CleanupErr;

    for (i = passLen; i > 0; i >>= 1)
    {
        if (i & 1)
        {
            if (!__SHAUpdate(&a, bHash.buf, __SHA512_SIZE))
                goto CleanupErr;
        }
        else
        {
            if (!__SHAUpdate(&a, password, passLen))
                goto CleanupErr;
        }
    }

    if (!__SHAFinal(&a, &aHash))
        goto CleanupErr;
    
    /* Create digest DP and sequence P. */    
    if (!__SHAInit(&dp, SHA512_ALG))
        goto CleanupErr;

    for (i = 0; i < passLen; i++)
    {
        if (!__SHAUpdate(&dp, password, passLen))
            goto CleanupErr;
    }

    if (!__SHAFinal(&dp, &dpHash))
        goto CleanupErr;
    
    p = (unsigned char*) Malloc(passLen);
    if (p == NULL)
        goto CleanupErr;

    for (i = 0; i + __SHA512_SIZE <= passLen; i += __SHA512_SIZE)
        Memcpy(p + i, dpHash.buf, __SHA512_SIZE);
    Memcpy(p + i, dpHash.buf, passLen - i);
    
    /* Create digest DS and sequence S. */
    if (!__SHAInit(&ds, SHA512_ALG))
        goto CleanupErr;

    for (i = 0; i < 16 + aHash.buf[0]; i++)
    {
        if (!__SHAUpdate(&ds, salt, saltLen))
            goto CleanupErr;
    }

    if (!__SHAFinal(&ds, &dsHash))
        goto CleanupErr;

    s = (unsigned char*) Malloc(saltLen);
    if (s == NULL)
        goto CleanupErr;

    for (i = 0; i  + __SHA512_SIZE <= saltLen; i  += __SHA512_SIZE) 
        Memcpy(s + i, dsHash.buf, __SHA512_SIZE);
    Memcpy(s + i, dsHash.buf, saltLen - i);

    /* Loop through the rounds to burn CPU time. */
    for (i = 0; i < LSVMPASS_ROUNDS_DEFAULT; i++)
    {
        __SHAContext ctx;
        
        if (!__SHAInit(&ctx, SHA512_ALG))
            goto CleanupErr;        


        if (i & 1)
        {
            if (!__SHAUpdate(&ctx, p, passLen))
                goto CleanupErr;
        }
        else
        {
            if (!__SHAUpdate(&ctx, aHash.buf, __SHA512_SIZE))
                goto CleanupErr;
        }

        if (i % 3 != 0)
        {
            if (!__SHAUpdate(&ctx, s, saltLen))
                goto CleanupErr;
        }

        if (i % 7 != 0)
        {
            if (!__SHAUpdate(&ctx, p, passLen))
                goto CleanupErr;
        }
        
        if (i & 1)
        {
            if (!__SHAUpdate(&ctx, aHash.buf, __SHA512_SIZE))
                goto CleanupErr;
        }
        else
        {
            if (!__SHAUpdate(&ctx, p, passLen))
                goto CleanupErr;
        }

        if (!__SHAFinal(&ctx, &aHash))
        {
            goto CleanupErr;
        }
    }

    /*
     * Format is $6$<salt>$<password><NULL>. Password is always 86 characters for SHA512.
     * Note that sizeof(LSVMPASS_SHA512_PRE) will account for the null terminator in
     * the size calculation.
     */ 
    resultSize = sizeof(LSVMPASS_SHA512_PRE) + saltLen + 1 + LSVMPASS_SHA512_PASS_SIZE;
    result = (char*) Malloc(resultSize);
    if (result == NULL)
        goto CleanupErr;

    resultPtr = result;
    Memcpy(resultPtr, LSVMPASS_SHA512_PRE, sizeof(LSVMPASS_SHA512_PRE) - 1);
    resultPtr += sizeof(LSVMPASS_SHA512_PRE) - 1;
    Memcpy(resultPtr, salt, saltLen);
    resultPtr += saltLen;
    *resultPtr++ = '$';

    /*
     * First, handle every row in the table aside from the last.
     * The first row starts with b3 = aHash.buf[0], b2 = aHash.buf[21],
     * and b1 = aHash.buf[42]. Then, each progressive row changes the
     * indices by (22 * i) % 63.
     */ 
    for (i = 0; i <= 20; i++)
    {
        unsigned char b3 = aHash.buf[( 0 + 22 * i) % 63];
        unsigned char b2 = aHash.buf[(21 + 22 * i) % 63];
        unsigned char b1 = aHash.buf[(42 + 22 * i) % 63];     
        UINT32 all = (b3 << 16) | (b2 << 8) | b1;
        UINT32 n = 4;        
 
        while (n > 0)
        {
            *resultPtr++ = BASE64_TABLE[(all & 0x3f)];
            all >>= 6;
            n--;
        } 
    }

    /* Handle the the last row. */
    *resultPtr++ = BASE64_TABLE[aHash.buf[63] & 0x3f];
    *resultPtr++ = BASE64_TABLE[aHash.buf[63] >> 6];
    *resultPtr = 0;
    goto Cleanup;

CleanupErr:
    if (result != NULL)
    {
        Memset(result, 0, resultSize);
        Free(s);
    }
Cleanup:
    /* Zero out the hashes. */
    Memset(&aHash, 0, sizeof(aHash));
    Memset(&bHash, 0, sizeof(bHash));
    Memset(&dpHash, 0, sizeof(dpHash));
    Memset(&dsHash, 0, sizeof(dsHash));
    if (p != NULL)
    {
        Memset(p, 0, passLen);
        Free(p);
    }
    if (s != NULL)
    {
        Memset(s, 0, saltLen);
        Free(s);
    } 
    return result;
}

int CryptPassword(
    char* buf,
    UINTN bufSize,
    const char* password)
{
    int rc = -1;
    char salt[SALT_STRING_SIZE];
    char* result;

    if (buf)
        *buf = '\0';

    if (!buf || !password)
        goto done;

    if (GenerateSalt(salt) != 0)
        goto done;

    if (!(result = SHA512Crypt(password, salt)))
        goto done;

    if (Strlcpy(buf, result, bufSize) >= bufSize)
        goto done;

    Free(result);
    rc = 0;

done:
    return rc;
}

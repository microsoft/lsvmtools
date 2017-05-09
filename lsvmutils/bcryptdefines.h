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
#ifndef _bcryptdefines_h
#define _bcryptdefines_h

#if !defined(BCRYPT_ALG_HANDLE)
#define BCRYPT_MD2_ALG_HANDLE                   ((BCRYPT_ALG_HANDLE) 0x00000001)
#define BCRYPT_MD4_ALG_HANDLE                   ((BCRYPT_ALG_HANDLE) 0x00000011)
#define BCRYPT_MD5_ALG_HANDLE                   ((BCRYPT_ALG_HANDLE) 0x00000021)
#define BCRYPT_SHA1_ALG_HANDLE                  ((BCRYPT_ALG_HANDLE) 0x00000031)
#define BCRYPT_SHA256_ALG_HANDLE                ((BCRYPT_ALG_HANDLE) 0x00000041)
#define BCRYPT_SHA384_ALG_HANDLE                ((BCRYPT_ALG_HANDLE) 0x00000051)
#define BCRYPT_SHA512_ALG_HANDLE                ((BCRYPT_ALG_HANDLE) 0x00000061)
#define BCRYPT_RC4_ALG_HANDLE                   ((BCRYPT_ALG_HANDLE) 0x00000071)
#define BCRYPT_RNG_ALG_HANDLE                   ((BCRYPT_ALG_HANDLE) 0x00000081)
#define BCRYPT_HMAC_MD5_ALG_HANDLE              ((BCRYPT_ALG_HANDLE) 0x00000091)
#define BCRYPT_HMAC_SHA1_ALG_HANDLE             ((BCRYPT_ALG_HANDLE) 0x000000a1)
#define BCRYPT_HMAC_SHA256_ALG_HANDLE           ((BCRYPT_ALG_HANDLE) 0x000000b1)
#define BCRYPT_HMAC_SHA384_ALG_HANDLE           ((BCRYPT_ALG_HANDLE) 0x000000c1)
#define BCRYPT_HMAC_SHA512_ALG_HANDLE           ((BCRYPT_ALG_HANDLE) 0x000000d1)
#define BCRYPT_RSA_ALG_HANDLE                   ((BCRYPT_ALG_HANDLE) 0x000000e1)
#define BCRYPT_ECDSA_ALG_HANDLE                 ((BCRYPT_ALG_HANDLE) 0x000000f1)
#define BCRYPT_AES_CMAC_ALG_HANDLE              ((BCRYPT_ALG_HANDLE) 0x00000101)
#define BCRYPT_AES_GMAC_ALG_HANDLE              ((BCRYPT_ALG_HANDLE) 0x00000111)
#define BCRYPT_HMAC_MD2_ALG_HANDLE              ((BCRYPT_ALG_HANDLE) 0x00000121)
#define BCRYPT_HMAC_MD4_ALG_HANDLE              ((BCRYPT_ALG_HANDLE) 0x00000131)
#define BCRYPT_3DES_CBC_ALG_HANDLE              ((BCRYPT_ALG_HANDLE) 0x00000141)
#define BCRYPT_3DES_ECB_ALG_HANDLE              ((BCRYPT_ALG_HANDLE) 0x00000151)
#define BCRYPT_3DES_CFB_ALG_HANDLE              ((BCRYPT_ALG_HANDLE) 0x00000161)
#define BCRYPT_3DES_112_CBC_ALG_HANDLE          ((BCRYPT_ALG_HANDLE) 0x00000171)
#define BCRYPT_3DES_112_ECB_ALG_HANDLE          ((BCRYPT_ALG_HANDLE) 0x00000181)
#define BCRYPT_3DES_112_CFB_ALG_HANDLE          ((BCRYPT_ALG_HANDLE) 0x00000191)
#define BCRYPT_AES_CBC_ALG_HANDLE               ((BCRYPT_ALG_HANDLE) 0x000001a1)
#define BCRYPT_AES_ECB_ALG_HANDLE               ((BCRYPT_ALG_HANDLE) 0x000001b1)
#define BCRYPT_AES_CFB_ALG_HANDLE               ((BCRYPT_ALG_HANDLE) 0x000001c1)
#define BCRYPT_AES_CCM_ALG_HANDLE               ((BCRYPT_ALG_HANDLE) 0x000001d1)
#define BCRYPT_AES_GCM_ALG_HANDLE               ((BCRYPT_ALG_HANDLE) 0x000001e1)
#define BCRYPT_DES_CBC_ALG_HANDLE               ((BCRYPT_ALG_HANDLE) 0x000001f1)
#define BCRYPT_DES_ECB_ALG_HANDLE               ((BCRYPT_ALG_HANDLE) 0x00000201)
#define BCRYPT_DES_CFB_ALG_HANDLE               ((BCRYPT_ALG_HANDLE) 0x00000211)
#define BCRYPT_DESX_CBC_ALG_HANDLE              ((BCRYPT_ALG_HANDLE) 0x00000221)
#define BCRYPT_DESX_ECB_ALG_HANDLE              ((BCRYPT_ALG_HANDLE) 0x00000231)
#define BCRYPT_DESX_CFB_ALG_HANDLE              ((BCRYPT_ALG_HANDLE) 0x00000241)
#define BCRYPT_RC2_CBC_ALG_HANDLE               ((BCRYPT_ALG_HANDLE) 0x00000251)
#define BCRYPT_RC2_ECB_ALG_HANDLE               ((BCRYPT_ALG_HANDLE) 0x00000261)
#define BCRYPT_RC2_CFB_ALG_HANDLE               ((BCRYPT_ALG_HANDLE) 0x00000271)
#define BCRYPT_DH_ALG_HANDLE                    ((BCRYPT_ALG_HANDLE) 0x00000281)
#define BCRYPT_ECDH_ALG_HANDLE                  ((BCRYPT_ALG_HANDLE) 0x00000291)
#define BCRYPT_ECDH_P256_ALG_HANDLE             ((BCRYPT_ALG_HANDLE) 0x000002a1)
#define BCRYPT_ECDH_P384_ALG_HANDLE             ((BCRYPT_ALG_HANDLE) 0x000002b1)
#define BCRYPT_ECDH_P521_ALG_HANDLE             ((BCRYPT_ALG_HANDLE) 0x000002c1)
#define BCRYPT_DSA_ALG_HANDLE                   ((BCRYPT_ALG_HANDLE) 0x000002d1)
#define BCRYPT_ECDSA_P256_ALG_HANDLE            ((BCRYPT_ALG_HANDLE) 0x000002e1)
#define BCRYPT_ECDSA_P384_ALG_HANDLE            ((BCRYPT_ALG_HANDLE) 0x000002f1)
#define BCRYPT_ECDSA_P521_ALG_HANDLE            ((BCRYPT_ALG_HANDLE) 0x00000301)
#define BCRYPT_RSA_SIGN_ALG_HANDLE              ((BCRYPT_ALG_HANDLE) 0x00000311)
#define BCRYPT_CAPI_KDF_ALG_HANDLE              ((BCRYPT_ALG_HANDLE) 0x00000321)
#define BCRYPT_PBKDF2_ALG_HANDLE                ((BCRYPT_ALG_HANDLE) 0x00000331)
#define BCRYPT_SP800108_CTR_HMAC_ALG_HANDLE     ((BCRYPT_ALG_HANDLE) 0x00000341)
#define BCRYPT_SP80056A_CONCAT_ALG_HANDLE       ((BCRYPT_ALG_HANDLE) 0x00000351)
#define BCRYPT_TLS1_1_KDF_ALG_HANDLE            ((BCRYPT_ALG_HANDLE) 0x00000361)
#define BCRYPT_TLS1_2_KDF_ALG_HANDLE            ((BCRYPT_ALG_HANDLE) 0x00000371)
#define BCRYPT_XTS_AES_ALG_HANDLE               ((BCRYPT_ALG_HANDLE) 0x00000381)
#endif /* !defined(BCRYPT_ALG_HANDLE) */

#endif /* _bcryptdefines_h */

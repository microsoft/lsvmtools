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
#ifndef _luks_h
#define _luks_h

#include "config.h"
#include <lsvmutils/eficommon.h>
#include <lsvmutils/blkdev.h>

#define LUKS_MAGIC_INITIALIZER { 'L', 'U', 'K', 'S', 0xba, 0xbe }
#define LUKS_SALT_SIZE 32
#define LUKS_SECTOR_SIZE 512
#define LUKS_MAGIC_SIZE 6
#define LUKS_CIPHER_NAME_SIZE 32
#define LUKS_CIPHER_MODE_SIZE 32
#define LUKS_HASH_SPEC_SIZE 32
#define LUKS_DIGEST_SIZE 20
#define LUKS_UUID_STRING_SIZE 40
#define LUKS_SLOTS_SIZE 8

typedef enum _LUKSCryptMode
{
    LUKS_CRYPT_MODE_DECRYPT = 0,
    LUKS_CRYPT_MODE_ENCRYPT = 1
}
LUKSCryptMode;

typedef struct
{
    UINT32 enabled;
    UINT32 password_iters;
    UINT8 password_salt[LUKS_SALT_SIZE];
    UINT32 key_material_offset;
    UINT32 af_stripes;
} 
LUKSKeySlot;

typedef struct
{
    UINT8 magic[LUKS_MAGIC_SIZE];
    UINT16 version;
    char cipher_name[LUKS_CIPHER_NAME_SIZE];
    char cipher_mode[LUKS_CIPHER_MODE_SIZE];
    char hash_spec[LUKS_HASH_SPEC_SIZE];
    UINT32 payload_offset;
    UINT32 key_bytes;
    UINT8 mk_digest[LUKS_DIGEST_SIZE];
    UINT8 mk_digest_salt[LUKS_SALT_SIZE];
    UINT32 mk_digest_iter;
    char uuid[LUKS_UUID_STRING_SIZE];
    LUKSKeySlot slots[LUKS_SLOTS_SIZE];
} 
LUKSHeader;

int LUKSReadHeader(
    Blkdev* rawdev,
    LUKSHeader* header);

void LUKSFixByteOrder(
    LUKSHeader *header);

int LUKSDumpHeader(
    const LUKSHeader* header);

int LUKSGetMasterKey(
    Blkdev* rawdev,
    const LUKSHeader *header, 
    const UINT8 *passphrase,
    UINTN passphraseSize,
    UINT8 *masterKey);

int LUKSCrypt(
    LUKSCryptMode mode,
    const LUKSHeader *header, 
    const UINT8 *key, 
    const UINT8 *dataIn, 
    UINT8 *dataOut, 
    UINTN dataSize, 
    UINT64 sectorNumber);

int LUKSGetPayloadSector(
    Blkdev* rawdev,
    const LUKSHeader *header,
    const UINT8* masterKey,
    UINTN sectorNumber,
    UINT8 sector[LUKS_SECTOR_SIZE]);

int LUKSPutPayloadSector(
    Blkdev* rawdev,
    const LUKSHeader *header,
    const UINT8* masterKey,
    UINTN sectorNumber,
    const UINT8 sector[LUKS_SECTOR_SIZE]);

int LUKSComputeMKDigest(
    const LUKSHeader* header,
    const UINT8* masterKey,
    UINT8 mkDigest[LUKS_DIGEST_SIZE]);

BOOLEAN LUKSMatchHeader(
    const UINT8 sectors[2 * LUKS_SECTOR_SIZE], /* Contains raw LUKS header */
    const UINT8* masterkeyData,
    UINT32 masterkeySize);

int LUKSFindBootDevice(
    const char* path, /* example: /dev/sda */
    const UINT8* masterkeyData,
    UINT32 masterkeySize,
    UINT8 uuid[16],
    UINT32* partitionNumber,
    UINT32* availPartitionNumber);

/*
**==============================================================================
**
** Portability: 
**
**==============================================================================
*/

typedef struct _LUKSCipher LUKSCipher;

void LUKSInitialize();

void LUKSShutdown();

LUKSCipher* LUKSGetAES256ECBCipher(
    const UINT8* key,
    UINTN keySize);

LUKSCipher* LUKSGetCipher(
    const LUKSHeader* header,
    const UINT8* key,
    UINTN keySize);

void LUKSReleaseCipher(
    LUKSCipher* cipher);

int LUKSCryptData(
    LUKSCipher* cipher,
    LUKSCryptMode mode,
    UINT8 *iv,
    UINTN ivSize,
    const UINT8 *in,
    UINTN inSize,
    UINT8 *out);

int LUKSDeriveKey(
    const char *pass, 
    int passlen,
    const unsigned  char* salt,
    int saltlen,
    int iter,
    const char* hashspec,
    int keylen,
    unsigned char* out);

#endif /* _luks_h */

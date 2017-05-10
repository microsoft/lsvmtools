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
#include "tpm2.h"
#include "dump.h"
#include "gpt.h"
#include "luksblkdev.h"
#include "ext2.h"

#if !defined(BUILD_EFI)
# include <stdio.h>
#endif

/*
**==============================================================================
**
** Private definitions:
**
**==============================================================================
*/

#if defined(BUILD_EFI)
# define VSNPRINTF VSPrint
# define SNPRINTF SPrint
#else
# define VSNPRINTF vsnprintf
# define SNPRINTF snprintf
#endif

#define LUKS_VERSION 1
#define LUKS_SLOT_ENABLED 0x00ac71f3
#define LUKS_SLOT_DISABLED 0x0000dead
#define LUKS_SECTOR_SIZE 512
#define LUKS_IV_SIZE 16

static UINT8 _magic[LUKS_MAGIC_SIZE] = LUKS_MAGIC_INITIALIZER;

void LUKSFixByteOrder(
    LUKSHeader *header)
{
    int i;

    if (!IS_BIG_ENDIAN)
    {
        header->version = ByteSwapU16(header->version);
        header->payload_offset = ByteSwapU32(header->payload_offset);
        header->key_bytes = ByteSwapU32(header->key_bytes);
        header->mk_digest_iter = ByteSwapU32(header->mk_digest_iter);

        for (i = 0; i < LUKS_SLOTS_SIZE; i++)
        {
            LUKSKeySlot* slot = &header->slots[i];
            slot->enabled = ByteSwapU32(slot->enabled);
            slot->password_iters = ByteSwapU32(slot->password_iters);
            slot->key_material_offset = ByteSwapU32(slot->key_material_offset);
            slot->af_stripes = ByteSwapU32(slot->af_stripes);
        }
    }
}

static void _PrintHexByte(UINT8 x)
{
# if defined(BUILD_EFI)
    {
        CHAR16 buf[9];
        UINTN n = SPrint(buf, sizeof(buf), L"%X", x);

        if (n == 8)
        {
            UINTN i;

            for (i = 0; i < n; i++)
                buf[i] = Tolower(buf[i]);

            Print(L"%s", buf + 6);
        }
        else
            Print(L"??");
    }
# else
    Print(TCS("%02x"), x);
# endif
}

static void _PrintHexBytes(const UINT8* data, UINTN size)
{
    int i;

    for (i = 0; i < size; i++)
    {
        _PrintHexByte(data[i]);

        if (i + 1 != size)
            Print(TCS(" "));
    }

    Print(TCS("\n"));
}

static void _DumpKeySlot(
    const LUKSKeySlot* slot)
{
    if (slot->enabled == LUKS_SLOT_DISABLED)
    {
        Print(TCS("DISABLED\n"));
        return;
    }

    if (slot->enabled != LUKS_SLOT_ENABLED)
    {
        /* ignore error */
        return;
    }

    Print(TCS("ENABLED\n"));

    Print(TCS("\tIterations:\t\t%d\n"), slot->password_iters);

    Print(TCS("\tSalt:\t\t\t"));

    _PrintHexBytes(slot->password_salt, LUKS_SALT_SIZE/2);
    Print(TCS("\t\t\t\t"));
    _PrintHexBytes(slot->password_salt + LUKS_SALT_SIZE/2, LUKS_SALT_SIZE/2);

    Print(TCS("\tKey material offset:\t%d\n"), slot->key_material_offset);
    Print(TCS("\tAF stripes:\t\t%d\n"), slot->af_stripes);
}

static int _GenIV(
    const LUKSHeader *header,
    UINT64 sec, 
    UINT8 *iv, 
    const UINT8 *key)
{
    int rc = -1;
    int ivSize = 8;
    LUKSCipher* cipher = NULL;

    if (Strcmp("ecb", header->cipher_mode) == 0)
    {
        rc = 0;
        goto done;
    }

    if (Strcmp("cbc-plain", header->cipher_mode) == 0)
        ivSize = 4;

    Memcpy(iv, &sec, ivSize);

    if (Strcmp("xts-plain64", header->cipher_mode) == 0)
    {
        rc = 0;
        goto done;
    }

    if (Strcmp("cbc-plain", header->cipher_mode) == 0)
    {
        rc = 0;
        goto done;
    }

    SHA256Hash sha256;
    UINT8 temp[LUKS_IV_SIZE] = { 0 };
    ComputeSHA256(key, header->key_bytes, &sha256);
    Memcpy(temp, &sec, ivSize);

    if (!(cipher = LUKSGetAES256ECBCipher(sha256.buf, sizeof(sha256))))
        goto done;

    if (LUKSCryptData(
        cipher,
        LUKS_CRYPT_MODE_ENCRYPT,
        NULL, /* iv */
        0, /* ivSize */
        temp,  /* in */
        LUKS_IV_SIZE, /* inSize */
        iv) != 0) /* out */
    {
        goto done;
    }

    rc = 0;

done:

    if (cipher)
        LUKSReleaseCipher(cipher);

    return rc;
}

static int _DiffuseSHA(
    SHAAlgorithm alg,
    UINT8 *data, 
    UINT32 bytes) 
{
    UINT32 i;
    INT32 hashSize;
    UINT32 pad;

    hashSize = SHAHashSize(alg);
    if (hashSize < 0)
        return -1;
    
    pad = (bytes % ((UINT32) hashSize)) != 0;
    for (i = 0; i < bytes / hashSize + pad; i++)
    {
        __SHAContext ctx;
        __SHAHash hash;
        UINT32 iv = ByteSwapU32(i);
        int size;

        if (!__SHAInit(&ctx, alg))
            return -1;

        if (!__SHAUpdate(&ctx, (UINT8*)&iv, sizeof(iv)))
            return -1;      

        size = hashSize;
        if (size > bytes - i * hashSize) 
            size = bytes - i * hashSize;

        if (!__SHAUpdate(&ctx, data + i * hashSize, size))
            return -1;     

        if (!__SHAFinal(&ctx, &hash))
            return -1;
        
        Memcpy(data + i * hashSize, hash.buf, size);
    }
    return 0;
}

static int _Diffuse(
    const LUKSHeader* header,
    UINT8 *data, 
    UINT32 bytes)
{
    SHAAlgorithm alg;

    if (Strcmp(header->hash_spec, "sha1") == 0)
        alg = SHA1_ALG;
    else if (Strcmp(header->hash_spec, "sha256") == 0)
        alg = SHA256_ALG;
    else if (Strcmp(header->hash_spec, "sha384") == 0)
        alg = SHA384_ALG;
    else if (Strcmp(header->hash_spec, "sha512") == 0)
        alg = SHA512_ALG;
    else
        return -1;

    return _DiffuseSHA(alg, data, bytes);
}

static void _ComputeXOR(
    UINT8* x,
    const UINT8* lhs,
    const UINT8* rhs,
    UINTN size) 
{
    /* x = lhs ^ rhs */
    while (size--)
        *x++ = *lhs++ ^ *rhs++;
}

/* Merge the stripes into the master key */
static int _AFMerge(
    const LUKSHeader *header, 
    const LUKSKeySlot* slot, 
    const UINT8 *stripes, 
    UINT8 *masterKey)
{
    UINT32 i;

    Memset(masterKey, 0, header->key_bytes);

    /* For each stripe */
    for (i = 0; i < slot->af_stripes; i++)
    {
        const UINT8* stripe;

        /* Get pointer to current stripe */
        stripe = stripes + (i * header->key_bytes);

        /* XOR current stripe into the master key */
        _ComputeXOR(masterKey, masterKey, stripe, header->key_bytes);

        /* Diffuse all but the last stripe */
        if (i + 1 != slot->af_stripes)
        {
            if (_Diffuse(header, masterKey, header->key_bytes) != 0)
                return -1;
        }
    }

    return 0;
}

static int _UnsealMasterKey(
    Blkdev* rawdev,
    const LUKSHeader *header, 
    const LUKSKeySlot* slot, 
    const UINT8 *passphrase,
    UINTN passphraseSize,
    UINT8 *masterKey)
{
    int rc = -1;
    UINT8 *derivedKey = NULL;
    UINT8 *encryptedStripes = NULL;
    UINT8 *decryptedStripes = NULL;
    UINTN stripesBytes = 0;
    UINT8 mkDigest[LUKS_DIGEST_SIZE];

    /* Check for null parameters */
    if (!rawdev || !header || !slot || !passphrase || !masterKey)
        goto done;

    /* Allocate bytes for the derived key (used to decrypt the stripes) */
    if (!(derivedKey = (UINT8*)Malloc(header->key_bytes)))
        goto done;

    /* Compute the derived key from passphrase, salt, and iterations-count */
    if (LUKSDeriveKey(
        (const char*) passphrase,
        passphraseSize,
        slot->password_salt,
        LUKS_SALT_SIZE,
        slot->password_iters,
        header->hash_spec,
        header->key_bytes,
        derivedKey) != 0)
    {
        goto done;
    }

    /* Compute the total size of the stripes */
    stripesBytes = header->key_bytes * slot->af_stripes;

    /* Allocate space for the encrypted stripes */
    if (!(encryptedStripes = (UINT8*)Malloc(stripesBytes)))
        goto done;

    /* Allocate space for the decrypted stripes */
    if (!(decryptedStripes = (UINT8*)Malloc(stripesBytes)))
        goto done;
      
    /* Read the key material (stripes) into memory */
    if ((BlkdevRead(
        rawdev,
        slot->key_material_offset,
        encryptedStripes, 
        stripesBytes)) != 0)
    {
        goto done;
    }

    /* Decrypt the stripes */
    if (LUKSCrypt(
        LUKS_CRYPT_MODE_DECRYPT,
        header,
        derivedKey, 
        encryptedStripes,
        decryptedStripes,
        stripesBytes, 
        0) != 0)
    {
        goto done;
    }

    /* Merge the split stripes into the unsplit master key */
    if (_AFMerge(header, slot, decryptedStripes, masterKey) != 0)
        goto done;

    /* Compute the digest of the master key */
    if (LUKSDeriveKey(
        (char*)masterKey,
        header->key_bytes,
        header->mk_digest_salt,
        LUKS_SALT_SIZE,
        header->mk_digest_iter,
        header->hash_spec,
        LUKS_DIGEST_SIZE,
        mkDigest) != 0)
    {
        goto done;
    }

    /* Verify that the master key digest matches the one in LUKS header */
    if (Memcmp(mkDigest, header->mk_digest, LUKS_DIGEST_SIZE) != 0)
    {
        goto done;
    }

    rc = 0;

done:

    if (derivedKey)
        Free(derivedKey);

    if (encryptedStripes)
        Free(encryptedStripes);

    if (decryptedStripes)
        Free(decryptedStripes);

    return rc;
}

/*
**==============================================================================
**
** Public definitions:
**
**==============================================================================
*/

int LUKSReadHeader(
    Blkdev* rawdev,
    LUKSHeader* header)
{
    int rc = -1;

    /* Reject null parameters */
    if (!rawdev || !header)
    {
        rc = -2;
        goto done;
    }

    /* Read the header */
    if (BlkdevRead(
        rawdev, 
        0, /* Block 0 */
        header, 
        sizeof(LUKSHeader)) != 0)
    {
        rc = -4;
        goto done;
    }

    /* Check the magic number */
    if (Memcmp(header->magic, _magic, sizeof(_magic)) != 0)
    {
        rc = -5;
        goto done;
    }

    /* Adjust byte order from big-endian to native */
    LUKSFixByteOrder(header);

    rc = 0;

done:
    return rc;
}

int LUKSDumpHeader(
    const LUKSHeader* header)
{
    int rc = -1;
    int i;

    /* Reject null parameters */
    if (!header)
    {
        rc = -2;
        goto done;
    }

    Print(TCS("Header Size: %d"), (int)sizeof(LUKSHeader));

    Print(TCS("\n"));

    Print(TCS("Version:\t%d\n"), header->version);

#if defined(BUILD_EFI)
    Print(TCS("Cipher name:\t%a\n"), header->cipher_name);
    Print(TCS("Cipher mode:\t%a\n"), header->cipher_mode);
    Print(TCS("Hash spec:\t%a\n"), header->hash_spec);
#else
    Print(TCS("Cipher name:\t%s\n"), header->cipher_name);
    Print(TCS("Cipher mode:\t%s\n"), header->cipher_mode);
    Print(TCS("Hash spec:\t%s\n"), header->hash_spec);
#endif

    Print(TCS("Payload offset:\t%d\n"), header->payload_offset);

    Print(TCS("MK bits:\t%d\n"), header->key_bytes * 8);

    Print(TCS("MK digest\t"));
    _PrintHexBytes(header->mk_digest, LUKS_DIGEST_SIZE);

    Print(TCS("MK salt:\t"));
    _PrintHexBytes(header->mk_digest_salt, LUKS_SALT_SIZE/2);
    Print(TCS("\t\t"));
    _PrintHexBytes(header->mk_digest_salt + LUKS_SALT_SIZE/2, LUKS_SALT_SIZE/2);

    Print(TCS("MK iterations:\t%d\n"), header->mk_digest_iter);

#if defined(BUILD_EFI)
    Print(TCS("UUID:\t\t%a\n"), header->uuid);
#else
    Print(TCS("UUID:\t\t%s\n"), header->uuid);
#endif

    Print(TCS("\n"));

    for (i = 0; i < LUKS_SLOTS_SIZE; i++)
    {
        Print(TCS("Key Slot %d: "), i);
        _DumpKeySlot(&header->slots[i]);
    }

    rc = 0;

done:
    return rc;
}

int LUKSGetMasterKey(
    Blkdev* rawdev,
    const LUKSHeader *header, 
    const UINT8 *passphrase,
    UINTN passphraseSize,
    UINT8 *masterKey)
{
    int rc = -1;
    int i;

    LUKSInitialize();

    for (i = 0; i < LUKS_SLOTS_SIZE; i++) 
    {
        const LUKSKeySlot* slot = &header->slots[i];

        if (slot->enabled == LUKS_SLOT_DISABLED) 
            continue;

        if (slot->enabled == LUKS_SLOT_ENABLED) 
        {
            rc = _UnsealMasterKey(rawdev, header, slot, passphrase, passphraseSize, masterKey);

            if (rc == 0)
                goto done;
        } 
    }

    /* Not found! */

done:

    return rc;
}

int LUKSCrypt(
    LUKSCryptMode mode,
    const LUKSHeader *header,
    const UINT8 *key,
    const UINT8 *dataIn,
    UINT8 *dataOut,
    UINTN dataSize,
    UINT64 sector)
{
    int rc = -1;
    LUKSCipher* cipher = NULL;
    UINT8 iv[LUKS_IV_SIZE];
    UINT64 i;

    LUKSInitialize();

    /* Get the EVP cipher */
    if (!(cipher = LUKSGetCipher(header, key, header->key_bytes)))
        goto done;

    UINT64 iters = dataSize / LUKS_SECTOR_SIZE;
    UINT64 block_len = LUKS_SECTOR_SIZE;

    if (Strcmp(header->cipher_mode, "ecb") == 0) 
    {
        iters = 1;
        block_len = dataSize;
    }

    for (i = 0; i < iters; i++) 
    {
        UINT64 pos;

        Memset(iv, 0, LUKS_IV_SIZE);

        if (_GenIV(header, sector + i, iv, key) == -1)
            goto done;

        pos = i * block_len;

        if(LUKSCryptData(
            cipher, 
            mode, 
            iv, 
            LUKS_IV_SIZE,
            dataIn + pos, 
            block_len,
            dataOut + pos) == -1)
        {
            goto done;
        }
    }

    rc = 0;

done:
    return rc;
}

int LUKSGetPayloadSector(
    Blkdev* rawdev,
    const LUKSHeader *header, 
    const UINT8* masterKey,
    UINTN sectorNumber,
    UINT8 sector[LUKS_SECTOR_SIZE])
{
    int rc = -1;
    UINT8 data[LUKS_SECTOR_SIZE];

    LUKSInitialize();

    /* Check parameters */
    if (!rawdev || !header || !masterKey || !sector)
        goto done;

    /* Read the encrypted sector */
    if (BlkdevRead(
        rawdev,
        header->payload_offset + sectorNumber,
        data,
        LUKS_SECTOR_SIZE) != 0)
    {
        goto done;
    }

    /* Decrypt the sector with the master key */
    if (LUKSCrypt(
        LUKS_CRYPT_MODE_DECRYPT,
        header,
        masterKey,
        data,
        sector,
        LUKS_SECTOR_SIZE,
        sectorNumber) != 0)
    {
        goto done;
    }

    rc = 0;

done:

    return rc;
}

int LUKSPutPayloadSector(
    Blkdev* rawdev,
    const LUKSHeader *header,
    const UINT8* masterKey,
    UINTN sectorNumber,
    const UINT8 sector[LUKS_SECTOR_SIZE])
{
    int rc = -1;
    UINT8 data[LUKS_SECTOR_SIZE];

    LUKSInitialize();

    /* Check parameters */
    if (!rawdev || !header || !masterKey || !sector)
        goto done;

    /* Decrypt the sector with the master key */
    if (LUKSCrypt(
        LUKS_CRYPT_MODE_ENCRYPT,
        header,
        masterKey,
        sector,
        data,
        LUKS_SECTOR_SIZE,
        sectorNumber) != 0)
    {
        goto done;
    }

    /* Write the encrypted sector */
    if (BlkdevWrite(
        rawdev,
        header->payload_offset + sectorNumber,
        data,
        LUKS_SECTOR_SIZE) != 0)
    {
        goto done;
    }

    rc = 0;

done:

    return rc;
}

int LUKSComputeMKDigest(
    const LUKSHeader* header,
    const UINT8* masterKey,
    UINT8 mkDigest[LUKS_DIGEST_SIZE])
{
    LUKSInitialize();

    /* Compute the digest of the master key */
    if (LUKSDeriveKey(
        (char*)masterKey,
        header->key_bytes,
        header->mk_digest_salt,
        LUKS_SALT_SIZE,
        header->mk_digest_iter,
        header->hash_spec,
        LUKS_DIGEST_SIZE,
        mkDigest) != 0)
    {
        return -1;
    }

    return 0;
}

BOOLEAN LUKSMatchHeader(
    const UINT8 sectors[2 * LUKS_SECTOR_SIZE], /* Contains raw LUKS header */
    const UINT8* masterkeyData,
    UINT32 masterkeySize)
{
    BOOLEAN result = FALSE;
    union 
    {
        LUKSHeader header;
        UINT8 blocks[2 * LUKS_SECTOR_SIZE];
    }
    u;
    UINT8 mkDigest[LUKS_DIGEST_SIZE];
    static const UINT8 magic[] = LUKS_MAGIC_INITIALIZER;

    if (!sectors || !masterkeyData || !masterkeySize)
        return FALSE;

    /* Read the LUKS header (blkno == 0) */
    Memcpy(&u, sectors, 2 * LUKS_SECTOR_SIZE);

    /* If the header does not have the LUKS magic number */
    if (Memcmp(u.header.magic, magic, sizeof(magic)) != 0)
        goto done;

    /* Fix the byte order of the hader */
    LUKSFixByteOrder(&u.header);

    /* If unsealed key is the wrong size it is from wrong parition */
    if (masterkeySize != u.header.key_bytes)
        goto done;

    /* Compute MK-digest of the masterkey */
    if (LUKSComputeMKDigest(&u.header, masterkeyData, mkDigest) != 0)
        goto done;

    /* Check whether this masterkey matches the MK-digest in the header */
    if (Memcmp(u.header.mk_digest, mkDigest, LUKS_DIGEST_SIZE) != 0)
        goto done;

    result = TRUE;

done:

    return result;
}

#if !defined(BUILD_EFI)
int LUKSFindBootDevice(
    const char* path, /* example: /dev/sda */
    const UINT8* masterkeyData,
    UINT32 masterkeySize,
    UINT8 uuid[16],
    UINT32* partitionNumber,
    UINT32* availPartitionNumber)
{
    int rc = -1;
    GPT gpt;
    UINTN i;
    EXT2* ext2 = NULL;

    /* Check parameters */
    if (!path || !masterkeyData || !masterkeySize || !uuid || 
        !availPartitionNumber)
    {
        goto done;
    }

    /* Initialize parameters */
    Memset(uuid, 0, 16);
    *partitionNumber = 0;
    *availPartitionNumber = 0;

    /* Load the GPT */
    if (LoadGPT(path, &gpt) != 0)
        goto done;

    /* Try each parition */
    for (i = 0; i < GPT_MAX_ENTRIES && gpt.entries[i].typeGUID1; i++)
    {
        char devname[32];
        U32tostrBuf buf;
        Blkdev* dev = NULL;
        Blkdev* luksdev = NULL;

        /* Format device name: example: "/dev/sda2" */
        Strlcpy(devname, path, sizeof(devname));
        Strlcat(devname, U32tostr(&buf, i + 1), sizeof(devname));

        /** Create object chain: [ext2] -> [luksdev] -> [dev] **/

        /* Open this block device */
        if (!(dev = BlkdevOpen(devname, BLKDEV_ACCESS_RDWR, 0)))
            goto done;

        /* Try to open as a LUKS device */
        if (!(luksdev = LUKSBlkdevFromMasterkey(
            dev,
            masterkeyData,
            masterkeySize)))
        {
            /* Not a valid LUKS device */
            dev->Close(dev);
            dev = NULL;
            continue;
        }

        /* Try to create EXT2 file system (will fail if not EXT2) */
        if (EXT2New(luksdev, &ext2) != EXT2_ERR_NONE)
        {
            /* Not a valid EXT2 file system */
            luksdev->Close(luksdev);
            continue;
        }

        /* Found valid ext2 */
        *partitionNumber = i + 1;
        break;
    }

    if (!ext2)
        goto done;

    Memcpy(uuid, ext2->sb.s_uuid, sizeof(ext2->sb.s_uuid));

    *availPartitionNumber = CountGPTEntries(&gpt) + 1;

    rc = 0;

done:

    if (ext2)
        EXT2Delete(ext2);

    return rc;
}
#endif /* !defined(BUILD_EFI) */

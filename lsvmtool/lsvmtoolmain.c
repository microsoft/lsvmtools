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
#include <ctype.h>
#include <errno.h>

#if defined(HAVE_OPENSSL)
# include <openssl/aes.h> 
# include <openssl/rand.h> 
#endif

#if defined(__linux__)
# include <unistd.h>
# include <sys/time.h> 
# include <sys/stat.h>
# include <sys/types.h> 
# include <sys/wait.h> 
# include <dirent.h> 
# include <fcntl.h>
# include <crypt.h>
#endif

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <lsvmutils/tpm2.h>
#include <lsvmutils/tpm2.h>
#include <lsvmutils/measure.h>
#include <lsvmutils/peimage.h>
#include <lsvmutils/conf.h>
#include <lsvmutils/getopt.h>
#include <lsvmutils/file.h>
#include <lsvmutils/linux.h>
#include <lsvmutils/luks.h>
#include <lsvmutils/dump.h>
#include <lsvmutils/ext2.h>
#include <lsvmutils/gpt.h>
#include <lsvmutils/guid.h>
#include <lsvmutils/luks.h>
#include <lsvmutils/alloc.h>
#include <lsvmutils/uefidb.h>
#include <lsvmutils/uefidbx.h>
#include <lsvmutils/cpio.h>
#include <lsvmutils/initrd.h>
#include <lsvmutils/utils.h>
#include <lsvmutils/grubcfg.h>
#include <lsvmutils/strarr.h>
#include <lsvmutils/grubcfg.h>
#include <lsvmutils/pass.h>
#include <lsvmutils/keys.h>
#include <xz/lzmaextras.h>
#include <lsvmutils/policy.h>
#include <lsvmutils/lsvmloadpolicy.h>
#include <lsvmutils/specialize.h>
#include <zlib.h>
#include "zlibextras.h"
#include "dbxupdate.h"

#if defined(__linux__)
# define ENABLE_LUKS
# define ENABLE_BINS_COMMAND
#endif


static const char* arg0;

void __posix_panic(const char* func)
{
    fprintf(stderr, "__posix_panic(): %s\n", func);
    exit(1);
}

void __posix_warn(const char* func)
{
    fprintf(stderr, "__posix_warn(): %s\n", func);
    exit(1);
}

void __efi_trace(const char* file, unsigned int line)
{
}

int posix_errno = 0;

static EFI_TCG2_PROTOCOL* _GetTPMProtocol()
{
    static EFI_TCG2_PROTOCOL* protocol = NULL;

    /* If TCG/TMP uninitialized (lsvmtool_main() can be called reentrantly) */
    if (!protocol)
    {
        /* Get the TCG2 protocol */
        if (!(protocol = TCG2_GetProtocol()))
        {
            fprintf(stderr, "Warning: TCG2_GetProtocol() failed\n");
        }

#if defined(__linux__)
        /* Initialize TPM parameters */
        {
            TPM_RC rc;

            if ((rc = TPM2X_SetDictionaryAttackLockReset(
                protocol)) != TPM_RC_SUCCESS)
            {
                fprintf(stderr, 
                    "%s: TPM2X_SetDictionaryAttackLockReset() failed\n", arg0);
                exit(1);
            }

            if ((rc = TPM2X_SetLockoutParams(protocol)) != TPM_RC_SUCCESS)
            {
                fprintf(stderr, "%s: TPM2X_SetLockoutParams() failed\n", arg0);
                exit(1);
            }
        }
#endif /* defined(__linux__) */
    }

    return protocol;
}

static void _TestIsTPMPresent()
{
    EFI_TCG2_PROTOCOL* protocol = _GetTPMProtocol();
    EFI_STATUS status;
    EFI_TCG2_BOOT_SERVICE_CAPABILITY capability;

    printf("=== TestIsTPMPresent()\n");

    status = TCG2_GetCapability(protocol, &capability);

    if (status != EFI_SUCCESS)
        assert(0);

    assert(capability.TPMPresentFlag == TRUE);
}

static void _TestGetTPMRevision()
{
    EFI_TCG2_PROTOCOL* protocol = _GetTPMProtocol();
    UINT32 revision;
    TPM_RC rc;

    printf("=== TestGetTPMRevision()\n");
    
    if ((rc = TPM2X_GetTPMRevision(
        protocol, 
        &revision)) != TPM_RC_SUCCESS)
    {
        fprintf(stderr, "TPM2_Utils_GetTPMRevision() failed: %08X\n", rc);
        exit(1);
    }

    /* assert(revision == 0x00000067); */
}

static void _TestSetDictionaryAttackLockReset()
{
    EFI_TCG2_PROTOCOL* protocol = _GetTPMProtocol();
    TPM_RC rc;

    printf("=== TestSetDictionaryAttackLockReset()\n");

    rc = TPM2X_SetDictionaryAttackLockReset(protocol);
    assert(rc == TPM_RC_SUCCESS);
}

static void _TestSetLockoutParams()
{
    EFI_TCG2_PROTOCOL* protocol = _GetTPMProtocol();
    TPM_RC rc;

    printf("=== TestSetLockoutParams()\n");

    rc = TPM2X_SetLockoutParams(protocol);
    assert(rc == TPM_RC_SUCCESS);
}

static void _TestCreateSRKKey()
{
    EFI_TCG2_PROTOCOL* protocol = _GetTPMProtocol();
    TPM_HANDLE srkHandle;
    TPM_RC rc;

    printf("=== TestCreateSRKKey()\n");

    rc = TPM2X_CreateSRKKey(protocol, &srkHandle);
    assert(rc == TPM_RC_SUCCESS);

    rc = TPM2_FlushContext(protocol, srkHandle);
    assert(rc == TPM_RC_SUCCESS);
}

static void _TestStartAuthSession()
{
    EFI_TCG2_PROTOCOL* protocol = _GetTPMProtocol();
    TPM_RC rc;
    TPMI_SH_AUTH_SESSION sessionHandle;

    printf("=== TestStartAuthSession()\n");

    if ((rc = TPM2X_StartAuthSession(
        protocol, 
        TRUE,
        TPM_ALG_SHA256,
        &sessionHandle)) != TPM_RC_SUCCESS)
    {
        fprintf(stderr, "TPM2X_StartAuthSession() failed: %08X\n", rc);
        exit(1);
    }

    if ((rc = TPM2_FlushContext(protocol, sessionHandle)) != TPM_RC_SUCCESS)
    {
        fprintf(stderr, "TPM2_FlushContext() failed: %08X\n", rc);
        exit(1);
    }
}

/*
**==============================================================================
**
** _IsDirectory()
**
**==============================================================================
*/

#if defined(ENABLE_BINS_COMMAND)
static int _IsDirectory(const char* path, int* flag)
{
    struct stat st;

    if (stat(path, &st) != 0)
    {
        return -1;
    }

    *flag = S_ISDIR(st.st_mode) ? 1 : 0;
    return 0;
}
#endif /* defined(ENABLE_BINS_COMMAND) */

/*
**==============================================================================
**
** _FindFile()
**
**     Find the under the given directory with the 'prefix' and 'suffix'
**
**==============================================================================
*/

#if defined(ENABLE_BINS_COMMAND)
int FindBins(const char* root)
{
    int rc = -1;
    DIR* dir = NULL;
    struct dirent* ent;
    char** paths = NULL;
    size_t npaths = 0;
    size_t i;

    /* Open the directory */
    if (!(dir = opendir(root)))
        goto done;

    /* Read each entry in the directory */
    while ((ent = readdir(dir)))
    {
        const char* name = ent->d_name;
        char tmppath[PATH_MAX];
        int isdir;

        /* Skip ".." and "." directories */
        if (strcmp(name, "..") == 0 || strcmp(name, ".") == 0)
            continue;

        /* Form the full path of this file */
        strcpy(tmppath, root);
        strcat(tmppath, "/");
        strcat(tmppath, name);

        /* If directory */
        if (_IsDirectory(tmppath, &isdir) == 0 && isdir)
        {
            char** p;

            if (!(p = (char**)realloc(paths, sizeof(char*) * (npaths + 1))))
                goto done;

            paths = p;

            if (!(paths[npaths] = strdup(tmppath)))
                goto done;

            npaths++;
        }
        else
        {
            unsigned char* data;
            size_t size;
            size_t i;

            /* Load the file into memory */
            if (LoadFile(tmppath, 0, (unsigned char**)&data, &size) != 0)
            {
                return -1;
            }

            /* Check for binary characters */
            for (i = 0; i < size; i++)
            {
                if (data[i] > '~')
                {
                    /* Found binary file */
                    printf("%s\n", tmppath);
                    break;
                }
            }

            Free(data);
        }
    }

    closedir(dir);
    dir = NULL;

    /* Visit each subdirectory */
    for (i = 0; i < npaths; i++)
    {
        if (FindBins(paths[i]) != 0)
            goto done;
    }

    rc = 0;

done:

    /* Release the paths[] */
    if (paths)
    {
        for (i = 0; i < npaths; i++)
            free(paths[i]);

        free(paths);
    }

    /* Close the direcrtory */
    if (dir)
        closedir(dir);

    return rc;
}
#endif /* defined(ENABLE_BINS_COMMAND) */

static BOOLEAN _IsSHA1Str(const char* str)
{
    size_t len = strlen(str);

    if (len != 2 * sizeof(SHA1Hash))
    {
        return FALSE;
    }

    while (*str)
    {
        if (!isxdigit(*str++))
            return FALSE;
    }

    return TRUE;
}

static BOOLEAN _IsHexStr(const char* str)
{
    while (*str)
    {
        if (!isxdigit(*str++))
            return FALSE;
    }

    return TRUE;
}

static char* _LoadHexStr(const char* path)
{
    char* data;
    size_t size;
    char* end;

    /* Load the file into memory */
    if (LoadFile(path, 0, (unsigned char**)&data, &size) != 0)
    {
        return NULL;
    }

    /* Remove the trailing whitespace if any */
    end = data + size;
    while (end != data && isspace(end[-1]))
        *--end = '\0';

    /* If file does not contain a SHA1 hexadicimal string, then fail */
    if (!_IsHexStr(data))
    {
        free(data);
        return NULL;
    }

    return data;
}

static int _LoadSHA1Str(
    const char* path, 
    SHA1Str* sha1str)
{
    char* data;
    size_t size;
    char* end;

    /* Load the file into memory */
    if (LoadFile(path, 0, (unsigned char**)&data, &size) != 0)
        return -1;

    /* Remove the trailing whitespace if any */
    end = data + size;
    while (end != data && isspace(end[-1]))
        *--end = '\0';

    /* If file does not contain a SHA1 hexadicimal string, then fail */
    if (!_IsSHA1Str(data))
    {
        free(data);
        return -1;
    }

    Memcpy(sha1str, data, sizeof(SHA1Str));
    free(data);
    return 0;
}

/*
**==============================================================================
**
** Commands:
**
**==============================================================================
*/

static int _cappcr11_command(
    int argc, 
    const char* argv[])
{
    EFI_TCG2_PROTOCOL* protocol = _GetTPMProtocol();
    TPM_RC rc;

    /* Check arguments */
    if (argc != 1)
    {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        return 1;
    }

    if ((rc = TPM2X_Cap(protocol, 11)) != TPM_RC_SUCCESS)
    {
        fprintf(stderr, "%s: TPM2X_CapPCR11() failed: %08X\n", argv[0], rc);
        return 1;
    }

    return 0;
}

static int _capall_command(
    int argc, 
    const char* argv[])
{
    EFI_TCG2_PROTOCOL* protocol = _GetTPMProtocol();
    TPM_RC rc;

    /* Check arguments */
    if (argc != 1)
    {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        return 1;
    }

    if ((rc = TPM2X_CapAll(protocol)) != TPM_RC_SUCCESS)
    {
        fprintf(stderr, "%s: TPM2X_CapAll() failed: %08X\n", argv[0], rc);
        return 1;
    }

    return 0;
}

static int _hexdump_command(
    int argc, 
    const char* argv[])
{
    BYTE* data = NULL;
    size_t size = 0;

    /* Check arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s FILENAME\n", argv[0]);
        return 1;
    }


    /* Read the file into memory */
    if (LoadFile(argv[1], 0, &data, &size) != 0)
    {
        fprintf(stderr, "%s: failed to read file: '%s'\n", argv[0], argv[1]);
        return 1;
    }

    HexDump(data, size);
    return 0;
}

static int _asciidump_command(
    int argc, 
    const char* argv[])
{
    BYTE* data = NULL;
    size_t size = 0;

    /* Check arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s FILENAME\n", argv[0]);
        return 1;
    }


    /* Read the file into memory */
    if (LoadFile(argv[1], 0, &data, &size) != 0)
    {
        fprintf(stderr, "%s: failed to read file: '%s'\n", argv[0], argv[1]);
        return 1;
    }

    ASCIIDump(data, size);
    return 0;
}

static int _crypt_command(int argc, const char** argv, int encrypt)
{
    int rc = 0;
    const char* arg0;
    unsigned char* key = NULL;
    size_t keyBytes = 0;
    const char* infile;
    const char* outfile;
    unsigned char* in = NULL;
    unsigned char* out = NULL;
    size_t inSize;
    unsigned long outSize = 0;
    const char* keyfile = NULL;

    /* Extract the --keyfile option (if any) */
    if ((rc = GetOpt(&argc, argv, "--keyfile", &keyfile)) < 0)
    {
        fprintf(stderr, "%s: missing option argument: --keyfile\n", argv[0]);
        rc = -1;
        goto done;
    }

    rc = 0;

    /* Check usage */
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s [OPTIONS] INFILE OUTFILE\n", argv[0]);
        rc = -1;
        goto done;
    }

    /* Collect arguments */
    arg0 = argv[0];
    infile = argv[1];
    outfile = argv[2];

    /* Load the key */
    if (keyfile)
    {
        if (LoadFile(keyfile, 0, &key, &keyBytes) != 0)
        {
            fprintf(stderr, "%s: failed to read %s\n", argv[0], keyfile);
            rc = -1;
            goto done;
        }

        const UINT32 KEYSIZE = 32;

        if (keyBytes != KEYSIZE)
        {
            fprintf(stderr, "%s: bootkey must be %u bytes\n", argv[0],
                (unsigned int)(KEYSIZE));
            rc = -1;
            goto done;
        }
    }

    /* If no key yet, then ask for passphrase interactively */
    if (!key)
    {
        char passphrase[_SC_PASS_MAX] = { '\0' };
        SHA256Hash sha256;

        /* Ask for a passphrase */
        {
            char* pw;

            if (!(pw = getpass("passphrase: ")))
            {
                fprintf(stderr, "%s: failed to get passphrase\n", argv[0]);
                rc = -1;
                goto done;
            }

            if (Strlcpy(passphrase, pw, _SC_PASS_MAX) >= _SC_PASS_MAX)
            {
                fprintf(stderr, "%s: passphrase is too long\n", argv[0]);
                rc = -1;
                goto done;
            }
        }

        if (!ComputeSHA256(passphrase, strlen(passphrase), &sha256))
        {
            fprintf(stderr, "%s: failed to compute SHA-256 of passphrase\n",
                argv[0]);
            rc = -1;
            goto done;
        }

        if (!(key = (unsigned char*)malloc(sizeof(SHA256Hash))))
        {
            fprintf(stderr, "%s: out of memory\n", argv[0]);
            rc = -1;
            goto done;
        }

        memcpy(key, sha256.buf, sizeof(SHA256Hash));
        keyBytes = sizeof(SHA256Hash);
    }

    /* Load the input file into memory */
    if (LoadFile(infile, 0, &in, &inSize) != 0)
    {
        fprintf(stderr, "%s: cannot load '%s'\n", arg0, infile);
        rc = -1;
        goto done;
    }

    if (encrypt)
    {
        /* Encrypt the data */
        if (TPM2X_AES_CBC_Encrypt(
            in, 
            inSize, 
            key, 
            keyBytes * 8, 
            &out, 
            &outSize) != 0)
        {
            fprintf(stderr, "%s: TPM2X_AES_CBC_Encrypt() failed\n", arg0);
            rc = -1;
            goto done;
        }
    }
    else
    {
        /* Get the initialization vector */
        unsigned char iv[AES_BLOCK_SIZE];
        const unsigned int ivSize = sizeof(iv);
        const unsigned char* data;
        unsigned long size;

        if (inSize < AES_BLOCK_SIZE + sizeof(ULONG))
        {
            fprintf(stderr, "%s: not enough data\n", arg0);
            rc = -1;
            goto done;
        }

        data = in;
        size = inSize;

        /* Extract [IV] */
        memcpy(iv, data, AES_BLOCK_SIZE);
        data += AES_BLOCK_SIZE;
        size -= AES_BLOCK_SIZE;

        /* Extract [ULONG] */
        ULONG tmp;
        memcpy(&tmp, data, sizeof(ULONG));
        data += sizeof(ULONG);
        size -= sizeof(ULONG);

        /* Decrypt the data */
        if ((rc = TPM2X_AES_CBC_Decrypt(
            data, 
            size, 
            key, 
            keyBytes * 8, /* keyBits */
            iv,
            ivSize,
            &out, 
            &outSize)) != 0)
        {
            fprintf(stderr, "%s: TPM2X_AES_CBC_Decrypt(): %d \n", arg0, rc);
            rc = -1;
            goto done;
        }
    }

    /* Write the output file */
    {
        FILE* os;

        if ((os = fopen(outfile, "wb")) == NULL)
        {
            fprintf(stderr, "%s: failed to open:  '%s'\n", arg0, outfile);
            rc = -1;
            goto done;
        }

        if (fwrite(out, 1, outSize, os) != outSize)
        {
            fprintf(stderr, "%s: failed to write: '%s'\n", arg0, outfile);
            fclose(os);
            rc = -1;
            goto done;
        }

        fclose(os);
    }

done:

    if (key)
        free(key);

    if (in)
        free(in);

    if (out)
        free(out);

    return rc;
}

static int _encrypt_command(
    int argc, 
    const char** argv)
{
    return _crypt_command(argc, argv, 1);
}

static int _decrypt_command(
    int argc, 
    const char** argv)
{
    return _crypt_command(argc, argv, 0);
}

static void _RandomBytes(unsigned char* data, size_t size)
{
#if defined(HAVE_OPENSSL)

    RAND_pseudo_bytes(data, size);

#elif defined(__linux__)

    size_t i;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand(tv.tv_sec + tv.tv_usec);

    for (i = 0; i < size; i++)
        data[i] = rand() % 256;

#elif defined(_WIN32)

    size_t i;
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    srand(li.QuadPart);

    for (i = 0; i < size; i++)
        data[i] = rand() % 256;

#endif /* defined(_WIN32) */
}

static int _key_command(
    int argc, 
    const char** argv)
{
    int rc = 0;
    unsigned char key[MAX_RSA_KEY_BYTES/8];
    size_t i;

    /* Check usage */
    if (argc != 1)
    {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        rc = 1;
        goto done;
    }

    _RandomBytes(key, sizeof(key));

    for (i = 0; i < sizeof(key); i++)
        printf("%02X", key[i]);

    printf("\n");

done:
    return rc;
}

static int _sha1_command(
    int argc, 
    const char** argv)
{
    unsigned char* data;
    size_t size;
    SHA1Hash sha1;
    SHA1Str sha1Str;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s FILENAME\n", argv[0]);
        return 1;
    }

    if (LoadFile(argv[1], 0, &data, &size) != 0)
    {
        fprintf(stderr, "%s: failed to load file: '%s'\n", argv[0], argv[1]);
        return 1;
    }

    if (!ComputeSHA1(data, size, &sha1))
    {
        fprintf(stderr, "%s: ComputeSHA1() failed\n", argv[0]);
        return 1;
    }

    sha1Str = SHA1ToStr(&sha1);
    printf("%s\n", sha1Str.buf);

    return 0;
}

static int _sha256_command(
    int argc, 
    const char** argv)
{
    unsigned char* data;
    size_t size;
    SHA256Hash sha256;
    SHA256Str sha256Str;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s FILENAME\n", argv[0]);
        return 1;
    }

    if (LoadFile(argv[1], 0, &data, &size) != 0)
    {
        fprintf(stderr, "%s: failed to load file: '%s'\n", argv[0], argv[1]);
        return 1;
    }

    /* Non-aligned */
    {
        if (!ComputeSHA256(data, size, &sha256))
        {
            fprintf(stderr, "%s: ComputeSHA256() failed\n", argv[0]);
            return 1;
        }

        sha256Str = SHA256ToStr(&sha256);
        printf("%s\n", sha256Str.buf);
    }

    return 0;
}

static int _sha1hexstr_command(
    int argc, 
    const char** argv)
{
    int status = 0;
    BYTE* data = NULL;
    UINT32 size;
    const char* hexstr;
    UINT32 hexstrlen;

    /* Check arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s HEXSTR\n", argv[0]);
        status = 1;
        goto done;
    }

    /* Collect arguments */
    hexstr = argv[1];
    hexstrlen = strlen(hexstr);

    /* Hex string must be multiple of two */
    if (hexstrlen % 2)
    {
        fprintf(stderr, "%s: malformed hex string\n", argv[0]);
        status = 1;
        goto done;
    }

    /* Allocate buffer to hold binary data */
    if (!(data = malloc(size = hexstrlen / 2)))
    {
        fprintf(stderr, "%s: malloc failed\n", argv[0]);
        status = 1;
        goto done;
    }

    /* Convert to binary */
    if (TPM2X_HexStrToBinary(hexstr, hexstrlen, data, &size) != 0)
    {
        fprintf(stderr, "%s: malformed hex string\n", argv[0]);
        status = 1;
        goto done;
    }

    /* Find the hash */
    {
        SHA1Hash sha1;

        if (!ComputeSHA1(data, size, &sha1))
        {
            fprintf(stderr, "%s: ComputeSHA1() failed\n", argv[0]);
            status = 1;
            goto done;
        }

        DumpSHA1(&sha1);
    }

done:

    if (data)
        free(data);

    return status;
}

static int _sha256hexstr_command(
    int argc, 
    const char** argv)
{
    int status = 0;
    BYTE* data = NULL;
    UINT32 size;
    const char* hexstr;
    UINT32 hexstrlen;

    /* Check arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s HEXSTR\n", argv[0]);
        status = 256;
        goto done;
    }

    /* Collect arguments */
    hexstr = argv[1];
    hexstrlen = strlen(hexstr);

    /* Hex string must be multiple of two */
    if (hexstrlen % 2)
    {
        fprintf(stderr, "%s: malformed hex string\n", argv[0]);
        status = 1;
        goto done;
    }

    /* Allocate buffer to hold binary data */
    if (!(data = malloc(size = hexstrlen / 2)))
    {
        fprintf(stderr, "%s: malloc failed\n", argv[0]);
        status = 1;
        goto done;
    }

    /* Convert to binary */
    if (TPM2X_HexStrToBinary(hexstr, hexstrlen, data, &size) != 0)
    {
        fprintf(stderr, "%s: malformed hex string\n", argv[0]);
        status = 1;
        goto done;
    }

    /* Find the hash */
    {
        SHA256Hash sha256;

        if (!ComputeSHA256(data, size, &sha256))
        {
            fprintf(stderr, "%s: ComputeSHA256() failed\n", argv[0]);
            status = 256;
            goto done;
        }

        DumpSHA256(&sha256);
    }

done:

    if (data)
        free(data);

    return status;
}

static int _pcrs_command(
    int argc, 
    const char** argv)
{
    EFI_TCG2_PROTOCOL* protocol = _GetTPMProtocol();
    UINT32 i;

    for (i = 0; i < 16; i++)
    {
        TPM_RC rc;
        SHA1Hash sha1;
        SHA1Str sha1Str;

        if ((rc = TPM2X_ReadPCRSHA1(protocol, i, &sha1)) != TPM_RC_SUCCESS)
        {
            continue;
        }

        sha1Str = SHA1ToStr(&sha1);
        printf("PCR[%02u]=%s\n", i, sha1Str.buf);
    }

    return 0;
}

static int _pcrs256_command(
    int argc, 
    const char** argv)
{
    EFI_TCG2_PROTOCOL* protocol = _GetTPMProtocol();
    UINT32 i;

    for (i = 0; i < 16; i++)
    {
        TPM_RC rc;
        SHA256Hash sha256;
        SHA256Str sha256Str;

        if ((rc = TPM2X_ReadPCRSHA256(
            protocol,
            i,
            &sha256)) != TPM_RC_SUCCESS)
        {
            continue;
        }

        sha256Str = SHA256ToStr(&sha256);
        printf("PCR[%02u]=%s\n", i, sha256Str.buf);
    }

    return 0;
}

static int _extend_command(
    int argc, 
    const char** argv)
{
    const UINT32 NSHA1 = 32;
    SHA1Hash sha1[32];
    UINT32 nsha1 = 0;
    int i;

    /* Check the arguments */
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s SHA1...\n", argv[0]);
        return 1;
    }

    if (argc - 1 > NSHA1)
    {
        fprintf(stderr, "%s: too many SHA1 arguments (> %u)\n", argv[0], NSHA1);
        return 1;
    }

    /* Copy/convert SHA1 hex strings into sha1[] array */
    for (i = 1; i < argc; i++)
    {
        SHA1Str sha1str;

        if (_LoadSHA1Str(argv[i], &sha1str) != 0)
        {
            if (_IsSHA1Str(argv[i]))
            {
                strcpy(sha1str.buf, argv[i]);
            }
            else
            {
                fprintf(stderr, "%s: bad argument: '%s'\n", argv[0], argv[i]);
                return 1;
            }
        }

        SHA1FromStr(sha1str.buf, 2 * sizeof(SHA1Str), &sha1[nsha1++]);
    }

    /* Accumulate extended SHA1 values */
    {
        SHA1Hash prev;
        SHA1Str sha1Str;

        prev = sha1[0];

        for (i = 1; i < nsha1; i++)
        {
            typedef union _Pair
            {
                SHA1Hash left;
                SHA1Hash right;
            }
            Pair;
            Pair pair;

            pair.left = prev;
            pair.right = sha1[i];

            SHA1Hash tmp;

            if (!ComputeSHA1(&pair, sizeof(Pair), &tmp))
            {
                fprintf(stderr, "%s: ComputeSHA1() failed\n", argv[0]);
                return 1;
            }

            prev = tmp;
        }

        sha1Str = SHA1ToStr(&prev);
        printf("%s\n", sha1Str.buf);
    }

    return 0;
}

static int _GetPEHash(
    const char* path,
    SHA256Hash* sha256)
{
    int rc = -1;
    unsigned char* data = NULL;
    size_t size;
    SHA1Hash sha1;

    /* Read the file into memory */
    if (LoadFile(path, 0, &data, &size) != 0)
        goto done;

    /* Calculate the hash */
    if (ParseAndHashImage((char*)data, size, &sha1, sha256) != 0)
        goto done;

    rc = 0;

done:

    if (data) 
        free(data);

    return rc;
}

static int _pehash_command(
    int argc, 
    const char** argv)
{
    SHA256Hash sha256;
    SHA256Str sha256Str;

    /* Check the arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s FILENAME\n", argv[0]);
        return 1;
    }

    memset(&sha256, 0, sizeof(sha256));

    /* Compute the PE-Hash of this file */
    if (_GetPEHash(argv[1], &sha256) != 0)
    {
        fprintf(stderr, "%s: failed to calculate hash\n", argv[0]);
        return 1;
    }

    /* Print the SHA256 */
    sha256Str = SHA256ToStr(&sha256);
    printf("%s\n", sha256Str.buf);

    return 0;
}

static int _pehash1_command(
    int argc, 
    const char** argv)
{
    SHA1Hash sha1;
    SHA256Hash sha256;
    unsigned char* data;
    size_t size;

    /* Check the arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s FILENAME\n", argv[0]);
        return 1;
    }

    /* Read the file into memory */
    if (LoadFile(argv[1], 0, &data, &size) != 0)
    {
        fprintf(stderr, "%s: failed to read file: '%s'\n", argv[0], argv[1]);
        return 1;
    }

    /* Calculate the hash */
    {
        if (ParseAndHashImage((char*)data, size, &sha1, &sha256) != 0)
        {
            fprintf(stderr, "%s: failed to calculate hash\n", argv[0]);
            return 1;
        }

        /* Print the SHA1 */
        SHA1Str sha1Str;
        sha1Str = SHA1ToStr(&sha1);
        printf("%s\n", sha1Str.buf);
    }

    /* Release the data */
    free(data);

    return 0;
}

#if defined(HAVE_OPENSSL)
static int _peverify_command(
    int argc, 
    const char** argv)
{
    int status = 1;
    unsigned char* imageData = NULL;
    size_t imageSize;
    unsigned char* certData = NULL;
    size_t certSize;

    /* Check the arguments */
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s FILENAME DERCERTIFICATE\n", argv[0]);
        goto done;
    }

    /* Read the file into memory */
    if (LoadFile(argv[1], 0, &imageData, &imageSize) != 0)
    {
        fprintf(stderr, "%s: failed to read file: '%s'\n", argv[0], argv[1]);
        goto done;
    }

    /* Load the certificate (DER format) */
    if (LoadFile(argv[2], 0, &certData, &certSize) != 0)
    {
        fprintf(stderr, "%s: failed to load certificate: '%s'\n", argv[0],
            argv[2]);
        goto done;
    }

    /* Check the certificate */
    if (CheckCert(imageData, imageSize, certData, certSize) != 0)
    {
        fprintf(stderr, "%s: certificate check failed\n", argv[0]);
        goto done;
    }

    status = 0;

done:

    /* Release the data */
    if (imageData)
        free(imageData);

    /* Release the data */
    if (certData)
        free(certData);

    return status;
}
#endif /* defined(HAVE_OPENSSL) */

static int _gpt_command(
    int argc, 
    const char** argv)
{
    int status = 1;
    GPT gpt;
    UINTN i;

    /* Check the arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s DEVICE\n", argv[0]);
        goto done;
    }

    /* Read the GPT */
    if (LoadGPT(argv[1], &gpt) != 0)
    {
        fprintf(stderr, "%s: failed to load GPT: %s\n", argv[0], argv[1]);
        goto done;
    }

    /* Dump the GPT header */
    printf("=== GPT HEADER\n");
    DumpGPTHeader(&gpt.header);
    printf("\n");

    /* Dump the entries */
    for (i = 0; i < GPT_MAX_ENTRIES; i++)
    {
        if (gpt.entries[i].typeGUID1 == 0)
            break;

        printf("=== GPT ENTRY: partition=%d\n", (int)i + 1);
        DumpGPTEntry(&gpt.entries[i]);
        printf("\n");
    }

    /* Count GPT entries */
    printf("First available partition: %d\n", (int)CountGPTEntries(&gpt));

    status = 0;

done:

    return status;
}

#if defined(__linux__)
static int _bootdev_command(
    int argc, 
    const char** argv)
{
    int status = 1;
    const char* device;
    const char* masterkeyfile;
    UINT8* masterkeyData = NULL;
    size_t masterkeySize;
    UINT8 uuid[16];
    UINT32 partitionNumber;
    UINT32 availPartitionNumber;
    EFI_GUID guid;
    char guidstr[GUID_STRING_SIZE];

    /* Check the arguments */
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s DEVICE MASTERKEYFILE\n", argv[0]);
        goto done;
    }

    /* Collect arguments */
    device = argv[1];
    masterkeyfile = argv[2];

    /* Load the master key */
    if (LoadFile(masterkeyfile, 0, &masterkeyData, &masterkeySize) != 0)
    {
        fprintf(stderr, "%s: failed to read: %s\n", argv[0], argv[1]);
        goto done;
    }

    if (LUKSFindBootDevice(
        device,
        masterkeyData, 
        masterkeySize,
        uuid,
        &partitionNumber,
        &availPartitionNumber) != 0)
    {
        fprintf(stderr, "%s: LUKSFindBootDevice() failed\n", argv[0]);
        goto done;
    }

    MakeGUIDFromBytes(&guid, uuid);
    FormatGUID(guidstr, &guid);
    printf("UUID: %s\n", guidstr);
    printf("Partition: %u\n", partitionNumber);
    printf("Available Partition: %u\n", availPartitionNumber);

    status = 0;

done:

    if (masterkeyData)
        free(masterkeyData);

    return status;
}
#endif /* defined(__linux__) */

static int _grubcfginitrd_command(
    int argc, 
    const char** argv)
{
    int status = 1;
    const char* filename;
    char* data = NULL;
    size_t size;
    char matched[PATH_MAX];
    char title[PATH_MAX];
    char initrd[PATH_MAX];
    BOOLEAN verbose = FALSE;

    /* Check arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s FILENAME\n", argv[0]);
        goto done;
    }

    /* Extract the --verbose option */
    if (GetOpt(&argc, argv, "--verbose", NULL) == 1)
        verbose = TRUE;

    /* Collect arguments */
    filename = argv[1];

    /* Load the file into memory */
    if (LoadFile(filename, 0, (unsigned char**)&data, &size) != 0)
    {
        fprintf(stderr, "%s: failed to open: %s\n", argv[0], filename);
        goto done;
    }

    /* Find initrd */
    if (GRUBCfgFindInitrd(data, size, matched, title, initrd) != 0)
    {
        fprintf(stderr, "%s: failed to resolve: %s\n", argv[0], filename);
        goto done;
    }

    if (verbose)
    {
        printf("matched: %s\n", matched);
        printf("title: %s\n", title);
        printf("initrd: %s\n", initrd);
    }

    printf("%s\n", initrd);

    status = 0;

done:

    if (data)
        Free(data);

    return status;
}

static int _grubefidir_command(
    int argc, 
    const char** argv)
{
    int status = 1;
    const char* grubefi;
    UINT8* data;
    size_t size;
    const char* grubefidir = NULL;
    UINTN count = 0;

    /* Check the arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s GRUBEFI\n", argv[0]);
        goto done;
    }

    /* Collect arguments */
    grubefi = argv[1];

    /* Load the file into memory */
    if (LoadFile(grubefi, 1, &data, &size) != 0)
    {
        fprintf(stderr, "%s: failed to read: %s\n", argv[0], argv[1]);
        goto done;
    }

    /* Null terminate the data */
    data[size] = '\0';

    /* First make sure it is really a PE-COFF image */
    {
        SHA1Hash sha1;
        SHA256Hash sha256;

        if (ParseAndHashImage((char*)data, size, &sha1, &sha256) != 0)
        {
            fprintf(stderr, "%s: not a PE/COFF image", grubefi);
            goto done;
        }
    }

    /* Search for the "/EFI/" string in this file */
    {
        const UINT8* p = data;
        const UINT8* end = data + size;

        while (p != end)
        {
            UINTN rem = end - p;
            const char needle[] = "/EFI/";
            const UINT8* start = p;

            /* If not enough bytes remaining */
            if (rem < sizeof(needle))
                break;

            if (memcmp(p, "/EFI/", sizeof(needle) - 1) == 0)
            {
                p += sizeof(needle) - 1;

                while (*p >= ' ' && *p <= '~')
                    p++;

                if (*p == '\0')
                {
                    grubefidir = (char*)start;
                    /* Keep searching to count duplicates */
                    count++;
                }
            }
            else
            {
                p++;
            }
        }
    }

    if (count > 1)
    {
        fprintf(stderr, "%s: failed to determine EFI directory", argv[0]);
        goto done;
    }

    if (!grubefidir)
    {
        grubefidir = "/EFI/boot";
    }

    printf("%s\n", grubefidir);

    status = 0;

done:

    if (data)
        free(data);

    return status;
}

static int _subst_command(
    int argc, 
    const char** argv)
{
    int status = 1;
    UINT8* data;
    size_t size;
    const char* path;
    const char* oldstr;
    const char* newstr;

    /* Check the arguments */
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s FILENAME OLDSTR NEWSTR\n", argv[0]);
        goto done;
    }

    /* Collect arguments */
    path = argv[1];
    oldstr = argv[2];
    newstr = argv[3];

    /* New and old string must be same length */
    if (strlen(oldstr) != strlen(newstr))
    {
        fprintf(stderr, "%s: strings must be same length\n", argv[0]);
        goto done;
    }

    /* Load the master key */
    if (LoadFile(path, 1, &data, &size) != 0)
    {
        fprintf(stderr, "%s: failed to read: %s\n", argv[0], path);
        goto done;
    }

    /* Find the old string and replace with the new string */
    {
        void* p = Memstr(data, size, oldstr, strlen(oldstr));

        if (!p)
        {
            fprintf(stderr, "%s: string not found", argv[0]);
            goto done;
        }

        memcpy(p, newstr, strlen(newstr));
    }

    /* Rewrite the file */
    if (PutFile(path, data, size) != 0)
    {
        fprintf(stderr, "%s: failed to write: %s\n", argv[0], path);
        goto done;
    }

    status = 0;

done:

    if (data)
        free(data);

    return status;
}

static int _hexhash_command(
    int argc, 
    const char** argv)
{
    unsigned char binary[16*1024];
    SHA1Str sha1Str;
    SHA256Str sha256Str;
    UINT32 binarySize;
    SHA1Hash sha1;
    SHA256Hash sha256;
    size_t len;

    /* Check the arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s HEXSTR\n", argv[0]);
        return 1;
    }

    /* Convert to binary */
    len = strlen(argv[1]);
    memset(binary, 0, sizeof(binary));
    TPM2X_HexStrToBinary(argv[1], len, binary, &binarySize);

    ComputeSHA1(binary, binarySize, &sha1);
    ComputeSHA256(binary, binarySize, &sha256);

    sha1Str = SHA1ToStr(&sha1);
    printf("%s\n", sha1Str.buf);

    sha256Str = SHA256ToStr(&sha256);
    printf("%s\n", sha256Str.buf);

    return 0;
}

static int _varhash_command(
    int argc, 
    const char** argv)
{
    int rc;
    static EFI_GUID guid;
    SHA1Hash sha1;
    SHA256Hash sha256;
    SHA1Str sha1Str;
    SHA256Str sha256Str;
    BYTE* binary_data;
    UINT32 binary_data_size;
    char* data = NULL;

    /* Check the arguments */
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s GUID VARIABLE DATA\n", argv[0]);
        return 1;
    }

    if (StrToGUID(argv[1], &guid) != 0)
    {
        fprintf(stderr, "%s: StrToGUID() failed\n", argv[0]);
        return 1;
    }

    /* First see if it's a file */
    if (!(data = _LoadHexStr(argv[3])))
    {
        /* Now treat it as a string */
        data = (char*)argv[3];
    }

    if (!(binary_data = HexStrToBinary(data, &binary_data_size)))
    {
        fprintf(stderr, "%s: HexStrToBinary() failed\n", argv[0]);
        return 1;
    }

    rc = HashVariable(
        &guid, 
        argv[2], 
        binary_data, 
        binary_data_size, 
        &sha1,
        &sha256);

    if (rc != 0)
    {
        fprintf(stderr, "%s: HashVariable() failed\n", argv[0]);
        return 1;
    }

    sha1Str = SHA1ToStr(&sha1);
    printf("%s\n", sha1Str.buf);

    sha256Str = SHA256ToStr(&sha256);
    printf("%s\n", sha256Str.buf);

    if (data != argv[3])
    {
        free(data);
    }

    return 0;
}

static int _efivar_command(
    int argc, 
    const char** argv)
{
    int status = 1;
    unsigned char* data = NULL;
    UINTN size;
    char path[PATH_MAX];
    EFI_GUID guid;
    char tmp[GUID_STR_SIZE];

    /* Check the arguments */
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s EFIVARGUID EFIVARNAME\n", argv[0]);
        goto done;
    }

    /* Convert string to guid */
    if (StrToGUID(argv[1], &guid) != 0)
    {
        fprintf(stderr, "%s: invalid GUID: %s\n", argv[0], argv[1]);
        goto done;
    }

    /* Load the EFI variable into memory */
    if (LoadEFIVar(argv[1], argv[2], &data, &size) != 0)
    {
        fprintf(stderr, "%s: failed to load variable\n", argv[0]);
        goto done;
    }

    /* Form output file name */
    {
        GUIDToStr(&guid, tmp);
        Strlcpy(path, tmp, sizeof(path));
        //Strtoupper(path);
        Strlcat(path, "-", sizeof(path));
        Strlcat(path, argv[2], sizeof(path));
    }

    /* Write the file to disk */
    if (PutFile(path, data, size) != 0)
    {
        fprintf(stderr, "%s: failed to write file: %s\n", argv[0], path);
        goto done;
    }

    status = 0;

done:

    if (data)
        free(data);

    return status;
}

static int _dbxupdate_command(
    int argc, 
    const char** argv)
{
    int status = 1;
    const char* dbxupdatefile;
    unsigned char* data = NULL;
    size_t size;

    /* Check the arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s DBXUPDATEFILE\n", argv[0]);
        goto done;
    }

    /* Colect arguments */
    dbxupdatefile = argv[1];

    /* Load the input file */
    if (LoadFile(dbxupdatefile, 0, &data, &size) != 0)
    {
        fprintf(stderr, "%s: failed to load file: %s\n", argv[0], 
            dbxupdatefile);
        goto done;
    }

    /* Load the EFI variable into memory */
    if (ApplyDBXUpdate(data, size) != 0)
    {
        fprintf(stderr, "%s: failed to apply DBX update\n", argv[0]);
        goto done;
    }

    status = 0;

done:

    if (data)
        free(data);

    return status;
}

static int _hextobin_command(
    int argc, 
    const char** argv)
{
    int status = 0;
    BYTE* data = NULL;
    UINT32 size;
    const char* hexstr;
    const char* outfile;
    UINT32 hexstrlen;
    FILE* os = NULL;

    /* Check arguments */
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s HEXSTR OUTFILE\n", argv[0]);
        status = 1;
        goto done;
    }

    /* Collect arguments */
    hexstr = argv[1];
    hexstrlen = strlen(hexstr);
    outfile = argv[2];

    /* Hex string must be multiple of two */
    if (hexstrlen % 2)
    {
        fprintf(stderr, "%s: malformed hex string\n", argv[0]);
        status = 1;
        goto done;
    }

    /* Allocate buffer to hold binary data */
    if (!(data = malloc(size = hexstrlen / 2)))
    {
        fprintf(stderr, "%s: malloc failed\n", argv[0]);
        status = 1;
        goto done;
    }

    /* Convert to binary */
    if (TPM2X_HexStrToBinary(hexstr, hexstrlen, data, &size) != 0)
    {
        fprintf(stderr, "%s: malformed hex string\n", argv[0]);
        status = 1;
        goto done;
    }

    /* Write the file */
    {
        if (!(os = fopen(outfile, "wb")))
        {
            fprintf(stderr, "%s: failed to open '%s'\n", argv[0], outfile);
            status = 1;
            goto done;
        }

        if (fwrite(data, 1, size, os) != size)
        {
            fprintf(stderr, "%s: failed to write '%s'\n", argv[0], outfile);
            status = 1;
            goto done;
        }
    }

done:

    if (data)
        free(data);

    if (os)
        fclose(os);

    return status;
}

#if 0
static int _findfile_command(
    int argc, 
    const char** argv)
{
    int status = 0;
    char path[PATH_MAX];
    int rc;

    path[0] = '\0';

    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s root prefix suffix\n", argv[0]);
        return 1;
    }

    rc = _FindFile(argv[1], argv[2], argv[3], path);

    if (rc == -1)
    {
        fprintf(stderr, "%s: _FindFile() failed\n", argv[0]);
        return 1;
    }
    else if (rc == 1)
    {
        fprintf(stderr, "%s: not found\n", argv[0]);
        return 1;
    }
    else if (rc == 0)
    {
        fprintf(stderr, "%s: found{%s}\n", argv[0], path);
        return 1;
    }

    return status;
}
#endif

static int _tests_command(
    int argc, 
    const char** argv)
{
    EFI_TCG2_PROTOCOL* protocol = _GetTPMProtocol();
    _TestIsTPMPresent(protocol);
    _TestGetTPMRevision(protocol);
#if !defined(_WIN32)
    _TestSetDictionaryAttackLockReset(protocol);
    _TestSetLockoutParams(protocol);
#endif
    _TestCreateSRKKey(protocol);
    _TestStartAuthSession(protocol);
    return 0;
}

static int _strtohex_command(
    int argc, 
    const char** argv)
{
    char* hexstr = NULL;
    const char* str;
    size_t len;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s STRING\n", argv[0]);
        return 1;
    }

    str = argv[1];
    len = strlen(str);

    if (!(hexstr = (char*)malloc(2 * len + 1)))
    {
        fprintf(stderr, "%s: malloc() failed\n", argv[0]);
        return 1;
    }

    TPM2X_BinaryToHexStr((BYTE*)argv[1], len, hexstr);
    hexstr[len*2] = '\0';
    printf("%s\n", hexstr);

    free(hexstr);

    return 0;
}

static int _tpmrev_command(
    int argc, 
    const char** argv)
{
    EFI_TCG2_PROTOCOL* protocol = _GetTPMProtocol();
    UINT32 revision;
    TPM_RC rc;

    if (argc != 1)
    {
        fprintf(stderr, "Usage %s\n", argv[0]);
        return 1;
    }

    if ((rc = TPM2X_GetTPMRevision(
        protocol,
        &revision)) != TPM_RC_SUCCESS)
    {
        fprintf(stderr, "TPM2_Utils_GetTPMRevision() failed: %08X\n", rc);
        return 1;
    }

    printf("%u(0x%08X)\n", revision, revision);
    return 0;
}

static int _tpmcap_command(
    int argc, 
    const char** argv)
{
    EFI_TCG2_PROTOCOL* protocol = _GetTPMProtocol();
    EFI_TCG2_BOOT_SERVICE_CAPABILITY capability;

    if (argc != 1)
    {
        fprintf(stderr, "Usage %s\n", argv[0]);
        return 1;
    }

    if (TCG2_GetCapability(protocol, &capability) != TPM_RC_SUCCESS)
    {
        fprintf(stderr, "%s: TCG2_GetCapability() failed\n", argv[0]);
        return 1;
    }

    DumpTCG2Capability(&capability);

    return 0;
}

static int _cencode_command(
    int argc, 
    const char** argv)
{
    int status = 1;
    unsigned char* data = NULL;
    size_t size;
    size_t i;
    const char* name = "file";
    BOOLEAN zeroterminate = FALSE;

    /* Extract the --nameoption (if any) */
    if ((status = GetOpt(&argc, argv, "--name", &name)) < 0)
    {
        fprintf(stderr, "%s: missing option argument: --name\n", argv[0]);
        goto done;
    }

    /* Extract the --zeroterminate */
    if (GetOpt(&argc, argv, "--zeroterminate", NULL) == 1)
    {
        zeroterminate = TRUE;
    }

    /* Check arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage %s [--name NAME, --zeroterminate] PATH\n", 
            argv[0]);
        goto done;
    }

    /* Read the file into memory */
    if (LoadFile(argv[1], 1, &data, &size) != 0)
    {
        fprintf(stderr, "%s: failed to read file: '%s'\n", argv[0], argv[1]);
        goto done;
    }

    if (zeroterminate)
        data[size++] = '\0';

    /* Print the array definitions */
    printf("/* %s */\n", argv[1]);
    printf("unsigned char %s[] =\n", name);
    printf("{");

    for (i = 0; i < size; i++)
    {
        if ((i) % 8 == 0)
            printf("\n    ");

        printf("0x%02X, ", data[i]);
    }

    printf("\n};\n");

    /* Print the size of the array */
    printf("unsigned int %s_size = sizeof(%s) / sizeof(%s[0]);\n",
        name, name, name);

    status = 0;

done:

    if (data)
        free(data);

    return status;
}

static int _confCallback(
    const char* name, 
    const char* value,
    void* callbackData,
    Error* err)
{
    printf("{%s}={%s}\n", name, value);
    return 0;
}

static int _conf_command(
    int argc, 
    const char** argv)
{
    unsigned char* data;
    size_t size;
    unsigned int errorLine = 0;
    Error err;

    if (argc != 2)
    {
        fprintf(stderr, "Usage %s PATH\n", argv[0]);
        return 1;
    }

    /* Read the file into memory */
    if (LoadFile(argv[1], 1, &data, &size) != 0)
    {
        fprintf(stderr, "%s: failed to read file: '%s'\n", argv[0], argv[1]);
        return 1;
    }

    /* Parse the configuration file */
    if (ParseConf(
        (const char*)data, 
        size, 
        _confCallback,
        NULL,
        &errorLine, 
        &err) != 0)
    {
        fprintf(stderr, "%s: parse failed: %s(%u): %s\n", argv[0], argv[1],
            (int)errorLine, err.buf);
        return 1;
    }

    return 0;
}

static int _numpcrs_command(
    int argc, 
    const char** argv)
{
    EFI_TCG2_PROTOCOL* protocol = _GetTPMProtocol();
    TPM_RC rc;
    UINT32 pcrCount = 0;

    if ((rc = TPM2X_GetPCRCount(
        protocol, 
        &pcrCount)) != TPM_RC_SUCCESS)
    {
        fprintf(stderr, "TPM2_Utils_GetTPMRevision() failed: %08X\n", rc);
        return 1;
    }

    printf("%u\n", pcrCount);

    return 0;
}

void PutWcs(const CHAR16* s)
{
    UINTN i;

    printf("{");

    for (i = 0; s[i]; i++)
    {
        printf("%c", s[i]);
    }

    printf("}\n");
}

static int _CheckKernel(
    const char* path,
    Error* err)
{
    int rc = -1;
    unsigned char* data = NULL;
    setup_header_t sh;
    size_t size;

    ClearErr(err);

    if (!path)
    {
        SetErr(err, "null path parameter");
        goto done;
    }

    /* Load kernel into memory */
    if (LoadFile(path, 0, &data, &size) != 0)
    {
        SetErr(err, "cannot read '%s'", path);
        goto done;
    }

    /* Extract the setup header */
    Memcpy(&sh, data + SETUP_OFFSET, sizeof(sh));

    /* Check the boot flag magic number */
    if (sh.boot_flag != BOOT_FLAG)
    {
        SetErr(err, "bad setup_header.boot_ flag value");
        goto done;
        goto done;
    }

    /* Check whether too many setup sections */
    if (sh.setup_sects > MAX_SETUP_SECTS)
    {
        SetErr(err, "too many sectup sectors");
        goto done;
    }

    /* Reject old versions of kernel (< 2.11) */
    if (sh.version < MINIMUM_SUPPORTED_VERSION)
    {
        SetErr(err, "kernel version is too old");
        goto done;
    }

    /* If kernel does not have an EFI handover offset */
    if (sh.handover_offset == 0)
    {
        SetErr(err, "kernel has no EFI handover entry point");
        goto done;
    }

    rc = 0;

done:

    if (data)
        free(data);

    return rc;
}

static int _seal_command(
    int argc, 
    const char* argv[])
{
    EFI_TCG2_PROTOCOL* protocol = _GetTPMProtocol();
    int status = -1;
    TPM_RC rc;
    TPM_HANDLE srk;
    BOOLEAN nullsrk = TRUE;
    BYTE* data = NULL;
    size_t size = 0;
    TPM2X_BLOB blob;
    BYTE blobBuffer[sizeof(blob)];
    UINT32 blobBufferSize;
    Error err;
    UINT32 pcrMask = 0;

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s INFILE OUTFILE\n", argv[0]);
        goto done;
    }

    if ((rc = TPM2X_CreateSRKKey(protocol, &srk)) != TPM_RC_SUCCESS)
    {
        char buf[TPM2X_CODETOSTR_BUFSIZE];
        const char* errstr = TPM2X_CodeToStr(buf, rc);
        fprintf(stderr, "%s: TPM2X_CreateSRKKey() failed: %X: %s\n", 
            argv[0], rc, errstr);
        goto done;
    }

    nullsrk = FALSE;

    /* Load the file into memory */
    if (LoadFile(argv[1], 0, (unsigned char**)&data, &size) != 0)
    {
        fprintf(stderr, "%s: failed to load file: %s\n", argv[0], argv[1]);
        goto done;
    }

    /* Maybe add --pcr option to select these */
    pcrMask |= (1 << 7);
    pcrMask |= (1 << 11);

    if ((rc = TPM2X_Seal(
        protocol,
        srk,
        FALSE,
        pcrMask,
        NULL, /* pcrs */
        data,
        size,
        &blob,
        &err)) != TPM_RC_SUCCESS)
    {
        fprintf(stderr, "%s: TPM2X_Seal() failed\n", argv[0]);
        goto done;
    }

    if (TPM2X_SerializeBlob(&blob, blobBuffer, sizeof(blob), &blobBufferSize) != 0)
    {
        fprintf(stderr, "%s: TPM2X_Serialize() failed\n", argv[0]);
        goto done;
    }


    /* Write new file to disk */
    if (PutFile(argv[2], blobBuffer, blobBufferSize) != 0)
    {
        fprintf(stderr, "%s: failed to write file: %s\n", argv[0], argv[2]);
        goto done;
    }

    status = 0;

done:

    if (!nullsrk)
        TPM2_FlushContext(protocol, srk);

    return status;
}

static int _unseal_command(
    int argc, 
    const char* argv[])
{
    EFI_TCG2_PROTOCOL* protocol = _GetTPMProtocol();
    int status = -1;
    TPM_RC rc;
    TPM_HANDLE srk;
    BOOLEAN nullsrk = TRUE;
    BYTE* blobBuffer = NULL;
    size_t blobSize = 0;
    BYTE data[TPM2X_MAX_DATA_SIZE];
    UINT16 dataSize = 0;
    Error err;
    UINT32 pcrMask = 0;
    BOOLEAN cap = FALSE;
    TPM2X_BLOB blob;

    /* Extract the optional --cap flag */
    if (GetOpt(&argc, argv, "--cap", NULL) == 1)
        cap = TRUE;

    /* Check parameters */
    if (argc != 2 && argc != 3)
    {
        fprintf(stderr, "Usage: %s [--cap] INFILE [OUTFILE]\n", argv[0]);
        goto done;
    }

    /* Create the storage root key */
    {
        if ((rc = TPM2X_CreateSRKKey(protocol, &srk)) != TPM_RC_SUCCESS)
        {
            char buf[TPM2X_CODETOSTR_BUFSIZE];
            const char* errstr = TPM2X_CodeToStr(buf, rc);
            fprintf(stderr, "%s: TPM2X_CreateSRKKey() failed: %X: %s\n", 
                argv[0], rc, errstr);
            goto done;
        }

        nullsrk = FALSE;
    }

    /* Load the file into memory */
    {
        if (LoadFile(argv[1], 0, (unsigned char**)&blobBuffer, &blobSize) != 0)
        {
            fprintf(stderr, "%s: failed to load file: %s\n", argv[0], argv[1]);
            goto done;
        }
    }

    /* Maybe add --pcr option to control these masks */
    pcrMask |= (1 << 7);
    pcrMask |= (1 << 11);


    /* Deserialize the blob. */
    if (TPM2X_DeserializeBlob(blobBuffer, blobSize, &blob) != 0)
    {
        fprintf(stderr, "%s: failed to deserialize blob.\n", argv[0]);
        goto done;
    }

    /* Unseal the data */
    if ((rc = TPM2X_Unseal(
        protocol,
        pcrMask,
        srk,
        &blob,
        data,
        &dataSize,
        &err)) != TPM_RC_SUCCESS)
    {
        fprintf(stderr, "%s: TPM2X_Unseal() failed: %s\n", argv[0], err.buf);
        goto done;
    }

    if (argc == 3)
    {
        /* Write new file to disk */
        if (PutFile(argv[2], data, dataSize) != 0)
        {
            fprintf(stderr, "%s: failed to write file: %s\n", argv[0], argv[2]);
            goto done;
        }
    }
    else
    {
        /* Write the standard output */
        fwrite(data, 1, dataSize, stdout);
    }

    status = 0;

done:

    if (cap || status != 0)
    {
        if (TPM2X_Cap(protocol, 11) != TPM_RC_SUCCESS)
        {
            fprintf(stderr, "%s: failed to cap PCR[11]\n", argv[0]);
            rc = -1;
        }
    }

    if (!nullsrk)
        TPM2_FlushContext(protocol, srk);

    return status;
}

static int _dumpblob_command(
    int argc, 
    const char* argv[])
{
    int status = -1;
    BYTE* data = NULL;
    size_t size = 0;

    /* Check parameters */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s INFILE\n", argv[0]);
        goto done;
    }

    /* Load the file into memory */
    if (LoadFile(argv[1], 0, (unsigned char**)&data, &size) != 0)
    {
        fprintf(stderr, "%s: failed to load file: %s\n", argv[0], argv[1]);
        goto done;
    }

    if (TPM2X_DumpBlob(data, size) != TPM_RC_SUCCESS)
    {
        fprintf(stderr, "%s: failed to dump blob\n", argv[0]);
        goto done;
    }

    status = 0;

done:

    if (!data)
        Free(data);

    return status;
}

static int _srk_command(
    int argc, 
    const char* argv[])
{
    EFI_TCG2_PROTOCOL* protocol = _GetTPMProtocol();
    int status = -1;
    TPM_RC rc;
    TPM_HANDLE srk;
    BOOLEAN nullsrk = TRUE;

    if (argc != 1)
    {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        goto done;
    }

    if ((rc = TPM2X_CreateSRKKey(protocol, &srk)) != TPM_RC_SUCCESS)
    {
        char buf[TPM2X_CODETOSTR_BUFSIZE];
        const char* errstr = TPM2X_CodeToStr(buf, rc);
        fprintf(stderr, "%s: TPM2X_CreateSRKKey() failed: %X: %s\n", 
            argv[0], rc, errstr);
        goto done;
    }

    nullsrk = FALSE;

done:

    if (!nullsrk)
        TPM2_FlushContext(protocol, srk);

    return status;
}

static int _strtok_command(
    int argc, 
    const char* argv[])
{
    int status = 1;
    char buf[128];
    char* p;
    char* save;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s STRING\n", argv[0]);
        goto done;
    }

    Strlcpy(buf, argv[1], sizeof(buf));

    for (p = Strtok(buf, "/", &save); p; p = Strtok(NULL, "/", &save))
    {
        printf("%s\n", p);
    }

    status = 0;

done:

    return status;
}

static int _chkkernel_command(
    int argc, 
    const char* argv[])
{
    int status = 1;
    Error err;

    /* Check arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s KERNELPATH\n", argv[0]);
        goto done;
    }

    /* Check the kernel */
    if (_CheckKernel(argv[1], &err) != 0)
    {
        fprintf(stderr, "%s: invalid kernel image: %s: %s\n", 
            argv[0], argv[1], err.buf);
        goto done;
    }

    status = 0;

done:

    return status;
}

static int _policy_command(
    int argc, 
    const char* argv[])
{
    int status = 1;
    Error err;
    Policy policy;

    /* Check arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s SEALFILEPATH\n", argv[0]);
        goto done;
    }

    /* Load the seal file into memory */
    if (LoadPolicy(argv[1], &policy, &err) != 0)
    {
        fprintf(stderr, "%s: failed to load: %s: %s\n", 
            argv[0], argv[1], err.buf);
        goto done;
    }

    /* Dump the file */
    DumpPolicy(&policy);

    status = 0;

done:

    return status;
}

static int _measurepolicy_command(
    int argc, 
    const char* argv[])
{
    EFI_TCG2_PROTOCOL* protocol = _GetTPMProtocol();
    int status = 1;
    Error err;
    Policy policy;
    UINT32 i;
    BOOLEAN log = FALSE;
    PCRBanks banks;

    /* Extract the --log option */
    if (GetOpt(&argc, argv, "--log", NULL) == 1)
        log = TRUE;

    /* Check arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s SEALFILEPATH\n", argv[0]);
        goto done;
    }

    /* Load the seal file into memory */
    if (LoadPolicy(argv[1], &policy, &err) != 0)
    {
        fprintf(stderr, "%s: failed to load: %s: %s\n", 
            argv[0], argv[1], err.buf);
        goto done;
    }

    /* Measure the entries in the seal file */
    if (MeasurePolicy(NULL, protocol, &policy, log, &banks, &err) != 0)
    {
        fprintf(stderr, "%s: MeasurePolicy() failed: %s\n", argv[0], err.buf);
        goto done;
    }

    /* Dump the PCR-256 bank */
    for (i = 0; i < MAX_PCRS; i++)
    {
        if (banks.pcrMask & (1 << i))
        {
            printf("PCR[%02u]:SHA1=", i);
            DumpSHA1(&banks.pcrs1[i]);

            printf("PCR[%02u]:SHA256=", i);
            DumpSHA256(&banks.pcrs256[i]);
        }
    }

    status = 0;

done:

    return status;
}

static int _sealpolicy_command(
    int argc, 
    const char* argv[])
{
    EFI_TCG2_PROTOCOL* protocol = _GetTPMProtocol();
    int status = 1;
    Error err;
    Policy policy;
    int nargs = 0;
    const char* arg0;
    const char* policyfile;
    const char* infile;
    const char* outfile;
    unsigned char* data = NULL;
    size_t size = 0;
    BOOLEAN log = FALSE;
    BYTE blob[TPM2X_BLOB_SIZE];
    UINTN blobSize;

    /* Extract the --log option */
    if (GetOpt(&argc, argv, "--log", NULL) == 1)
        log = TRUE;

    /* Check arguments */
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s SEALFILEPATH INFILE OUTFILE\n", argv[0]);
        goto done;
    }

    /* Collect arguments */
    arg0 = argv[nargs++];
    policyfile = argv[nargs++];
    infile = argv[nargs++];
    outfile = argv[nargs++];

    /* Load the seal file into memory */
    if (LoadPolicy(policyfile, &policy, &err) != 0)
    {
        fprintf(stderr, "%s: failed to load: %s: %s\n", arg0, policyfile,
            err.buf);
        goto done;
    }

    /* Load the file into memory */
    if (LoadFile(infile, 0, (unsigned char**)&data, &size) != 0)
    {
        fprintf(stderr, "%s: failed to load file: %s\n", arg0, infile);
        goto done;
    }

    /* Measure the entries in the seal file */
    if (SealPolicy(
        NULL,
        protocol, 
        &policy, 
        log, 
        data, 
        size, 
        blob, 
        &blobSize, &err) != 0)
    {
        fprintf(stderr, "%s: SealPolicy() failed: %s\n", arg0, err.buf);
        goto done;
    }

    /* Write the file */
    if (PutFile(outfile, blob, blobSize) != 0)
    {
        fprintf(stderr, "%s: cannot write: %s\n", argv[0], outfile);
        goto done;
    }

    status = 0;

done:

    if (data)
        free(data);

    return status;
}

static int _seallsvmloadpolicy_command(
    int argc, 
    const char* argv[])
{
    EFI_TCG2_PROTOCOL* protocol = _GetTPMProtocol();
    int status = 1;
    Error err;
    int nargs = 0;
    const char* arg0;
    const char* lsvmload;
    const char* infile;
    const char* outfile;
    unsigned char* infileData = NULL;
    size_t infileSize = 0;
    unsigned char* lsvmloadData = NULL;
    size_t lsvmloadSize = 0;
    BOOLEAN secureboot = FALSE;
    BYTE outfileData[TPM2X_BLOB_SIZE];
    UINTN outfileSize;

    /* Extract the --log option */
    if (GetOpt(&argc, argv, "--secureboot", NULL) == 1)
        secureboot = TRUE;

    /* Check arguments */
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s LSVMLOAD INFILE OUTFILE\n", argv[0]);
        goto done;
    }

    /* Collect arguments */
    arg0 = argv[nargs++];
    lsvmload = argv[nargs++];
    infile = argv[nargs++];
    outfile = argv[nargs++];

    /* Load the LSVMLOAD executable file */
    if (LoadFile(lsvmload, 0, (unsigned char**)&lsvmloadData, &lsvmloadSize)!=0)
    {
        fprintf(stderr, "%s: cannot load %s\n", arg0, lsvmload);
        goto done;
    }

    /* Load the input file into memory */
    if (LoadFile(infile, 0, (unsigned char**)&infileData, &infileSize) != 0)
    {
        fprintf(stderr, "%s: cannot load %s\n", arg0, infile);
        goto done;
    }

    if (SealLSVMLoadPolicy(
        protocol, 
        lsvmloadData,
        lsvmloadSize,
        secureboot,
        infileData,
        infileSize,
        outfileData,
        &outfileSize,
        &err) != 0)
    {
        fprintf(stderr, "%s: SealPolicy() failed: %s\n", arg0, err.buf);
        goto done;
    }

    /* Write the file */
    if (PutFile(outfile, outfileData, outfileSize) != 0)
    {
        fprintf(stderr, "%s: cannot write: %s\n", argv[0], outfile);
        goto done;
    }

    status = 0;

done:

    if (infileData)
        free(infileData);

    return status;
}

static int _imagetype_command(
    int argc, 
    const char* argv[])
{
    int status = 1;
    unsigned char* data = NULL;
    size_t size;

    /* Check arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s IMAGEFILENAME\n", argv[0]);
        goto done;
    }

    /* Load the file into memory */
    if (LoadFile(argv[1], 0, (unsigned char**)&data, &size) != 0)
    {
        goto done;
    }

    if (IsKernelImage(data, size))
    {
        printf("bzimage\n");
    }
    else if (IsEFIImage(data, size))
    {
        printf("efi\n");
    }
    else
    {
        printf("unknown\n");
    }

    status = 0;

done:

    if (data)
        free(data);

    return status;
}

#if defined(ENABLE_LUKS)
static int _luks_command(
    int argc, 
    const char* argv[])
{
    int status = 1;
    const char* luksfs;
    Blkdev* rawdev = NULL;
    LUKSHeader header;
    UINT8 *masterKey = NULL;
    UINT8 mkDigest[LUKS_DIGEST_SIZE];
    const char* mkfile = NULL;
    const char* keyfile;
    UINT8* passphrase = NULL;
    size_t passphraseSize = 0;
    BOOLEAN dump = FALSE;

    /* Extract the --dump */
    if (GetOpt(&argc, argv, "--dump", NULL) == 1)
        dump = TRUE;

    /* Extract the --mkfile option (if any) */
    if (GetOpt(&argc, argv, "--mkfile", &mkfile) < 0)
    {
        fprintf(stderr, "%s: missing option argument: --keyfile\n", argv[0]);
        goto done;
    }

    /* Check arguments */
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s [ --mkfile OUTFILE ] LUKSFS KEYFILE\n", 
            argv[0]);
        goto done;
    }

    /* Collect arguments */
    luksfs = argv[1];
    keyfile = argv[2];

    /* Open the raw block device */
    if (!(rawdev = BlkdevOpen(luksfs, BLKDEV_ACCESS_RDWR, 0)))
    {
        fprintf(stderr, "%s: failed to open: %s\n", argv[0], luksfs);
        status = 1;
        goto done;
    }

    /* Read the LUKS header */
    if (LUKSReadHeader(rawdev, &header) != 0)
    {
        fprintf(stderr, "%s: failed to read LUKS header: %s\n", argv[0], 
            luksfs);
        goto done;
    }

    /* Dump the header */
    if (dump)
        LUKSDumpHeader(&header);

    /* Load the key file (allocate an extra byte) */
    {
        if (LoadFile(keyfile, 1, &passphrase, &passphraseSize) != 0)
        {
            fprintf(stderr, "%s: failed to read: %s\n", argv[0], keyfile);
            goto done;
        }

        passphrase[passphraseSize] = '\0';
    }

    /* Get the master key */
    {
        if (!(masterKey = (UINT8*)malloc(header.key_bytes)))
            goto done;

        if (LUKSGetMasterKey(
            rawdev, 
            &header,
            passphrase,
            passphraseSize,
            masterKey) != 0)
        {
            fprintf(stderr, "%s: failed to get master key", argv[0]);
            goto done;
        }

        if (dump)
        {
            printf("=== Master key:");
            HexDump(masterKey, header.key_bytes);
        }
    }

    /* Compute the digest */
    if (LUKSComputeMKDigest(&header, masterKey, mkDigest) != 0)
    {
        fprintf(stderr, "%s: failed to compute MK digest", argv[0]);
        goto done;
    }

    if (dump)
    {
        printf("=== MK digest:");
        HexDump(mkDigest, sizeof(mkDigest));
    }

    if (mkfile)
    {
        /* Standard output? */
        if (strcmp(mkfile, "-") == 0)
        {
            if (fwrite(
                masterKey, 
                1,
                header.key_bytes, 
                stdout) != header.key_bytes)
            {
                fprintf(stderr, "%s: failed to write mater key", argv[0]);
                goto done;
            }
        }
        else 
        {
            if (PutFile(mkfile, masterKey, header.key_bytes) != 0)
            {
                fprintf(stderr, "%s: failed to write mater key file", argv[0]);
                goto done;
            }
        }
    }

    status = 0;

done:

    if (masterKey)
        free(masterKey);

    return status;
}
#endif /* defined(ENABLE_LUKS) */

#if defined(ENABLE_LUKS)
static int _luksmk_command(
    int argc, 
    const char* argv[])
{
    int status = 1;
    const char* luksfs;
    const char* ppfile;
    const char* mkfile;
    Blkdev* rawdev = NULL;
    LUKSHeader header;
    UINT8 *masterKey = NULL;
    UINT8 *passphrase = NULL;
    size_t passphraseSize = 0;

    /* Check arguments */
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s LUKSFS PPINFILE MKOUTFILE\n", argv[0]);
        goto done;
    }

    /* Collect arguments */
    luksfs = argv[1];
    ppfile = argv[2];
    mkfile = argv[3];

    /* Open the raw block device */
    if (!(rawdev = BlkdevOpen(luksfs, BLKDEV_ACCESS_RDWR, 0)))
    {
        fprintf(stderr, "%s: failed to open: %s\n", argv[0], luksfs);
        goto done;
    }

    /* Read the LUKS header */
    if (LUKSReadHeader(rawdev, &header) != 0)
    {
        fprintf(stderr, "%s: failed to read LUKS header: %s\n", argv[0], 
            luksfs);
        goto done;
    }

    /* Load the passphrase file (allocate an extra byte) */
    {
        if (LoadFile(ppfile, 1, &passphrase, &passphraseSize) != 0)
        {
            fprintf(stderr, "%s: failed to read: %s\n", argv[0], ppfile);
            goto done;
        }

        passphrase[passphraseSize] = '\0';
    }

    /* Get the master key */
    {
        if (!(masterKey = (UINT8*)malloc(header.key_bytes)))
            goto done;

        if (LUKSGetMasterKey(
            rawdev, 
            &header,
            passphrase,
            passphraseSize,
            masterKey) != 0)
        {
            fprintf(stderr, "%s: failed to get master key", argv[0]);
            goto done;
        }
    }

    /* Standard output? */
    if (strcmp(mkfile, "-") == 0)
    {
        if (fwrite(
            masterKey, 
            1,
            header.key_bytes, 
            stdout) != header.key_bytes)
        {
            fprintf(stderr, "%s: failed to write mater key", argv[0]);
            goto done;
        }
    }
    else 
    {
        if (PutFile(mkfile, masterKey, header.key_bytes) != 0)
        {
            fprintf(stderr, "%s: failed to write mater key file", argv[0]);
            goto done;
        }
    }

    status = 0;

done:

    if (passphrase)
        Free(passphrase);

    if (masterKey)
        free(masterKey);

    return status;
}
#endif /* defined(ENABLE_LUKS) */

static int _newpart_command(
    int argc, 
    const char** argv)
{
    int status = 1;
    GPT gpt;

    /* Check the arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s DEVICE\n", argv[0]);
        goto done;
    }

    /* Read the GPT */
    if (LoadGPT(argv[1], &gpt) != 0)
    {
        fprintf(stderr, "%s: failed load GTP: %s\n", argv[0], argv[1]);
        goto done;
    }

    /* Count GPT entries */
    printf("%u\n", (unsigned int)CountGPTEntries(&gpt) + 1);

    status = 0;

done:

    return status;
}

#if defined(ENABLE_BINS_COMMAND)
static int _bins_command(
    int argc, 
    const char** argv)
{
    int status = 1;
    const char* path = ".";

    /* Check the arguments */
    if (argc != 1 && argc != 2)
    {
        fprintf(stderr, "Usage: %s [PATH]\n", argv[0]);
        goto done;
    }

    /* If path argument present */
    if (argc == 2)
        path = argv[1];

    /* Find all binary files */
    if (FindBins(path) != 0)
    {
        fprintf(stderr, "%s: unexpected error\n", argv[0]);
        goto done;
    }

    status = 0;

done:

    return status;
}
#endif /* defined(ENABLE_BINS_COMMAND) */

#if defined(__linux__)
static int _parsedb_command(
    int argc, 
    const char** argv)
{
    BOOLEAN quiet;
    const char* dbFile;
    const char* binaryFile;
    int rc = -1;


    if (argc != 3 && argc != 4)
    {
        fprintf(stderr, "Usage: %s [--quiet] DBFILE BINARY\n", argv[0]);
        return -1;
    }
    
    /* Collect arguments. */
    quiet = GetOpt(&argc, argv, "--quiet", NULL) == 1;
    dbFile = argv[1];
    binaryFile = argv[2];    
    
    {
        UINT8* database = NULL;
        size_t databaseSize;
        UINT8* binary = NULL;
        size_t binarySize;
        UINT8* result = NULL;
        UINTN resultSize;
        Error err;

        ClearErr(&err);

        if (LoadFile(dbFile, 0, &database, &databaseSize))
            goto end;

        if (LoadFile(binaryFile, 0, &binary, &binarySize))
            goto end;
        
        if (CheckImageDB(
                database,
                databaseSize,
                binary,
                binarySize,
                &result,
                &resultSize,
                quiet))
        {
            goto end;
        }

        if (result != NULL && resultSize != 0)
        {
            if (!quiet)
            {
                printf("MATCH FOUND\n");
                HexDump(result, resultSize);
            }
            if (fwrite(result, 1, resultSize, stdout) != resultSize)
                goto end;
        }
        else if (!quiet)
        {
            printf("NO MATCH FOUND\n");
        }
        
        rc = 0;

end:
        if (database)
            free(database);
        if (binary)
            free(binary);
        if (result)
            free(result);
    }
    return rc;
}
#endif /* defined(__linux__) */

static int _dumpdbx_command(
    int argc, 
    const char** argv)
{
    int rc = 1;
    const char* dbxfile = NULL;
    unsigned char* data = NULL;
    size_t size;
    UINTN i;

    /* Check arguments */
    if (argc != 1 && argc != 2)
    {
        fprintf(stderr, "Usage: %s [DBXFILE]\n", argv[0]);
        goto done;
    }
    
    /* Collect arguments. */
    if (argc == 2)
        dbxfile = argv[1];

    if (dbxfile)
    {
        /* Load DBXFILE into memory */
        if (LoadFile(dbxfile, 0, &data, &size) != 0)
        {
            fprintf(stderr, "%s: failed to load: %s\n", argv[0], dbxfile);
            goto done;
        }
    }
    else
    {
        const char guidstr[] = "d719b2cb-3d3a-4596-a3bc-dad00e67656f";
        const char name[] = "dbx";

        UINTN tmp;

        if (LoadEFIVar(guidstr, name, &data, &tmp) != 0)
        {
            fprintf(stderr, "%s: failed to load dbx variable\n", argv[0]);
            goto done;
        }

        size = (size_t)tmp;
    }

    SHA256Hash* hashesData = NULL;
    UINTN hashesSize = 0;

    if (LoadDBXHashes(data, size, &hashesData, &hashesSize) != 0)
    {
        fprintf(stderr, "%s: failed to load hashes\n", argv[0]);
        goto done;
    }

    for (i = 0; i < hashesSize; i++)
    {
        SHA256Str str = SHA256ToStr(&hashesData[i]);
        printf("%s\n", str.buf);
    }

    rc = 0;

done:

    if (data) 
        free(data);

    if (hashesData)
        free(hashesData);

    return rc;
}

static int _needdbxupdate_command(
    int argc, 
    const char** argv)
{
    int rc = 1;
    const char* dbxfile = NULL;
    BOOLEAN need = FALSE;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s DBXFILE\n", argv[0]);
        goto done;
    }

    dbxfile = argv[1];

    if (NeedDBXUpdate(dbxfile, &need) != 0)
    {
        fprintf(stderr, "%s: failed to check update need", argv[0]);
        goto done;
    }

    if (need)
        printf("yes\n");
    else
        printf("no\n");
    
    rc = 0;

done:

    return rc;
}

static int _stripdbx_command(
    int argc, 
    const char** argv)
{
    int rc = 1;
    const char* dbxinfile;
    const char* dbxoutfile;
    unsigned char* data = NULL;
    size_t size;

    /* Check usage */
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s DBXINFILE DBXOUTFILE\n", argv[0]);
        goto done;
    }

    /* Collect arguments */
    dbxinfile = argv[1];
    dbxoutfile = argv[2];

    /* Load the file into memroy */
    if (LoadFile(dbxinfile, 0, &data, &size) != 0)
    {
        fprintf(stderr, "%s: cannot load: %s\n", argv[0], dbxinfile);
        goto done;
    }

    /* Validate file by trying to load the hashses */
    {
        SHA256Hash* hashesData;
        UINTN hashesSize;

        if (LoadDBXHashes(data, size, &hashesData, &hashesSize) != 0)
        {
            fprintf(stderr, "%s: invalid DBX file: %s\n", argv[0], dbxinfile);
            goto done;
        }

        free(hashesData);
    }

    /* Remove optional header and write new file */
    {
        UINTN n = size;
        const void* p = SkipOptionalDBXHeader(data, &n);

        if (PutFile(dbxoutfile, p, n) != 0)
        {
            fprintf(stderr, "%s: failed to write: %s\n", argv[0], dbxoutfile);
            goto done;
        }
    }

    rc = 0;

done:

    if (data)
        free(data);

    return rc;
}

static int _revoked_command(
    int argc, 
    const char** argv)
{
    int rc = 1;
    const char* bootloader = NULL;
    const char* dbxfile = NULL;
    SHA256Hash hash;
    unsigned char* data = NULL;
    size_t size;
    SHA256Hash* hashesData = NULL;
    UINTN hashesSize = 0;
    UINTN i;

    /* Check arguments */
    if (argc != 2 && argc != 3)
    {
        fprintf(stderr, "Usage: %s BOOTLOADER [DBXFILE]\n", argv[0]);
        goto done;
    }
    
    /* Collect arguments. */
    {
        bootloader = argv[1];

        if (argc == 3)
            dbxfile = argv[2];
    }

    /* Compute the PE-Hash of the boot loader */
    if (_GetPEHash(bootloader, &hash) != 0)
    {
        fprintf(stderr, "%s: failed to calculate hash\n", argv[0]);
        return 1;
    }

    /* Load the optional DBXFILE into memory */
    if (dbxfile)
    {
        if (LoadFile(dbxfile, 0, &data, &size) != 0)
        {
            fprintf(stderr, "%s: failed to load: %s\n", argv[0], dbxfile);
            goto done;
        }
    }
    else 
    {
        const char guidstr[] = "d719b2cb-3d3a-4596-a3bc-dad00e67656f";
        const char name[] = "dbx";

        UINTN tmp;

        if (LoadEFIVar(guidstr, name, &data, &tmp) != 0)
        {
            fprintf(stderr, "%s: failed to load dbx variable\n", argv[0]);
            goto done;
        }

        size = (size_t)tmp;
    }

    /* Load the hashes from the DBX file or variable into memory */
    if (LoadDBXHashes(data, size, &hashesData, &hashesSize) != 0)
    {
        fprintf(stderr, "%s: failed to load hashes\n", argv[0]);
        goto done;
    }

#if 0
    /* Append hash for testing only */
    AppendElem((void**)&hashesData, &hashesSize, sizeof(hash), &hash);
#endif

    /* Search for this hash in the hashes list */
    for (i = 0; i < hashesSize; i++)
    {
        if (memcmp(&hashesData[i], &hash, sizeof(hash)) == 0)
        {
            rc = 0;
            printf("yes\n");
            goto done;
        }
    }

    printf("no\n");

    rc = 0;

done:

    if (data) 
        free(data);

    if (hashesData)
        free(hashesData);

    return rc;
}

static int _partuuid_command(
    int argc, 
    const char** argv)
{
    const char* partition;
    int status= 1;
    Blkdev* dev = NULL;
    EXT2* ext2 = NULL;
    LUKSHeader luksHeader;

    /* Check arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s PARTITION\n", argv[0]);
        goto done;
    }

    /* Collect arguments */
    partition = argv[1];

    /* Open device */
    if (!(dev = BlkdevOpen(partition, BLKDEV_ACCESS_RDWR, 0)))
    {
        fprintf(stderr, "%s: failed to open: %s\n", argv[0], partition);
        goto done;
    }

    /* Try to open device as EXT2 file system */
    if (EXT2New(dev, &ext2) == EXT2_ERR_NONE)
    {
        EFI_GUID guid;
        char buf[GUID_STRING_SIZE];
        MakeGUIDFromBytes(&guid, ext2->sb.s_uuid);
        FormatGUID(buf, &guid);
        printf("%s\n", buf);
    }
    else if (LUKSReadHeader(dev, &luksHeader) == 0)
    {
        printf("%s\n", luksHeader.uuid);
    }
    else
    {
        fprintf(stderr, "%s: unable to resolve parition UUID\n", argv[0]);
        goto done;
    }

    status = 0;
    
done:

    if (dev)
        dev->Close(dev);

    return status;
}

static int _keyscpio_command(
    int argc, 
    const char** argv)
{
    int status = 1;
    const char* cpiopath;
    const char* bootkey;
    const char* rootkey;
    unsigned char* bootkeyData = NULL;
    size_t bootkeySize;
    unsigned char* rootkeyData = NULL;
    size_t rootkeySize;
    void* cpioData = NULL;
    UINTN cpioSize;

    /* Check arguments */
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s CPIOPATH BOOTKEY ROOTKEY\n", argv[0]);
        goto done;
    }

    /* Collect arguments */
    cpiopath = argv[1];
    bootkey = argv[2];
    rootkey = argv[3];

    /* Load bootkey into memory */
    if (LoadFile(bootkey, 0, &bootkeyData, &bootkeySize) != 0)
    {
        fprintf(stderr, "%s: failed to load %s\n", argv[0], bootkey);
        goto done;
    }

    /* Load rootkey into memory */
    if (LoadFile(rootkey, 0, &rootkeyData, &rootkeySize) != 0)
    {
        fprintf(stderr, "%s: failed to load %s\n", argv[0], rootkey);
        goto done;
    }

    /* Create the new archive (in memroy) */
    if (InitrdMakeArchive(
        bootkeyData,
        bootkeySize,
        rootkeyData,
        rootkeySize,
        &cpioData,
        &cpioSize) != 0)
    {
        fprintf(stderr, "%s: failed to create archive\n", argv[0]);
        goto done;
    }

    /* Create archive file */
    if (PutFile(cpiopath, cpioData, cpioSize) != 0)
    {
        fprintf(stderr, "%s: failed to create %s\n", argv[0], cpiopath);
        goto done;
    }

    status = 0;
    
done:

    if (bootkeyData)
        Free(bootkeyData);

    if (rootkeyData)
        Free(rootkeyData);

    if (cpioData)
        Free(cpioData);

    return status;
}

#if defined(__linux__)
static int _Exec(int argc, const char* argv[])
{
    pid_t pid;
    int rc = 1;

    /* Create a child to execute argv[0] */
    if ((pid = fork()) < 0)
    {
        fprintf(stderr, "%s: fork() failed\n", arg0);
        goto done;
    }

    if (pid > 0) /* Parent */
    {
        int status;

        /* Wait for child to finish */
        if (wait(&status) != pid)
        {
            fprintf(stderr, "%s: wait() failed\n", arg0);
            status = 1;
            goto done;
        }

        /* If status is bad */
        if (WEXITSTATUS(status) != 0)
        {
            rc = WEXITSTATUS(status);
            goto done;
        }
    }
    else if (pid == 0) /* Child */
    {
        execv(argv[0], (char**)argv);
        /* Means argv[0] not found */
        fprintf(stderr, "%s: failed to exec %s\n", arg0, argv[0]);
        exit(1);
    }

    rc = 0;

done:
    return rc;
}
#endif /* defined(__linux__) */

#if defined(__linux__)
static int _askpass_command(
    int argc, 
    const char** argv)
{
    int status = 1;
    const char* crypttarget;
    const char* keyfile = NULL;
    unsigned char* data = NULL;
    size_t size;

    /* Check the arguments */
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s %s ASKPASSPROGRAM ARGS...\n", arg0, argv[0]);
        goto done;
    }

    /* Get the "crypttarget" environment variable */
    if (!(crypttarget = Dupenv("crypttarget")))
    {
        fprintf(stderr, "%s %s: crypttarget environment variable undefined", 
            arg0, argv[0]);
        goto manual;
    }

    /* Resolve the keyfile path */
    if (Strcmp(crypttarget, "boot") == 0)
        keyfile = "/etc/lsvmload/bootkey";
    else
        keyfile = "/etc/lsvmload/rootkey";

    /* Load the key file into memory */
    if (LoadFile(keyfile, 0, (unsigned char**)&data, &size) != 0)
    {
        fprintf(stderr, "%s %s: failed to load %s", arg0, argv[0], keyfile);
        goto manual;
    }

    /* Write the key to standard output */
    fwrite(data, 1, size, stdout);

    status = 0;
    goto done;

manual:

#if 0
    {
        int i;

        for (i = 1; i < argc; i++)
        {
            fprintf(stderr, "ARG{%s}\n", argv[i]);
        }
        sleep(4);
    }
#endif

    /* Obtain password manually */
    if (_Exec(argc - 1, argv + 1) != 0)
    {
        fprintf(stderr, "%s: failed to get password\n", argv[1]);
        goto done;
    }

    status = 0;

done:

    return status;
}
#endif /* defined(__linux__) */

#if defined(__linux__)
static int _inject_command(
    int argc, 
    const char** argv)
{
    int status = 1;
    const char* infile;
    const char* bootkey;
    const char* rootkey;
    const char* outfile;
    unsigned char* infileData = NULL;
    size_t infileSize;
    unsigned char* bootkeyData = NULL;
    size_t bootkeySize;
    unsigned char* rootkeyData = NULL;
    size_t rootkeySize;
    void* outfileData = NULL;
    UINTN outfileSize = 0;

    /* Check the arguments */
    if (argc < 5)
    {
        fprintf(stderr, "Usage: %s INFILE BOOTKEY ROOTKEY OUTFILE\n", argv[0]);
        goto done;
    }

    /* Collect arguments */
    infile = argv[1];
    bootkey = argv[2];
    rootkey = argv[3];
    outfile = argv[4];

    /* Load the infile */
    if (LoadFile(infile, 0, &infileData, &infileSize) != 0)
    {
        fprintf(stderr, "%s: failed to load: %s\n", argv[0], infile);
        goto done;
    }

    /* Load the bootkey */
    if (LoadFile(bootkey, 0, &bootkeyData, &bootkeySize) != 0)
    {
        fprintf(stderr, "%s: failed to load: %s\n", argv[0], bootkey);
        goto done;
    }

    /* Load the rootkey */
    if (LoadFile(rootkey, 0, &rootkeyData, &rootkeySize) != 0)
    {
        fprintf(stderr, "%s: failed to load: %s\n", argv[0], rootkey);
        goto done;
    }

    /* Inject the keys into the input file */
    if (InitrdInjectFiles(
        infileData,
        infileSize,
        bootkeyData,
        bootkeySize,
        rootkeyData,
        rootkeySize,
        &outfileData,
        &outfileSize) != 0)
    {
        fprintf(stderr, "%s: failed to inject keys\n", argv[0]);
        goto done;
    }

    /* Write the output file */
    if (PutFile(outfile, outfileData, outfileSize) != 0)
    {
        fprintf(stderr, "%s: failed to create %s\n", argv[0], outfile);
        goto done;
    }

    status = 0;

done:

    if (infileData)
        Free(infileData);

    if (bootkeyData)
        Free(bootkeyData);

    if (rootkeyData)
        Free(rootkeyData);

    if (outfileData)
        Free(outfileData);

    return status;
}
#endif /* defined(__linux__) */

#if defined(__linux__)
static int _cryptpw_command(
    int argc, 
    const char** argv)
{
    int status = 1;
    const char* password;
    char buf[1024];

    /* Check the arguments */
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s PASSWORD\n", argv[0]);
        goto done;
    }

    /* Collect arguments */
    password = argv[1];

    /* Encrypt the password */
    if (CryptPassword(buf, sizeof(buf), password) != 0)
    {
        fprintf(stderr, "%s: failed to encrypt the password\n", argv[0]);
        goto done;
    }

    printf("%s\n", buf);

    status = 0;

done:

    return status;
}
#endif /* defined(__linux__) */

typedef int (*CompressFunction)(
    const unsigned char* inData,
    unsigned long inSize,
    unsigned char** outData,
    unsigned long* outSize);

static int _compress(
    int argc, 
    const char** argv,
    CompressFunction compress)
{
    int status = 1;
    unsigned char* dataIn = NULL;
    size_t sizeIn;
    unsigned char* dataOut = NULL;
    size_t sizeOut;
    const char* infile;
    const char* outfile;

    /* Check the arguments */
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s INFILE OUTFILE\n", argv[0]);
        goto done;
    }

    /* Collect arguments */
    infile = argv[1];
    outfile = argv[2];

    /* Load the master key */
    if (LoadFile(infile, 1, &dataIn, &sizeIn) != 0)
    {
        fprintf(stderr, "%s: failed to read: %s\n", argv[0], infile);
        goto done;
    }

    /* Compress the file */
    if (compress(dataIn, sizeIn, &dataOut, &sizeOut) != 0)
    {
        fprintf(stderr, "%s: lzma_compress_file() failed\n", argv[0]);
        goto done;
    }

    /* Rewrite the file */
    if (PutFile(outfile, dataOut, sizeOut) != 0)
    {
        fprintf(stderr, "%s: failed to write: %s\n", argv[0], outfile);
        goto done;
    }

    status = 0;

done:

    if (dataIn)
        free(dataIn);

    if (dataOut)
        free(dataOut);

    return status;
}

typedef int (*DecompressFunction)(
    const unsigned char* inData,
    unsigned long inSize,
    unsigned char** outData,
    unsigned long* outSize);

typedef int (*TestFunction)(
    const unsigned char* data,
    unsigned long size);

static int _decompress(
    int argc, 
    const char** argv,
    DecompressFunction decompress,
    TestFunction test)
{
    int status = 1;
    unsigned char* dataIn = NULL;
    size_t sizeIn;
    unsigned char* dataOut = NULL;
    size_t sizeOut;
    const char* infile;
    const char* outfile;

    /* Check the arguments */
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s INFILE OUTFILE\n", argv[0]);
        goto done;
    }

    /* Collect arguments */
    infile = argv[1];
    outfile = argv[2];

    /* Load the master key */
    if (LoadFile(infile, 1, &dataIn, &sizeIn) != 0)
    {
        fprintf(stderr, "%s: failed to read: %s\n", argv[0], infile);
        goto done;
    }

    /* Verify that it's a .xz file */
    if (test(dataIn, sizeIn) != 0)
    {
        fprintf(stderr, "%s: lzmaextras_test() failed\n", argv[0]);
        goto done;
    }

    /* Compress the file */
    if (decompress(dataIn, sizeIn, &dataOut, &sizeOut) != 0)
    {
        fprintf(stderr, "%s: lzma_compress_file() failed\n", argv[0]);
        goto done;
    }

    /* Rewrite the file */
    if (PutFile(outfile, dataOut, sizeOut) != 0)
    {
        fprintf(stderr, "%s: failed to write: %s\n", argv[0], outfile);
        goto done;
    }

    status = 0;

done:

    if (dataIn)
        free(dataIn);

    if (dataOut)
        free(dataOut);

    return status;
}

static int _xz_command(
    int argc, 
    const char** argv)
{
    return _compress(argc, argv, lzmaextras_compress);
}

static int _unxz_command(
    int argc, 
    const char** argv)
{
    return _decompress(argc, argv, lzmaextras_decompress,
        lzmaextras_test);
}

static int _gzip_command(
    int argc, 
    const char** argv)
{
    return _compress(argc, argv, zlibextras_compress);
}

static int _gunzip_command(
    int argc, 
    const char** argv)
{
    return _decompress(argc, argv, zlibextras_decompress,
        zlibextras_test);
}

static int _testxz_command(
    int argc, 
    const char** argv)
{
    int status = 1;
    unsigned char* data = NULL;
    size_t size;
    const char* infile;

    /* Check the arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s INFILE\n", argv[0]);
        goto done;
    }

    /* Collect arguments */
    infile = argv[1];

    /* Load the master key */
    if (LoadFile(infile, 1, &data, &size) != 0)
    {
        fprintf(stderr, "%s: failed to read: %s\n", argv[0], infile);
        goto done;
    }

    /* Compress the file */
    if (lzmaextras_test_compress_decompress(data, size) != 0)
    {
        fprintf(stderr, "%s: lzmaextras_test_compress_decompress() failed\n", 
            argv[0]);
        goto done;
    }

    status = 0;

done:

    if (data)
        free(data);

    return status;
}

static int _testzlib_command(
    int argc, 
    const char** argv)
{
    int status = 1;
    unsigned char* data = NULL;
    size_t size;
    const char* infile;

    /* Check the arguments */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s INFILE\n", argv[0]);
        goto done;
    }

    /* Collect arguments */
    infile = argv[1];

    /* Load the master key */
    if (LoadFile(infile, 1, &data, &size) != 0)
    {
        fprintf(stderr, "%s: failed to read: %s\n", argv[0], infile);
        goto done;
    }

    if (zlibextras_test_compress_decompress(data, size) != 0)
    {
        fprintf(stderr, "%s: zlibextras_test_compress_decompress() failed\n", 
            argv[0]);
        goto done;
    }

    
    status = 0;

done:

    if (data)
        free(data);

    return status;
}


static int _serializekeys_command(
    int argc,
    const char** argv)
{
    int status = 1;
    const char* bootkey;
    const char* rootkey;
    const char* outfile;
    unsigned char* dataBoot = NULL;
    unsigned char* dataRoot = NULL;
    unsigned char* outData = NULL;
    size_t dataBootSize;
    size_t dataRootSize;
    UINTN outDataSize;

    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s BOOTKEY ROOTKEY OUTFILE\n", argv[0]);
        goto done;
    }

    bootkey = argv[1];
    rootkey = argv[2];
    outfile = argv[3];

    if (LoadFile(bootkey, 1, &dataBoot, &dataBootSize) != 0)
    {
        fprintf(stderr, "%s: failed to read: %s\n", argv[0], bootkey);
        goto done;
    }

    if (LoadFile(rootkey, 1, &dataRoot, &dataRootSize) != 0)
    {
        fprintf(stderr, "%s: failed to read: %s\n", argv[0], bootkey);
        goto done;
    }

    if (CombineKeys(
        dataBoot,
        dataRoot,
        (UINTN) dataBootSize,
        (UINTN) dataRootSize,
        &outData,
        &outDataSize) != 0)
    {
        fprintf(stderr, "%s: failed to combine keys\n", argv[0]);
        goto done;
    }

    if (PutFile(outfile, outData, outDataSize) != 0)
    {
        fprintf(stderr, "%s: failed to write file: %s\n", argv[0], outfile);
        goto done;
    }
  
    status = 0;  
done:
    if (dataBoot != NULL)
    {
        free(dataBoot);
    }
    if (dataRoot != NULL)
    {
        free(dataRoot);
    }
    if (outData != NULL)
    {
        free(outData);
    }
    return status;
}

static int _deserializekeys_command(
    int argc,
    const char** argv)
{
    int status = 1;
    const char* bootkey;
    const char* rootkey;
    const char* infile;
    unsigned char* dataBoot = NULL;
    unsigned char* dataRoot = NULL;
    unsigned char* inData = NULL;
    UINTN dataBootSize;
    UINTN dataRootSize;
    size_t inDataSize;

    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s INFILE BOOTKEY ROOTKEY\n", argv[0]);
        goto done;
    }

    infile = argv[1];
    bootkey = argv[2];
    rootkey = argv[3];

    if (LoadFile(infile, 1, &inData, &inDataSize) != 0)
    {
        fprintf(stderr, "%s: failed to read: %s\n", argv[0], infile);
        goto done;
    }

    if (SplitKeys(
        inData,
        (UINTN) inDataSize,
        &dataBoot,
        &dataRoot,
        &dataBootSize,
        &dataRootSize) != 0)
    {
        fprintf(stderr, "%s: failed to split keys\n", argv[0]);
        goto done;
    }

    if (PutFile(bootkey, dataBoot, (size_t) dataBootSize) != 0)
    {
        fprintf(stderr, "%s: failed to write file: %s\n", argv[0], bootkey);
        goto done;
    }

    if (PutFile(rootkey, dataRoot, (size_t) dataRootSize) != 0)
    {
        fprintf(stderr, "%s: failed to write file: %s\n", argv[0], rootkey);
        goto done;
    }
  
    status = 0;

done:
    if (dataBoot != NULL)
    {
        free(dataBoot);
    }
    if (dataRoot != NULL)
    {
        free(dataRoot);
    }
    if (inData != NULL)
    {
        free(inData);
    }
    return status;
}

static int _serialize_specfile_command(
    int argc,
    const char **argv)
{ 
    const char* indir;
    DIR* dir = NULL;
    struct dirent *files = NULL;
    UINT32 numfiles = 0;
    SPECIALIZATION_FILE* specFiles = NULL;
    UINT32 numSpecFiles = 0;
    size_t pathLen;
    const char* outfile;
    UINT8* outData = NULL;
    UINT32 outDataSize;
    int status = -1;
    char path[PATH_MAX];

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s INDIR OUTFILE\n", argv[0]);
        goto done;
    }

    indir = argv[1];
    outfile = argv[2];
    pathLen = strlen(indir);
    
    if (pathLen + 1 + NAME_MAX + 1 > sizeof(path))
    {
        fprintf(stderr, "%s: directory path too long: %s\n", argv[0], indir);
        return -1;
    }

    /* Get the number of files, so we can allocate. */
    dir = opendir(indir);
    if (dir == NULL)
    {
        fprintf(stderr, "%s: Failed to open dir: %s\n", argv[0], indir);
        return -1;
    }

    while ((files = readdir(dir)) != NULL)
    {
        numfiles++;
    }

    if (closedir(dir) != 0)
    {
        fprintf(stderr, "%s: failed to close dir: %s\n", argv[0], indir);
        return -1;
    }
    dir = NULL;

    /* Now actually get the files. */ 
    specFiles = (SPECIALIZATION_FILE*) malloc(numfiles * sizeof(SPECIALIZATION_FILE));
    if (specFiles == NULL)
    {
        fprintf(stderr, "%s: failed to allocate spec files\n", argv[0]);
        goto done;
    }

    dir = opendir(indir);
    if (dir == NULL)
    {
        fprintf(stderr, "%s: Failed to open dir: %s\n", argv[0], indir);
        return -1;
    }

    strncpy(path, indir, pathLen);
    while ((files = readdir(dir)) != NULL)
    {
        struct stat statbuf;       
        SPECIALIZATION_FILE* cur = specFiles + numSpecFiles;
        size_t tmpSize;

        path[pathLen] = '/';
        strncpy(path + pathLen + 1, files->d_name, NAME_MAX);
        path[pathLen + 1 + NAME_MAX] = 0;

        if (lstat(path, &statbuf) != 0)
        {
            fprintf(stderr, "%s: Failed to stat: %s\n", argv[0], path);
            goto done;
        }

        if (!S_ISREG(statbuf.st_mode))
        {
            continue;
        }

        cur->FileName = (char*) malloc(strlen(files->d_name) + 1);
        if (cur->FileName == NULL)
        {
            fprintf(stderr, "%s: Failed to allocate specfile: %s\n", argv[0], path);
            goto done;
        }
        strncpy(cur->FileName, files->d_name, strlen(files->d_name) + 1);


        if (LoadFile(path, 0, &cur->PayloadData, &tmpSize) != 0)
        {
            free(cur->FileName);
            fprintf(stderr, "%s: Failed to load file. %s\n", argv[0], path);
            goto done; 
        }        

        /* Truncate since we don't expect anything > 32 bits. */ 
        cur->PayloadSize = (UINT32) tmpSize;
        numSpecFiles++;
    } 

    if (CombineSpecFiles(
            specFiles,
            numSpecFiles,
            &outData,
            &outDataSize) != 0)
    {
        fprintf(stderr, "%s: failed to combine spec file.\n", argv[0]);
        goto done;
    }

    if (PutFile(outfile, outData, outDataSize) != 0)
    {
        fprintf(stderr, "%s: failed to put outfile: %s\n", argv[0], outfile);
        goto done;
    }

    status = 0;


done:
    if (dir != NULL)
    {
        closedir(dir);
    }

    if (specFiles != NULL)
    {
        FreeSpecFiles(specFiles, numSpecFiles);
    }

    if (outData != NULL)
    {
        free(outData);
    }

    return status;
}

static int _deserialize_specfile_command(
    int argc,
    const char **argv)
{
    int status = 1;
    const char* infile;
    const char* outfile;
    unsigned char* inData = NULL;
    size_t inDataSize;
    SPECIALIZATION_FILE *r = NULL;
    UINTN rSize;
    UINTN i = 0;

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s INFILE OUTDIR\n", argv[0]);
        goto done;
    }

    infile = argv[1];
    outfile = argv[2];

    if (LoadFile(infile, 1, &inData, &inDataSize) != 0)
    {
        fprintf(stderr, "%s: failed to read: %s\n", argv[0], infile);
        goto done;
    }

    if (ExtractSpecFiles(inData, inDataSize, &r, &rSize) != 0)
    {
        fprintf(stderr, "%s: failed to extract spec files.%s\n", argv[0], infile);
        goto done;
    }

    for (i = 0; i < rSize; i++)
    {
        char path[4096] = {0};
        size_t sz = strlen(outfile);
        memcpy(path, outfile, sz);
        path[sz] = '/';
        memcpy(path + sz + 1, r[i].FileName, strlen(r[i].FileName));

        if (PutFile(path, r[i].PayloadData, r[i].PayloadSize) != 0)
        {
            fprintf(stderr, "%s: failed to write file: %s\n", argv[0], outfile);
            goto done;
        } 
    }

    status = 0;

done: 
    if (inData != NULL)
    {
        free(inData);
    }
    
    if (r != NULL)
    {
        FreeSpecFiles(r, rSize);
    }
    return status;
}

/*
**==============================================================================
**
** Command callbacks:
**
**==============================================================================
*/

typedef int (*CommandCallback)(
    int argc, 
    const char* argv[]);

typedef struct _Command
{
    const char* name;
    const char* help;
    CommandCallback callback;
}
Command;

static Command _commands[] =
{
    {
        "tests", 
        "Run various TPM related tests",
        _tests_command,
    },
#if 0
    {
        "findfile", 
        "Finds a file",
        _findfile_command 
    },
#endif
    {
        "hextobin", 
        "Converts a hex string argument to a binary file",
        _hextobin_command,
    },
    {
        "efivar", 
        "Dump the contents of the given EFI varaible",
        _efivar_command,
    },
    {
        "dbxupdate", 
        "Update the DBX variable",
        _dbxupdate_command,
    },
    {
        "varhash", 
        "Compute the hash of the given EFI variable",
        _varhash_command,
    },
    {
        "hexhash", 
        "Compute the hash of the data given by a hex string",
        _hexhash_command,
    },
    {
        "pehash", 
        "Compute PE hash of the given binary image",
        _pehash_command,
    },
    {
        "pehash1", 
        "Compute PE hash of the given binary image",
        _pehash1_command,
    },
    {
        "extend", 
        "Compute the extension of a series of SHA1 hashes",
        _extend_command,
    },
    {
        "pcrs", 
        "Dump the contents of the PCR SHA1 banks",
        _pcrs_command,
    },
    {
        "pcrs256", 
        "Dump the contents of the PCR SHA256 banks",
        _pcrs256_command,
    },
    {
        "sha1", 
        "Compute the SHA1 of a file",
        _sha1_command,
    },
    {
        "sha256", 
        "Compute the SHA256 of a file",
        _sha256_command,
    },
    {
        "sha1hexstr", 
        "Compute the SHA1 of a hex string",
        _sha1hexstr_command,
    },
    {
        "sha256hexstr", 
        "Compute the SHA256 of a hex string",
        _sha256hexstr_command,
    },
    {
        "key", 
        "Generate a 256-bit key",
        _key_command,
    },
    {
        "hexdump", 
        "Perform a hex dump of a file",
        _hexdump_command,
    },
    {
        "encrypt", 
        "Encrypt a file",
        _encrypt_command,
    },
    {
        "decrypt", 
        "Decrypt a file",
        _decrypt_command,
    },
    {
        "asciidump", 
        "Perform a ASCII dump of a file",
        _asciidump_command,
    },
    {
        "cappcr11", 
        "Cap PCR[4]",
        _cappcr11_command,
    },
    {
        "capall", 
        "Cap all PCRs",
        _capall_command,
    },
    {
        "strtohex", 
        "Convert a string to a hex string",
        _strtohex_command,
    },
    {
        "tpmrev", 
        "Print out the TPM revision (in decimal and hex)",
        _tpmrev_command,
    },
    {
        "tpmcap", 
        "Print out the TPM capabilities",
        _tpmcap_command,
    },
    {
        "cencode", 
        "Encode binary files as a C-program array",
        _cencode_command,
    },
    {
        "conf", 
        "Dump the contents of a configuration file",
        _conf_command,
    },
    {
        "numpcrs", 
        "Print the number of PCRs",
        _numpcrs_command,
    },
    {
        "seal", 
        "Seal data with respect to PCR[4,7,11]",
        _seal_command,
    },
    {
        "unseal", 
        "Unseal data with respect to PCR[4,7,11]",
        _unseal_command,
    },
    {
        "dumpblob", 
        "Dump a TPM sealed blob",
        _dumpblob_command,
    },
    {
        "srk", 
        "Test creation of a storage root key",
        _srk_command,
    },
    {
        "strtok", 
        "Test strtok",
        _strtok_command,
    },
    {
        "chkkernel", 
        "Check whether kernel has sufficient features",
        _chkkernel_command,
    },
    {
        "policy", 
        "Dump the given .seal file",
        _policy_command,
    },
    {
        "measurepolicy", 
        "Perform measurements from a seal file",
        _measurepolicy_command,
    },
    {
        "sealpolicy", 
        "Perform sealing of the given policy",
        _sealpolicy_command,
    },
    {
        "seallsvmloadpolicy", 
        "Perform sealing of the lsvmload policy",
        _seallsvmloadpolicy_command, 
    },
    {
        "imagetype", 
        "Determine whether boot loader or kernel",
        _imagetype_command,
    },
#if defined(ENABLE_LUKS)
    {
        "luks", 
        "Dump the LUKS image",
        _luks_command,
    },
#endif /* defined(ENABLE_LUKS) */
#if defined(ENABLE_LUKS)
    {
        "luksmk", 
        "Use a LUKS passphrase to retrieve the LUKS masterkey",
        _luksmk_command,
    },
#endif /* defined(ENABLE_LUKS) */
#if defined(HAVE_OPENSSL)
    {
        "peverify", 
        "Verify the certificate of this image",
        _peverify_command,
    },
#endif /* defined(HAVE_OPENSSL) */
    {
        "gpt", 
        "Dump the gpt table on the given device",
        _gpt_command,
    },
#if defined(__linux__)
    {
        "bootdev", 
        "Print out information about the boot device",
        _bootdev_command,
    },
#endif /* defined(__linux__) */
#if defined(__linux__)
    {
        "grubcfginitrd", 
        "Generate the grub.cfg that goes on the ESP",
        _grubcfginitrd_command,
    },
#endif /* defined(__linux__) */
    {
        "grubefidir", 
        "Determine EFI directory to place grub.cfg into from grub.efi",
        _grubefidir_command,
    },
    {
        "subst", 
        "Substitute one string with another in the given file",
        _subst_command,
    },
    {
        "newpart", 
        "Assign the next available partition number from this GPT",
        _newpart_command,
    },
#if defined(ENABLE_BINS_COMMAND)
    {
        "bins", 
        "Find binary files (with non-ASCII characters)",
        _bins_command,
    },
#endif /* defined(ENABLE_BINS_COMMAND) */
#if defined(__linux__)
    {
        "parsedb",
        "Parse a UEFI db variable",
        _parsedb_command,  
    },
    {
        "dumpdbx",
        "Dump the contents of the DBX file",
        _dumpdbx_command,
    },
    {
        "needdbxupdate",
        "Print 'yes' if a DBX update is needed",
        _needdbxupdate_command,
    },
    {
        "stripdbx",
        "Remove the optional auth wrapper around a dbx file",
        _stripdbx_command,
    },
    {
        "revoked",
        "Determine whether this boot loader is revoked",
        _revoked_command,
    },
    {
        "partuuid",
        "Find the UUID for this parition",
        _partuuid_command,  
    },
    {
        "keyscpio",
        "Create CPIO archvie with bootkey and rootkey",
        _keyscpio_command,  
    },
#if 0
    {
        "initrds",
        "List the initds on the boot device",
        _initrds_command,  
    },
#endif
    {
        "askpass",
        "Acquire password (for Ubuntu only)",
        _askpass_command,  
    },
    {
        "inject",
        "Inject keys into initrd",
        _inject_command,  
    },
    {
        "cryptpw",
        "Encrypt a Linux password",
        _cryptpw_command,  
    },
    {
        "gunzip",
        "Unzip a gzipped file",
        _gunzip_command,
    },
    {
        "gzip",
        "Unzip a gzipped file",
        _gzip_command,
    },
    {
        "xz",
        "Compress a file",
        _xz_command,
    },
    {
        "unxz",
        "Compress a file",
        _unxz_command,
    },
    {
        "testxz",
        "Test xz compression/decompression",
        _testxz_command,
    },
    {
        "testzlib",
        "Test zlib compression/decompression",
        _testzlib_command,
    },
#endif /* defined(__linux__) */
    {
        "serializekeys",
        "Serializes the boot+rootkey to be ready for sealing",
        _serializekeys_command,
    },
    {
        "deserializekeys",
        "Deserializes the boot+rootkey from unsealed data",
        _deserializekeys_command,
    },
    {
        "serialize_specfile",
        "Serialize files in a directory to generate a specialization file.",
        _serialize_specfile_command,
    },
    {
        "deserialize_specfile",
        "Deserializes the specialization file",
        _deserialize_specfile_command,
    },
};

static size_t _ncommands = sizeof(_commands) / sizeof(_commands[0]);

/*
**==============================================================================
**
** main()
**
**==============================================================================
*/

static const char USAGE[] = "\
\n\
Usage: %s COMMAND ARGUMENTS...\n\
\n\
SYNOPSIS:\n\
    This utility performs various Linux measured boot tasks.\n\
\n\
COMMANDS:\n\
";

extern unsigned char g_logo[];

extern int vfat_main(int argc, const char* argv[]);
extern int ext2_main(int argc, const char* argv[]);
extern int cpio_main(int argc, const char* argv[]);

/* Called by zliblinux library in case of panic */
void __zlib_panic(const char* func)
{
    fprintf(stderr, "%s: %s\n", __FUNCTION__, func);
    assert(0);
}

int lsvmtool_main(int argc, const char* argv[])
{
    size_t i;
    BOOLEAN logo = FALSE;
    arg0 = argv[0];

    /* Check the --version or -v option */
    if (GetOpt(&argc, argv, "-v", NULL) == 1 ||
        GetOpt(&argc, argv, "--version", NULL) == 1)
    {
        printf("%s\n", LSVMTOOL_VERSION);
        return 0;
    }

    /* Get the --logo option */
    if (GetOpt(&argc, argv, "--logo", NULL) == 1)
        logo = TRUE;

    /* Print logo */
    if (logo)
        fprintf(stderr, "%s\n", (char*)g_logo);

    /* Print usage */
    if (argc < 2)
    {
        fprintf(stderr, USAGE, argv[0]);

        for (i = 0; i < _ncommands; i++)
        {
            fprintf(stderr, "    %s - %s\n", 
                _commands[i].name,
                _commands[i].help);
        }

        fprintf(stderr, "\n");
        return 1;
    }

    /* Find and execute the command given by argv[1] */
    for (i = 0; i < _ncommands; i++)
    {
        if (strcmp(argv[1], _commands[i].name) == 0)
        {
            return ((*_commands[i].callback)(argc-1, argv+1));
        }
    }

    if (strcmp(argv[1], "vfat") == 0)
        return (vfat_main(argc-1, argv+1));
    else if (strcmp(argv[1], "ext2") == 0)
        return (ext2_main(argc-1, argv+1));
    else if (strcmp(argv[1], "cpio") == 0)
        return (cpio_main(argc-1, argv+1));

    fprintf(stderr, "%s: unknown command: '%s'\n", argv[0], argv[1]);
    return 1;

    return 0;
}

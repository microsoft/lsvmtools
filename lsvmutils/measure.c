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

#if !defined(BUILD_EFI)
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <stdlib.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/ioctl.h>
# include <linux/fs.h>
# include <fcntl.h>
# include <limits.h>
# include <stddef.h>
# include <unistd.h>
# include <errno.h>
#endif

#include "measure.h"
#include <lsvmutils/tpm2.h>
#include <lsvmutils/alloc.h>
#include <lsvmutils/eficommon.h>
#include <lsvmutils/peimage.h>
#include <lsvmutils/ext2.h>
#include <lsvmutils/tpmbuf.h>
#include "print.h"
#include "vars.h"

#if !defined(BUILD_EFI)
#include <lsvmutils/file.h>
#endif /* !defined(BUILD_EFI) */

static int _LoadVariableOrFile(
    const Vars* vars,
    const char* path, 
    size_t extraBytes,
    unsigned char** dataOut,
    UINTN* outSize)
{
    /* If this is a variable */
    if (VarsLoad(vars, path, extraBytes, dataOut, outSize) == 0)
        return 0;

#if defined(BUILD_EFI)
    /* Files not implemented in EFI */
    return -1;
#else
    {
        size_t tmp;

        if (LoadFile(path, extraBytes, dataOut, &tmp) != 0)
            return -1;

        *outSize = (UINTN)tmp;
        return 0;
    }
#endif /* defined(BUILD_EFI) */
}

typedef struct _EFI_VARIABLE_DATA_TREE
{
    EFI_GUID VariableName;
    UINT64 UnicodeNameLength;
    UINT64 VariableDataLength;
    CHAR16 UnicodeName[];
    /* Optional variable data follows if VariableDataLength > 0 */
    /* INT8 VariableData[]; */
}
EFI_VARIABLE_DATA_TREE;

static void _Char16Char8Memcpy(CHAR16* s1, const char* s2, size_t size)
{
    while (size--)
        *s1++ = *s2++;
}

void DumpSHA1(const SHA1Hash* sha1)
{
    size_t i;

    for (i = 0; i < sizeof(SHA1Hash); i++)
    {
        PRINTF("%02X", sha1->buf[i]);
    }

    PRINTF0("\n");
}

void DumpSHA256(const SHA256Hash* sha256)
{
    size_t i;

    for (i = 0; i < sizeof(SHA256Hash); i++)
    {
        PRINTF("%02X", sha256->buf[i]);
    }

    PRINTF0("\n");
}

BYTE* HexStrToBinary(
    const char* hexstr,
    UINT32* size)
{
    UINT32 len = strlen(hexstr);
    BYTE* data;

    if (len < 2 || len % 2)
        return NULL;

    if (!(data = Malloc(len / 2)))
        return NULL;

    if (TPM2X_HexStrToBinary(hexstr, len, data, size) != 0)
    {
        Free(data);
        return NULL;
    }

    if (*size != len / 2)
    {
        Free(data);
        return NULL;
    }

    return data;
}

int StrToGUID(
    const char* str,
    EFI_GUID* guid)
{
    int rc = -1;
    UINT32 size;
    BYTE* p = NULL;
    char* hexstr = NULL;

    /* Copy 'str' to 'hexstr', omitting '-' characters */
    {
        char* end;
        
        if (!(hexstr = (char*)Malloc(strlen(str) + 1)))
            goto done;

        /* Strip out any special characters */
        for (end = hexstr; *str; str++)
        {
            if (*str != '-')
                *end++ = *str;
        }

        *end = '\0';
    }

    if (strlen(hexstr) != GUID_STR_SIZE - 1)
        goto done;

    if (!(p = HexStrToBinary(hexstr, &size)))
        goto done;

    if (size != sizeof(EFI_GUID))
        goto done;

    guid->Data1 = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
    guid->Data2 = (p[4] << 8) | p[5];
    guid->Data3 = (p[6] << 8) | p[7];
    guid->Data4[0] = p[8];
    guid->Data4[1] = p[9];
    guid->Data4[2] = p[10];
    guid->Data4[3] = p[11];
    guid->Data4[4] = p[12];
    guid->Data4[5] = p[13];
    guid->Data4[6] = p[14];
    guid->Data4[7] = p[15];

    rc = 0;

done:

    if (p)
        Free(p);

    if (hexstr)
        Free(hexstr);

    return rc;
}

void GUIDToStr(
    EFI_GUID* guid, 
    char str[GUID_STR_SIZE])
{
    EFI_GUID tmp = *guid;
    tmp.Data1 = ByteSwapU32(guid->Data1);
    tmp.Data2 = ByteSwapU16(guid->Data2);
    tmp.Data3 = ByteSwapU16(guid->Data3);
    Memcpy(tmp.Data4, guid->Data4, sizeof(tmp.Data4));

typedef struct {          
    UINT32  Data1;
    UINT16  Data2;
    UINT16  Data3;
    UINT8   Data4[8]; 
} EFI_GUID;

    TPM2X_BinaryToHexStr((BYTE*)&tmp, sizeof(EFI_GUID), str);
    str[GUID_STR_SIZE] = '\0';
}

int LoadEFIVar(
    const char* guidstr,
    const char* name,
    unsigned char** dataOut,
    UINTN* sizeOut)
{
#if defined(BUILD_EFI)

    int rc = -1;
    CHAR16 wcsname[PATH_MAX];
    static EFI_GUID guid;
    EFI_STATUS status;
    const UINTN KILOBYTE = 1024;

    if (dataOut)
        *dataOut = NULL;

    if (sizeOut)
        *sizeOut = 0;

    /* Reject null parameters */
    if (!guidstr || !name || !dataOut || !sizeOut)
        goto done;

    /* Convert variable to wide character */
    WcsStrlcpy(wcsname, name, PATH_MAX);

    /* Convert guidstr string to GUID */
    if (StrToGUID(guidstr, &guid) != 0)
        goto done;

    /* Set initial size of buffer */
    *sizeOut = KILOBYTE;

    /* Allocate memory to hold the variable data */
    if (!(*dataOut = Malloc(*sizeOut)))
        goto done;

    /* Load the EFI variable */
    status = RT->GetVariable(wcsname, &guid, NULL, sizeOut, *dataOut);

    /* If buffer too small, try again with larger buffer */
    if (status == EFI_BUFFER_TOO_SMALL)
    {
        /* Expand buffer to size returned by previous call to GetVariable() */
        if (!(*dataOut = Realloc(*dataOut, KILOBYTE, *sizeOut)))
            goto done;

        status = RT->GetVariable(wcsname, &guid, NULL, sizeOut, *dataOut);
    }

    if (status != EFI_SUCCESS)
        goto done;

    rc = 0;

done:

    if (rc != 0)
    {
        if (dataOut && *dataOut)
            Free(*dataOut);

        *dataOut = NULL;
        *sizeOut = 0;
    }

    return rc;

#else /* defined(BUILD_EFI) */

    int rc = 0;
    static EFI_GUID guid;
    char path[256];
    unsigned char* data = NULL;
    size_t size;

    if (!guidstr || !name)
    {
        rc = -1;
        goto done;
    }

    /* Convert guidstr string to GUID */
    if (StrToGUID(guidstr, &guid) != 0)
    {
        rc = -1;
        goto done;
    }

    /* Format the GUID */
    {
        unsigned int w = guid.Data1;
        unsigned short x = guid.Data2;
        unsigned short y = guid.Data3;
        unsigned char z[8];
        memcpy(z, guid.Data4, sizeof(z));

        sprintf(path, "/sys/firmware/efi/efivars/%s-"
            "%02x%02x%02x%02x-"
            "%02x%02x-"
            "%02x%02x-"
            "%02x%02x-"
            "%02x%02x%02x%02x%02x%02x",
            name,
            (w & 0xFF000000) >> 24,
            (w & 0x00FF0000) >> 16,
            (w & 0x0000FF00) >> 8,
            (w & 0x000000FF),
            (x & 0xFF00) >> 8,
            (x & 0x00FF),
            (y & 0xFF00) >> 8,
            (y & 0x00FF),
            z[0],
            z[1],
            z[2],
            z[3],
            z[4],
            z[5],
            z[6],
            z[7]);
    }

    /* Load the file */
    {
        if (LoadFile(path, 0, &data, &size) != 0)
        {
            rc = -1;
            goto done;
        }

        if (size < 4)
        {
            rc = -1;
            goto done;
        }

        /* Remove the 4-byte header */
        memmove(data, data + 4, size - 4);
        size -= 4;
    }

done:

    if (rc == 0)
    {
        *dataOut = data;
        *sizeOut = size;
    }
    else
    {
        *dataOut = NULL;
        *sizeOut = 0;

        if (data)
            Free(data);
    }

    return rc;

#endif
}

static int _ExtendSHA1(
    IN EFI_TCG2_PROTOCOL *protocol,
    SHA1Hash* sha1,
    const SHA1Hash* data)
{
    typedef struct _pair
    {
        SHA1Hash left;
        SHA1Hash right;
    }
    Pair;
    Pair pair;

    pair.left = *sha1;
    pair.right = *data;

    if (!ComputeSHA1(&pair, sizeof(pair), sha1))
        return -1;

    return 0;
}

static int _ExtendSHA256(
    EFI_TCG2_PROTOCOL *protocol,
    SHA256Hash* sha256,
    const SHA256Hash* data)
{
    typedef struct _pair
    {
        SHA256Hash left;
        SHA256Hash right;
    }
    Pair;
    Pair pair;

    pair.left = *sha256;
    pair.right = *data;

    if (!ComputeSHA256(&pair, sizeof(pair), sha256))
        return -1;

    return 0;
}

#if !defined(BUILD_EFI)
int ExtendRootkey(
    EFI_TCG2_PROTOCOL* protocol,
    IN const char* rootkey_path)
{
    int rc = 0;
    unsigned char* data = NULL;
    UINTN size;
    UINT32 pcr = 11;

    if (rootkey_path == NULL)
    {
        rc = -1;
        goto done;
    }

    /* Measure and extend the entire contents of the boot directory */
    {
        SHA1Hash sha1;
        SHA256Hash sha256;

        if (_LoadVariableOrFile(NULL, rootkey_path, 0, &data, &size) != 0)
        {
            rc = -1;
            goto done;
        }

        if (!ComputeSHA1(data, size, &sha1))
        {
            rc = -1;
            goto done;
        }

        if (!ComputeSHA256(data, size, &sha256))
        {
            rc = -1;
            goto done;
        }

        if (TPM2X_ExtendPCR_SHA1(protocol, pcr, &sha1) != TPM_RC_SUCCESS)
        {
            rc = -1;
            goto done;
        }

        if (TPM2X_ExtendPCR_SHA256(
            protocol, 
            pcr, 
            &sha256) != TPM_RC_SUCCESS)
        {
            rc = -1;
            goto done;
        }

        Free(data);
        data = NULL;
    }

done:

    if (data)
        Free(data);

    return rc;
}
#endif /* !defined(BUILD_EFI) */

int HashVariable(
    IN EFI_GUID *guid,
    IN const char *name,
    IN void *data,
    IN UINTN data_size,
    OUT SHA1Hash* sha1,
    OUT SHA256Hash* sha256)
{
    int rc = -1;
    UINT32 var_data_size;
    UINTN name_len;
    EFI_VARIABLE_DATA_TREE *var_data = NULL;
    SHA1Context sha1_ctx;
    SHA256Context sha256_ctx;

    name_len = strlen(name);

    var_data_size = 
        sizeof(EFI_VARIABLE_DATA_TREE) + 
        (name_len * sizeof (CHAR16)) + 
        data_size;

    var_data = (EFI_VARIABLE_DATA_TREE *)Malloc(var_data_size);

    if (var_data == NULL) 
    {
        goto done;
    }

    var_data->VariableName = *guid;
    var_data->UnicodeNameLength = name_len;
    var_data->VariableDataLength = data_size;

    /* Copy name onto end of structure */
    _Char16Char8Memcpy(var_data->UnicodeName, name, name_len);

    if (data && data_size)
    {
        /* Advance end past the UnicodeName */

        memcpy((CHAR16*)var_data->UnicodeName + name_len, data, data_size);
    }

    if (!SHA1Init(&sha1_ctx)) 
    {
        goto done;
    }

    if (!SHA256Init(&sha256_ctx)) 
    {
        goto done;
    }

    if (!SHA1Update(&sha1_ctx, var_data, var_data_size))
    {
        goto done;
    }

    if (!SHA256Update(&sha256_ctx, var_data, var_data_size))
    {
        goto done;
    }

    if (!SHA1Final(&sha1_ctx, sha1))
    {
        goto done;
    }

    if (!SHA256Final(&sha256_ctx, sha256))
    {
        goto done;
    }

    rc = 0;

done:

    if (var_data)
        Free(var_data);

    return rc;
}

int Measure(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN BOOLEAN predictive,
    IN UINT32 pcr,
    IN const SHA1Hash* sha1,
    IN const SHA256Hash* sha256,
    IN OUT SHA1Hash* pcr_sha1,
    IN OUT SHA256Hash* pcr_sha256)
{
    int rc = -1;

    /* Check parameters */
    if (!protocol || !sha1 || !sha256 || !pcr_sha1 || !pcr_sha256)
    {
        goto done;
    }

    /* Predictively extend the PCR */
    {
        if (_ExtendSHA1(protocol, pcr_sha1, sha1) != 0)
        {
            goto done;
        }

        if (_ExtendSHA256(protocol, pcr_sha256, sha256) != 0)
        {
            goto done;
        }
    }

    /* Extend the PCR */
    if (!predictive)
    {
        if (TPM2X_ExtendPCR_SHA1(protocol, pcr, sha1) != TPM_RC_SUCCESS)
        {
            goto done;
        }

        if (TPM2X_ExtendPCR_SHA256(protocol, pcr, sha256) != TPM_RC_SUCCESS)
        {
            goto done;
        }
    }

    rc = 0;

done:

    return rc;
}

int MeasureSeparator(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN BOOLEAN predictive,
    IN UINT32 pcr,
    IN OUT SHA1Hash* pcr_sha1,
    IN OUT SHA256Hash* pcr_sha256,
    OUT SHA1Hash* sha1,
    OUT SHA256Hash* sha256)
{
    int rc = -1;
    UINT32 separator = 0;

    /* Check parameters */
    if (!protocol || !pcr_sha1 || !pcr_sha256 || !sha1 || !sha256)
    {
        goto done;
    }

    /* Compute the separator hashes */
    {
        if (!ComputeSHA1(&separator, sizeof(separator), sha1))
        {
            goto done;
        }

        if (!ComputeSHA256(&separator, sizeof(separator), sha256))
        {
            goto done;
        }
    }

    /* Perform the measurement */
    if (Measure(
        protocol,
        predictive,
        pcr,
        sha1,
        sha256,
        pcr_sha1,
        pcr_sha256) != 0)
    {
        goto done;
    }

    rc = 0;

done:

    return rc;
}

#if !defined(BUILD_EFI)
int MeasurePEImage(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN BOOLEAN predictive,
    IN UINT32 pcr,
    const char* path,
    IN OUT SHA1Hash* pcr_sha1,
    IN OUT SHA256Hash* pcr_sha256,
    OUT SHA1Hash* sha1,
    OUT SHA256Hash* sha256)
{
    int rc = -1;
    unsigned char* data = NULL;
    UINTN size;

    /* Check parameters */
    if (!protocol || !path || !pcr_sha1 || !pcr_sha256 || !sha1 || !sha256)
    {
        goto done;
    }

    /* Compute the PE-hash of the shim executable */
    {
        if (_LoadVariableOrFile(NULL, path, 0, &data, &size) != 0)
        {
            rc = -1;
            goto done;
        }

        if (ParseAndHashImage(data, size, sha1, sha256) != 0)
        {
            rc = -1;
            goto done;
        }
    }

    /* Perform the measurement */
    if (Measure(
        protocol,
        predictive,
        pcr,
        sha1,
        sha256,
        pcr_sha1,
        pcr_sha256) != 0)
    {
        goto done;
    }

    rc = 0;

done:

    if (data)
        Free(data);

    return rc;
}
#endif /* !defined(BUILD_EFI) */

int MeasureBinaryFile(
    const Vars* vars,
    IN EFI_TCG2_PROTOCOL *protocol,
    IN BOOLEAN predictive,
    IN UINT32 pcr,
    const char* path,
    IN OUT SHA1Hash* pcr_sha1,
    IN OUT SHA256Hash* pcr_sha256,
    OUT SHA1Hash* sha1,
    OUT SHA256Hash* sha256)
{
    int rc = -1;
    unsigned char* data = NULL;
    UINTN size = 0;

    /* Check parameters */
    if (!protocol || !path || !pcr_sha1 || !pcr_sha256 || !sha1 || !sha256)
    {
        goto done;
    }

    /* Compute the hashes */
    {
        if (_LoadVariableOrFile(vars, path, 0, &data, &size) != 0)
        {
            goto done;
        }

        if (!ComputeSHA1(data, size, sha1))
        {
            goto done;
        }

        if (!ComputeSHA256(data, size, sha256))
        {
            goto done;
        }
    }

    /* Perform the measurement */
    if (Measure(
        protocol,
        predictive,
        pcr,
        sha1,
        sha256,
        pcr_sha1,
        pcr_sha256) != 0)
    {
        goto done;
    }

    rc = 0;

done:

    if (data)
        Free(data);

    return rc;
}

int MeasureEFIVariable(
    const Vars* vars,
    IN EFI_TCG2_PROTOCOL *protocol,
    IN BOOLEAN predictive,
    IN UINT32 pcr,
    const char* path,
    IN OUT SHA1Hash* pcr_sha1,
    IN OUT SHA256Hash* pcr_sha256,
    OUT SHA1Hash* sha1,
    OUT SHA256Hash* sha256)
{
    int rc = -1;
    unsigned char* data = NULL;
    UINTN size = 0;
    char buf[PATH_MAX];
    EFI_GUID guid;
    char* name;

    /* Check for null parameters */
    if (!protocol || !path || !pcr_sha1 || !pcr_sha256 || !sha1 || !sha256)
        goto done;

    /* Split path into GUID and name */
    /* Example: 8BE4DF61DF61DF61AA0D00E098032B8C-PK */
    {
        Strlcpy(buf, path, sizeof(buf));

        /* Remove any extension */
        {
            char* p = Strrchr(buf, '.');

            if (p)
                *p = '\0';
        }

        if (!(name = strchr(buf, '-')))
            goto done;

        *name++ = '\0';

        if (StrToGUID(buf, &guid) != 0)
            goto done;
    }

    /* Load the file into memory */
    if (_LoadVariableOrFile(vars, path, 0, &data, &size) != 0)
    {
        /* No such file, so now try loading as EFI variable */
        if (LoadEFIVar(buf, name, &data, &size) != 0)
            goto done;
    }

    /* Hash the variable */
    if (HashVariable(&guid, name, data, size, sha1, sha256) != 0)
        goto done;

    /* Perform the measurement */
    if (Measure(
        protocol,
        predictive,
        pcr,
        sha1,
        sha256,
        pcr_sha1,
        pcr_sha256) != 0)
    {
        goto done;
    }

    rc = 0;

done:

    if (data)
        Free(data);

    return rc;
}

int MeasureCap(
    IN EFI_TCG2_PROTOCOL *protocol,
    IN BOOLEAN predictive,
    IN UINT32 pcr,
    IN OUT SHA1Hash* pcr_sha1,
    IN OUT SHA256Hash* pcr_sha256,
    OUT SHA1Hash* sha1,
    OUT SHA256Hash* sha256)
{
    int rc = -1;
    UINT8 zeros[20];

    /* Check parameters */
    if (!protocol || !pcr_sha1 || !pcr_sha256 || !sha1 || !sha256)
    {
        goto done;
    }

    /* Zero fill the buffer */
    Memset(zeros, 0, sizeof(zeros));

    /* Compute the hashes of this zeros buffer */
    ComputeSHA1(zeros, sizeof(zeros), sha1);
    ComputeSHA256(zeros, sizeof(zeros), sha256);

    /* Perform the measurement */
    if (Measure(
        protocol,
        predictive,
        pcr,
        sha1,
        sha256,
        pcr_sha1,
        pcr_sha256) != 0)
    {
        goto done;
    }

    rc = 0;

done:
    return rc;
}

int MeasureBinaryFilePad32(
    const Vars* vars,
    IN EFI_TCG2_PROTOCOL *protocol,
    IN BOOLEAN predictive,
    IN UINT32 pcr,
    const char* path,
    IN OUT SHA1Hash* pcr_sha1,
    IN OUT SHA256Hash* pcr_sha256,
    OUT SHA1Hash* sha1,
    OUT SHA256Hash* sha256)
{
    int rc = -1;
    unsigned char* data = NULL;
    UINTN size = 0;

    /* Check parameters */
    if (!protocol || !path || !pcr_sha1 || !pcr_sha256 || !sha1 || !sha256)
    {
        goto done;
    }

    /* Compute the hashes */
    {
        if (_LoadVariableOrFile(vars, path, 0, &data, &size) != 0)
        {
            goto done;
        }

        if (!ComputeSHA1Pad32(data, size, sha1))
        {
            goto done;
        }

        if (!ComputeSHA256Pad32(data, size, sha256))
        {
            goto done;
        }
    }

    /* Perform the measurement */
    if (Measure(
        protocol,
        predictive,
        pcr,
        sha1,
        sha256,
        pcr_sha1,
        pcr_sha256) != 0)
    {
        goto done;
    }

    rc = 0;

done:

    if (data)
        Free(data);

    return rc;
}

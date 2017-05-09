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
#include "eficommon.h"
#include "config.h"
#include "sha.h"
#include "strings.h"
#include "alloc.h"
#include "peimage.h"
#include "buf.h"
#include "alloc.h"
#include "tpm2.h"
#include "linux.h"

#if defined(HAVE_OPENSSL)
# include <openssl/objects.h>
# include <openssl/pkcs7.h> 
# include <openssl/objects.h>
# include <openssl/x509.h>
# include <openssl/x509v3.h>
#endif /* defined(HAVE_OPENSSL) */

#if !defined(TRACE)
# define TRACE
#endif

#if defined(BUILD_EFI)
# include "../lsvmload/log.h"
# define LSVMLOAD_TRACE LOGI(L"LSVMLOAD_TRACE: %a(%d)", __FILE__, __LINE__)
#else
# define LSVMLOAD_TRACE
#endif

static int _compare(
    const void *x_,
    const void *y_)
{
    EFI_IMAGE_SECTION_HEADER* x = (EFI_IMAGE_SECTION_HEADER*)x_;
    EFI_IMAGE_SECTION_HEADER* y = (EFI_IMAGE_SECTION_HEADER*)y_;
    return x->PointerToRawData - y->PointerToRawData;
}

static int _AppendRegion(
    Image* image,
    const void* data,
    UINTN size)
{
    int status = -1;
    Region* regions;

    if (!(regions = (Region*)Realloc(
        image->regions,
        sizeof(Region) * image->nregions,
        sizeof(Region) * (image->nregions + 1))))
    {
        goto done;
    }

    regions[image->nregions].data = data;
    regions[image->nregions].size = size;

    image->regions = regions;
    image->nregions++;

    status = 0;

done:
    return status;
}

void ReleaseImage(
    Image* image)
{
    Free(image->regions);
}

static int _ParseSignature(
    Image* image,
    UINTN signatureAddress,
    UINTN signatureSize)
{
    int status = -1;
    typedef struct _Header Header;
    struct _Header
    {
        UINT32 size;
        UINT16 revision;
        UINT16 type;
    }
    PACKED;
    Header* header;

    /* Check parameters */
    if (!image)
        goto done;

    /* If no signature */
    if (signatureSize == 0)
    {
        image->signatureData = NULL;
        image->signatureSize = 0;
        status = 0;
        goto done;
    }

    /* Check the header (must bre PKCS) */
    {
        header = (Header*)((char*)image->imageData + signatureAddress);

        /* PKCS */
        if (header->type != 2)
            goto done;
    }

    /* Resolve pointer and size of signature */
    image->signatureData = (const void*)(header + 1);
    image->signatureSize = header->size - sizeof(Header);

    status = 0;

done:
    return status;
}

int ParseImage(
    Image* image,
    IN const void* imageData,
    IN UINTN imageSize)
{
    int status = 0;
    unsigned int i;
    EFI_IMAGE_SECTION_HEADER *sections = NULL;
    EFI_IMAGE_DOS_HEADER *dos_hdr = (void *)imageData;
    unsigned int pe_hdr_offset;
    EFI_IMAGE_NT_HEADERS64* pe_hdr;
    EFI_IMAGE_DATA_DIRECTORY* sec_dir;

    /* Check parameters */
    if (!imageData || imageSize < sizeof(EFI_IMAGE_DOS_HEADER) || !image)
    {
        status = -1;
        goto done;
    }

    /* Initialize the image */
    Memset(image, 0, sizeof(Image));
    image->imageData = imageData;
    image->imageSize = imageSize;

    /* Check header magic number */
    if (dos_hdr->e_magic != EFI_IMAGE_DOS_SIGNATURE)
    {
        status = -1;
        goto done;
    }

    /* Set a pointer to the PE header */
    pe_hdr_offset = dos_hdr->e_lfanew;
    pe_hdr = (EFI_IMAGE_NT_HEADERS64*)((UINT8*)imageData + pe_hdr_offset);

    /* Region 1: up to (but not including) the checksum */
    {
        const void* p = imageData;
        UINTN n = (char*)&pe_hdr->OptionalHeader.CheckSum - (char*)imageData;

        if (_AppendRegion(image, p, n) != 0)
        {
            status = -1;
            goto done;
        }
    }


    /* Set a pointer to the security directory entry (holds signature) */
    sec_dir = &pe_hdr->OptionalHeader.DataDirectory[
        EFI_IMAGE_DIRECTORY_ENTRY_SECURITY];

    /* Save the digital signature data (do not add to regions list) */
    if (_ParseSignature(image, sec_dir->VirtualAddress, sec_dir->Size) != 0)
    {
        status = -1;
        goto done;
    }

    /* Region 2: from after checksum up to security directory entry */
    {
        const void* p = (char*)&pe_hdr->OptionalHeader.CheckSum + sizeof(int);
        UINTN n = (char*)sec_dir - (char*)p;

        if (_AppendRegion(image, p, n) != 0)
        {
            status = -1;
            goto done;
        }
    }

    /* Region 3: end of certificate table to end of imageData header */
    {
        const void* p = sec_dir + 1;
        UINTN n = pe_hdr->OptionalHeader.SizeOfHeaders - 
            ((char*)p - (char*)imageData);

        if (_AppendRegion(image, p, n) != 0)
        {
            status = -1;
            goto done;
        }
    }

    /* Sort the section headers (in auxilliary storage) */
    {
        int i;
        int j;

        if (!(sections = (EFI_IMAGE_SECTION_HEADER *)Calloc(
            pe_hdr->FileHeader.NumberOfSections,
            sizeof(EFI_IMAGE_SECTION_HEADER))))
        {
            status = -1;
            goto done;
        }

        Memcpy(sections, 
            (char*)imageData + 
            pe_hdr_offset + 
            sizeof(UINT32) +
            sizeof(EFI_IMAGE_FILE_HEADER) + 
            pe_hdr->FileHeader.SizeOfOptionalHeader,
            pe_hdr->FileHeader.NumberOfSections * 
            sizeof(EFI_IMAGE_SECTION_HEADER));

        for (i = 0; i < pe_hdr->FileHeader.NumberOfSections - 1; i++)
        {
            for (j = 0; j < pe_hdr->FileHeader.NumberOfSections - 1; j++)
            {
                if (_compare(&sections[j], &sections[j+1]) > 0)
                {
                    EFI_IMAGE_SECTION_HEADER tmp = sections[j];
                    sections[j] = sections[j+1];
                    sections[j+1] = tmp;
                }
            }
        }
    }

    /* Add a region for each section */
    for (i = 0; i < pe_hdr->FileHeader.NumberOfSections; i++) 
    {
        EFI_IMAGE_SECTION_HEADER *section = &sections[i];

        if (section->SizeOfRawData)
        {
            const void* p = (char*)imageData + section->PointerToRawData;
            UINTN n = section->SizeOfRawData;

            if (_AppendRegion(image, p, n) != 0)
            {
                status = -1;
                goto done;
            }
        }
    }

    /* Add a region for whatever follows the final section */
    {
        EFI_IMAGE_SECTION_HEADER *section = 
            &sections[pe_hdr->FileHeader.NumberOfSections-1];
        unsigned int offset = 
            section->PointerToRawData + section->SizeOfRawData;

        /* Hash data following the sections */
        if (offset < imageSize)
        {
            const void* p = (char*)imageData + offset;
            UINTN n = imageSize - sec_dir->Size - offset;

            if (_AppendRegion(image, p, n) != 0)
            {
                status = -1;
                goto done;
            }
        }
    }

done:

    if (sections)
        Free(sections);

    return status;
}

int HashImage(
    const Image* image,
    OUT SHA1Hash* sha1,
    OUT SHA256Hash* sha256)
{
    int status = -1;
    SHA1Context sha1Ctx;
    SHA256Context sha256Ctx;
    UINTN i;

    /* Check the parameters */
    if (!image || !sha1 || !sha256)
        goto done;

    /* Initialize the hashes */
    {
        if (!SHA1Init(&sha1Ctx))
            goto done;

        if (!SHA256Init(&sha256Ctx))
            goto done;
    }

    for (i = 0; i < image->nregions; i++)
    {
        const Region* region = &image->regions[i];

        if (!SHA1Update(&sha1Ctx, region->data, region->size))
            goto done;

        if (!SHA256Update(&sha256Ctx, region->data, region->size))
            goto done;
    }

    /* Finalize the hashes */
    {
        if (!SHA1Final(&sha1Ctx, sha1))
            goto done;

        if (!SHA256Final(&sha256Ctx, sha256))
            goto done;
    }

    status = 0;

done:

    return status;
}

int ParseAndHashImage(
    const void* imageData,
    UINTN imageSize,
    OUT SHA1Hash* sha1,
    OUT SHA256Hash* sha256)
{
    int status = -1;
    Image image;

    Memset(&image, 0, sizeof(Image));

    if (ParseImage(&image, imageData, imageSize) != 0)
        goto done;

    if (HashImage(&image, sha1, sha256) != 0)
        goto done;

    status = 0;

done:

    ReleaseImage(&image);

    return status;
}

#if defined(HAVE_OPENSSL)
typedef struct _PKCS7WrapHeader
{
    UINT8 byte1;
    UINT8 byte2;
    UINT16 length1;
    UINT8 byte3;
    UINT8 byte4;
    UINT8 oid[9];
    UINT8 byte5;
    UINT8 byte6;
    UINT16 length2;
}
PACKED
PKCS7WrapHeader;
#endif /* defined(HAVE_OPENSSL) */

#if defined(HAVE_OPENSSL)
static PKCS7WrapHeader _header =
{
    0x30,
    0x82,
    0x0000, /* length1 */
    0x06,
    0x09,
    { 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x07, 0x02 },
    0xA0, 
    0x82,
    0x0000 /* length2 */
};
#endif /* defined(HAVE_OPENSSL) */

#if defined(HAVE_OPENSSL)
static BOOLEAN _IsPKCS7Wrapped(
    const UINT8 *certData,
    UINTN certSize)
{
    BOOLEAN status = FALSE;
    PKCS7WrapHeader header1 = _header;
    PKCS7WrapHeader header2;

    /* If not big enough hold contain header */
    if (certSize < sizeof(PKCS7WrapHeader))
        goto done;

    /* Copy the header from 'certData' and blank out the lengths */
    Memcpy(&header2, certData, sizeof(PKCS7WrapHeader));
    header2.length1 = 0;
    header2.length2 = 0;

    /* Compate the two headers (with zero lengths) */
    if (Memcmp(&header1, &header2, sizeof(PKCS7WrapHeader)) != 0)
        goto done;

    status = TRUE;

done:

    return status;
}
#endif /* defined(HAVE_OPENSSL) */

#if defined(HAVE_OPENSSL)
static int _WrapPKCS7Cert(
    IN const UINT8 *certData,
    IN UINTN certSize,
    OUT UINT8 **certDataOut,
    OUT UINTN *certSizeOut)
{
    int status = -1;
    UINT8* p = NULL;
    UINTN n;
    PKCS7WrapHeader header = _header;

    /* Check parameters */
    if (!certData || !certSize || !certDataOut || !certSizeOut)
        goto done;

    /* Clear output parameters */
    *certDataOut = NULL;
    *certSizeOut = 0;

    /* Compute size of wrapped certificate */
    n = certSize + sizeof(PKCS7WrapHeader);

    /* Allocate memory for certificate */
    if (!(p = Malloc(n)))
        goto done;

    /* Set the wrapped certificate size (big endian) */
    header.length1 = ByteSwapU16(n - 4);

    /* Set the unwrapped certificate size (big endian) */
    header.length2 = ByteSwapU16(certSize);

    /* Copy header */
    Memcpy(p, &header, sizeof(PKCS7WrapHeader));

    /* Copy certificate */
    Memcpy (p + sizeof(PKCS7WrapHeader), certData, certSize);

    status = 0;

done:

    if (status == 0)
    {
        *certDataOut = p;
        *certSizeOut = n;
    }
    else
        Free(p);

    return status;
}
#endif /* defined(HAVE_OPENSSL) */

#if defined(HAVE_OPENSSL)
int _VerifyCallback(int status, X509_STORE_CTX *ctx)
{
    int err;
    
    switch (err = X509_STORE_CTX_get_error(ctx))
    {
        case X509_V_ERR_INVALID_PURPOSE:
        {
            if (ctx->cert->ex_xkusage == XKU_CODE_SIGN)
                status = 1;

            break;
        }
        case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
        case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
        case X509_V_ERR_CERT_UNTRUSTED:
        {
            X509_OBJECT obj;

            obj.type = X509_LU_X509;
            obj.data.x509 = ctx->current_cert;

            if (X509_OBJECT_retrieve_match(ctx->ctx->objs, &obj)) 
                status = 1;

            break;
        }
        case X509_V_ERR_CERT_HAS_EXPIRED:
        case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
        case X509_V_ERR_CERT_NOT_YET_VALID:
        case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
        {
            status = 1;
            break;
        }
    }

    return status;
}
#endif /* defined(HAVE_OPENSSL) */

#if defined(HAVE_OPENSSL)
static int _CreateStoreWithCert(
    X509 *x509,
    X509_STORE** storeOut)
{
    int status = -1;
    X509_STORE* store = NULL;

    /* Set output parameter */
    if (storeOut)
        *storeOut = NULL;

    /* Check for null parameters */
    if (!x509 || !storeOut)
        goto done;

    /* Create new X509 store object */
    if (!(store = X509_STORE_new()))
        goto done;

    /* Add X509 certificate to the store */
    if (!(X509_STORE_add_cert(store, x509)))
        goto done;

    /* Register the verification callback for this store */
    X509_STORE_set_verify_cb_func(store, _VerifyCallback);

    /* Set store's purpose */
    X509_STORE_set_purpose(store, X509_PURPOSE_ANY);

    /* Set output parameter */
    *storeOut = store;

    status = 0;

done:

    if (status != 0)
    {
        if (store)
            X509_STORE_free(store);
    }

    return status;
}
#endif /* defined(HAVE_OPENSSL) */

#if defined(HAVE_OPENSSL)
static int _VerifyPKCS7Certificate(
    const UINT8 *trustedCertData,
    UINTN trustedCertSize,
    const UINT8 *imageCertData,
    UINTN imageCertSize,
    const UINT8 *contentData,
    UINTN contentSize)
{
    int status = -1;
    PKCS7 *p7 = NULL;
    BIO *bio = NULL;
    X509 *x509 = NULL;
    X509_STORE *store = NULL;
    UINT8* wrappedCertData = NULL;
    UINTN wrappedCertSize = 0;

    /* Check for null parameters */
    if (!trustedCertSize || !imageCertData || !contentData)
    {
        goto done;
    }

    /* Add SHA256 digest algorithm */
    if (!EVP_add_digest(EVP_sha256()))
    {
        goto done;
    }

    /* Wrap certificate if not already wrapped */
    if (_IsPKCS7Wrapped(imageCertData, imageCertSize))
    {
        wrappedCertData = (UINT8*)imageCertData;
        wrappedCertSize = imageCertSize;
    }
    else
    {
        if (_WrapPKCS7Cert(
            imageCertData,
            imageCertSize,
            &wrappedCertData,
            &wrappedCertSize) != 0)
        {
            goto done;
        }
    }

    /* Create PKCS7 object */
    {
        const UINT8 *tmp = wrappedCertData;

        if (!(p7 = d2i_PKCS7(NULL, &tmp, wrappedCertSize)))
            goto done;
    }

    /* If not signed data, then fail */
    if (!PKCS7_type_is_signed(p7))
    {
        goto done;
    }

    /* Get the X509 certificate */
    {
        const UINT8* tmp = trustedCertData;

        if (!(x509 = d2i_X509(NULL, &tmp, trustedCertSize)))
        {
            goto done;
        }
    }

    /* Create store with X509 certificate */
    if (_CreateStoreWithCert(x509, &store) != 0)
    {
        goto done;
    }

    /* Create BIO object */
    if (!(bio = BIO_new(BIO_s_mem())))
    {
        goto done;
    }

    /* Write SPC indirect content into the BIO */
    if (BIO_write(bio, contentData, contentSize) != contentSize)
    {
        goto done;
    }

    /* Verify the certificate */
    if (!PKCS7_verify(p7, NULL, store, bio, NULL, PKCS7_BINARY))
    {
        goto done;
    }

    /* Success! */
    status = 0;

done:

    if (bio)
        BIO_free(bio);

    if (store)
        X509_STORE_free(store);

    if (x509)
        X509_free(x509);

    if (p7)
        PKCS7_free(p7);

    if (imageCertData != wrappedCertData)
        Free(wrappedCertData);

    return status;
}
#endif /* defined(HAVE_OPENSSL) */

#if defined(HAVE_OPENSSL)
static int _CheckCertAux(
    const UINT8 *trustedCertData,
    UINTN trustedCertSize,
    const UINT8 *imageCertData,
    UINTN imageCertSize,
    const UINT8 *hashData,
    UINTN hashSize)
{
    int status = -1;
    PKCS7 *p7 = NULL;
    const UINT8 SPC_OID[] = {0x2B,0x06,0x01,0x04,0x01,0x82,0x37,0x02,0x01,0x04};
    const UINT8 *contentData;
    UINTN contextSize;


    /* Check for null parameters */
    if (!trustedCertSize || !imageCertData || !hashData)
    {
        goto done;
    }

    /* Parse the image's cert into PKCS7 format */
    {
        const UINT8* tmp = imageCertData;

        if (!(p7 = d2i_PKCS7(NULL, &tmp, imageCertSize)))
        {
            goto done;
        }
    }

    /* If not signed data, then fail */
    if (!PKCS7_type_is_signed(p7))
    {
        goto done;
    }

    /* Check the software-publisher's certificate (SPC) OID */
    if (Memcmp(p7->d.sign->contents->type->data, SPC_OID, sizeof(SPC_OID)) != 0)
    {
        goto done;
    }

    /* Get SPC indirect content from the ANS1 string and verify hash */
    {
        ASN1_STRING* asn1 = p7->d.sign->contents->d.other->value.asn1_string;
        BYTE* p = ASN1_STRING_data(asn1);
        UINTN n = ASN1_STRING_length(asn1);

        /* If not enough room for short length encoding and hash */
        if (n < 3 + hashSize)
        {
            goto done;
        }

        /* Advance past the length encoding (long or short) */
        if (p[1] & 0x80)
        {
            /* Long length encoding */
            p += 4;
            n -= 4;
        }
        else
        {
            /* Short length encoding */
            p += 2;
            n -= 2;
        }

        /* Set pointer and size of the SPC indirect content */
        contentData = p;
        contextSize = n;

        /* Set p to digest */
        p += n - hashSize;

        /* Check length of the digest */
        if (p[-1] != hashSize)
        {
            goto done;
        }

        /* Verify that image hash and certificate hash are identical */
        if (Memcmp(p, hashData, hashSize) != 0)
        {
            goto done;
        }
    }

    /* Verify the signature */
    if (_VerifyPKCS7Certificate(
        trustedCertData, 
        trustedCertSize, 
        imageCertData, 
        imageCertSize, 
        contentData, 
        contextSize) != 0)
    {
        goto done;
    }

    status = 0;

done:

    if (p7)
        PKCS7_free(p7);

    return status;
}
#endif /* defined(HAVE_OPENSSL) */

#if defined(HAVE_OPENSSL)
int CheckCert(
    const void* imageData,
    UINTN imageSize,
    const void* certData,
    UINTN certSize)
{
    int status = -1;
    Image image;
    SHA1Hash sha1;
    SHA256Hash sha256;

    /* Initialize the image */
    Memset(&image, 0, sizeof(Image));

    /* Check the parameters */
    if (!imageData || !imageSize)
        goto done;

    /* Parse the image */
    if (ParseImage(&image, imageData, imageSize) != 0)
    {
        goto done;
    }

    /* Hash the image */
    if (HashImage(&image, &sha1, &sha256) != 0)
    {
        goto done;
    }

    /* Check the certificate */
    if (_CheckCertAux(
        certData,
        certSize,
        image.signatureData,
        image.signatureSize,
        (const UINT8*)&sha256,
        sizeof(sha256)) != 0)
    {
        goto done;
    }

    status = 0;

done:

    ReleaseImage(&image);

    return status;
}
#endif /* defined(HAVE_OPENSSL) */

BOOLEAN IsEFIImage(
    IN const void* imageData,
    IN UINTN imageSize)
{
    const EFI_IMAGE_DOS_HEADER *dh;

    if (!imageData || imageSize < sizeof(EFI_IMAGE_DOS_HEADER))
        return FALSE;

    dh = (const EFI_IMAGE_DOS_HEADER*)imageData;

    if (dh->e_magic != EFI_IMAGE_DOS_SIGNATURE)
        return FALSE;

    return TRUE;
}

BOOLEAN IsKernelImage(
    IN const void* imageData,
    IN UINTN imageSize)
{
    const setup_header_t* sh;

    if (!imageData || imageSize < SETUP_OFFSET + sizeof(sh))
        return FALSE;

    if (!IsEFIImage(imageData, imageSize))
        return FALSE;

    sh = (const setup_header_t*)((char*)imageData + SETUP_OFFSET);

    if (sh->boot_flag != BOOT_FLAG)
        return FALSE;

    if (sh->header != HEADER_MAGIC)
        return FALSE;

    return TRUE;
}

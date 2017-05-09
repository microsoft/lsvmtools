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
#include <zlib.h>
#include "zlibextras.h"
#include <stdlib.h>
#include <string.h>

/*
**==============================================================================
**
** Local definitions:
**
**==============================================================================
*/


#if defined(printf)
# undef printf
#endif

#if defined(malloc)
# undef malloc
#endif

#if defined(free)
# undef free
#endif

#if !defined(BUILD_EFI)
extern int printf(const char *format, ...);
#endif

static void* _Alloc(posix_size_t size)
{
#if defined(BUILD_EFI)
    return AllocatePool(size);
#else
    extern void *malloc(size_t size);
    return malloc(size);
#endif
}

static void _Free(void* ptr)
{
#if defined(BUILD_EFI)
    FreePool(ptr);
#else
    extern void free(void* ptr);
    free(ptr);
#endif
}

/*
**==============================================================================
**
** InputContext:
**
**==============================================================================
*/

#define INPUTCONTEXT_INITIALIZER { NULL, 0, 0 }

typedef struct _InputContext
{
    const void* data;
    posix_size_t size;
    posix_off_t offset;
}
InputContext;

static posix_off_t _InputLSeekCallback(
    void* context_,
    posix_off_t offset,
    int whence)
{
    InputContext* context = (InputContext*)context_;
    posix_off_t n = -1;

    if (!context)
        goto done;

    if (whence == SEEK_SET)
        n = offset;
    else if (whence == SEEK_CUR)
        n = context->offset + offset;
    else if (whence == SEEK_END)
        n = context->size + offset;
    else
        goto done;

    if (n < 0 || n >= context->size)
    {
        n = -1;
        goto done;
    }

    context->offset = n;

done:
    return n;
}

static posix_ssize_t _InputReadCallback(
    void* context_,
    void* buf,
    posix_ssize_t count)
{
    InputContext* context = (InputContext*)context_;
    posix_ssize_t n = -1;
    posix_ssize_t r;

    if (!context || !buf)
        goto done;

    r = context->size - context->offset;

    if (r == 0)
    {
        n = 0;
    }
    else if (r >= count)
    {
        posix_memcpy(buf, context->data + context->offset, count);
        context->offset += count;
        n = count;
    }
    else
    {
        posix_memcpy(buf, context->data + context->offset, r);
        context->offset += r;
        n = r;
    }

done:
    return n;
}

static posix_ssize_t _InputWriteCallback(
    void* context,
    const void* buf,
    posix_ssize_t size)
{
    /* Writes not permited on read files */
    return -1;
}

static int _InputCloseCallback(
    void* context)
{
    /* Nothing special to do! */
    return 0;
}

/*
**==============================================================================
**
** OutputContext: (write-only, no seek)
**
**==============================================================================
*/

typedef struct _OutputBlock OutputBlock;

struct _OutputBlock
{
    OutputBlock* next;
    posix_size_t size;
    char buf[0];
};

#define OUTPUTCONTEXT_INITIALIZER { NULL, NULL, 0 }

typedef struct _OutputContext
{
    OutputBlock* head;
    OutputBlock* tail;
    posix_size_t size;
}
OutputContext;

static posix_off_t _OutputLSeekCallback(
    void* context_,
    posix_off_t offset,
    int whence)
{
    /* Not implemenented */
    return -1;
}

static posix_ssize_t _OutputReadCallback(
    void* context_,
    void* buf,
    posix_ssize_t count)
{
    /* Not implemenented */
    return -1;
}

static posix_ssize_t _OutputWriteCallback(
    void* context_,
    const void* buf,
    posix_ssize_t size)
{
    OutputContext* context = (OutputContext*)context_;
    posix_ssize_t rc = -1;
    OutputBlock* block = NULL;

    if (!context)
        goto done;

    /* Create a new block */
    if (!(block = (OutputBlock*)_Alloc(sizeof(OutputBlock) + size)))
        goto done;

    /* Copy buffer into new block */
    posix_memcpy(block->buf, buf, size);
    block->size = size;

    /* Append block to end of the list */
    {
        block->next = NULL;

        if (context->head)
            context->tail->next = block;
        else
            context->head = block;

        context->tail = block;
    }

    /* Update the total size in bytes */
    context->size += size;

    rc = size;

done:

    return rc;
}

static int _OutputCloseCallback(
    void* context_)
{
    return 0;
}

static int _ReleaseOutBlocks(
    OutputContext* context)
{
    int rc = -1;
    OutputBlock* p;

    if (!context)
        goto done;

    for (p = context->head; p; )
    {
        OutputBlock* next = p->next;
        _Free(p);
        p = next;
    }

    rc = 0;

done:
    return rc;
}

static int _CombineBlocks(
    OutputContext* context,
    void** dataOut,
    unsigned long* sizeOut)
{
    int rc = -1;
    void* data = NULL;

    if (dataOut)
        *dataOut = NULL;

    if (sizeOut)
        *sizeOut = 0;

    if (!context || !dataOut || !sizeOut)
        goto done;

    /* Handle zero blocks case */
    if (context->size == 0)
    {
        rc = 0;
        goto done;
    }

    /* Allocate memory to hold all blocks */
    if (!(data = _Alloc(context->size)))
        goto done;

    /* Copy blocks onto flat memory */
    {
        OutputBlock* p;
        posix_off_t offset = 0;

        for (p = context->head; p; p = p->next)
        {
            posix_memcpy((unsigned char*)data + offset, p->buf, p->size);
            offset += p->size;
        }
    }

    *dataOut = data;
    *sizeOut = context->size;

    rc = 0;
    
done:
    return rc;
}

/*
**==============================================================================
**
** Public definitions:
**
**==============================================================================
*/

void __efi_trace(const char* file, unsigned int line);

int zlibextras_compress_with_mode(
    const char* mode,
    const unsigned char* inData,
    unsigned long inSize,
    unsigned char** outData,
    unsigned long* outSize)
{
    int rc = -1;
    const char OUTFILE[] = "zlibextras_compress.gz";
    gzFile gz = NULL;
    int n;
    OutputContext out = OUTPUTCONTEXT_INITIALIZER;

    if (outData)
        *outData = NULL;

    if (outSize)
        *outSize = 0;

    /* Check for null parameters */
    if (!mode || !inData || !outData || !outSize)
        goto done;

    /* Register the output file */
    if (posix_register_file(
        OUTFILE,
        &out,
        _OutputLSeekCallback,
        _OutputReadCallback,
        _OutputWriteCallback,
        _OutputCloseCallback) != 0)
    {
        goto done;
    }

    /* Open output file */
    if (!(gz = gzopen(OUTFILE, mode)))
    {
        goto done;
    }

    /* Write the entire file */
    if ((n = gzwrite(gz, inData, inSize)) < 0)
    {
        goto done;
    }

    /* Close the file, calling _OutputCloseCallback() */
    gzclose(gz);

    /* Combine blocks into flat storage */
    if (_CombineBlocks(&out, (void**)outData, outSize) != 0)
        goto done;

    rc = 0;

done:

    _ReleaseOutBlocks(&out);

    return rc;
}

int zlibextras_compress(
    const unsigned char* inData,
    unsigned long inSize,
    unsigned char** outData,
    unsigned long* outSize)
{
    return zlibextras_compress_with_mode("wb", inData, inSize, outData, outSize);
}

int zlibextras_decompress(
    const unsigned char* inData,
    unsigned long inSize,
    unsigned char** outData,
    unsigned long* outSize)
{
    int rc = -1;
    const char INFILE[] = "zlibextras_decompress";
    gzFile is = NULL;
    int n;
    InputContext in = INPUTCONTEXT_INITIALIZER;
    OutputContext out = OUTPUTCONTEXT_INITIALIZER;

    if (outData)
        *outData = NULL;

    if (outSize)
        *outSize = 0;

    /* Check for null parameters */
    if (!inData || !outData || !outSize)
        goto done;

    /* Initialize the input context */
    in.data = inData;
    in.size = inSize;

    /* Register input file */
    if (posix_register_file(
        INFILE, 
        &in,
        _InputLSeekCallback,
        _InputReadCallback,
        _InputWriteCallback,
        _InputCloseCallback) != 0)
    {
        goto done;
    }

    /* Open input file */
    if (!(is = gzopen(INFILE, "rb")))
    {
        goto done;
    }

    /* Read until no more data */
    {
        char* buf = NULL;
        posix_size_t bufSize = 1024 * 1024;

        if (!(buf = _Alloc(bufSize)))
            goto done;

        while ((n = gzread(is, buf, bufSize)) > 0)
        {
            if (_OutputWriteCallback(&out, buf, n) != n)
            {
                _Free(buf);
                goto done;
            }
        }

        _Free(buf);
    }

    /* Close the file, calling _InputCloseCallback() */
    gzclose(is);

    /* Combine blocks into flat storage */
    if (_CombineBlocks(&out, (void**)outData, outSize) != 0)
    {
        _OutputCloseCallback(&out);
        goto done;
    }

    rc = 0;

done:

    _ReleaseOutBlocks(&out);

    return rc;
}

int zlibextras_test_compress_decompress(
    const unsigned char* data,
    unsigned long size)
{
    int rc = -1;
    unsigned char* compressedData = NULL;
    unsigned long compressedSize;
    unsigned char* decompressedData = NULL;
    unsigned long decompressedSize;

    if (!data)
        goto done;

    /* Compress */
    if (zlibextras_compress(
        data, 
        size, 
        &compressedData, 
        &compressedSize) != 0)
    {
        goto done;
    }

    /* Decompress */
    if (zlibextras_decompress(
        compressedData, 
        compressedSize, 
        &decompressedData, 
        &decompressedSize) != 0)
    {
        goto done;
    }

    if (decompressedSize != size)
        goto done;

    if (posix_memcmp(data, decompressedData, size) != 0)
        goto done;

    rc = 0;

done:

    if (compressedData)
        _Free(compressedData);

    if (decompressedData)
        _Free(decompressedData);

    return rc;
}

int zlibextras_test(
    const unsigned char* data,
    unsigned long size)
{
    static unsigned char _magic[] = { 0x1F, 0x8B };

    if (size >= sizeof(_magic) && 
        posix_memcmp(data, _magic, sizeof(_magic)) == 0)
    {
        return 0;
    }

    return -1;
}

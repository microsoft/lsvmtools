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
#include "lzmaextras.h"
#include <lzma.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
**==============================================================================
**
** Local definitions:
**
**==============================================================================
*/

#if defined(malloc)
# undef malloc
#endif

#if defined(free)
# undef free
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
** BlockList:
**
**==============================================================================
*/

typedef struct _Block Block;

struct _Block
{
    Block* next;
    posix_size_t size;
    char buf[0];
};

#define BLOCKLIST_INITIALIZER { NULL, NULL, 0 }

typedef struct _BlockList
{
    Block* head;
    Block* tail;
    posix_size_t size;
}
BlockList;

static int _BlockListAppend(
    BlockList* list,
    const unsigned char* buf,
    unsigned long size)
{
    posix_ssize_t rc = -1;
    Block* block = NULL;

    if (!list || !buf)
        goto done;

    /* Create a new block */
    if (!(block = (Block*)_Alloc(sizeof(Block) + size)))
        goto done;

    /* Copy buffer into new block */
    posix_memcpy(block->buf, buf, size);
    block->size = size;

    /* Append block to end of the list */
    {
        block->next = NULL;

        if (list->head)
            list->tail->next = block;
        else
            list->head = block;

        list->tail = block;
    }

    /* Update the total size in bytes */
    list->size += size;

    rc = 0;

done:

    return rc;
}

static int _BlockListMerge(
    BlockList* list,
    void** dataOut,
    unsigned long* sizeOut)
{
    int rc = -1;
    void* data = NULL;

    if (dataOut)
        *dataOut = NULL;

    if (sizeOut)
        *sizeOut = 0;

    if (!list || !dataOut || !sizeOut)
        goto done;

    /* Handle zero blocks case */
    if (list->size == 0)
    {
        rc = 0;
        goto done;
    }

    /* Allocate memory to hold all blocks */
    if (!(data = _Alloc(list->size)))
        goto done;

    /* Copy blocks onto flat memory */
    {
        Block* p;
        unsigned long offset = 0;

        for (p = list->head; p; p = p->next)
        {
            posix_memcpy((unsigned char*)data + offset, p->buf, p->size);
            offset += p->size;
        }
    }

    *dataOut = data;
    *sizeOut = list->size;

    rc = 0;
    
done:
    return rc;
}

static int _BlockListRelease(
    BlockList* list)
{
    int rc = -1;
    Block* p;

    if (!list)
        goto done;

    for (p = list->head; p; )
    {
        Block* next = p->next;
        _Free(p);
        p = next;
    }

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

int lzmaextras_compress(
    const unsigned char* inData,
    unsigned long inSize,
    unsigned char** outData,
    unsigned long* outSize)
{
    int rc = -1;
    lzma_stream strm = LZMA_STREAM_INIT; 
    uint8_t buf[4096];
    lzma_ret ret;
    BlockList list = BLOCKLIST_INITIALIZER;

    if (outData)
        *outData = NULL;

    if (outSize)
        *outSize = 0;

    if (!inData || !outData || !outSize)
        goto done;

    /* Initialize the encoder stream */
    if ((ret = lzma_easy_encoder(&strm, 1, LZMA_CHECK_CRC64)) != LZMA_OK)
        goto done;

    /* Initialize the input stream */
    strm.next_in = inData;
    strm.avail_in = inSize;

    /* Initialize the output stream */
    strm.next_out = buf; 
    strm.avail_out = sizeof(buf); 

    /* While more data to compress */
    for (;;)
    {
        ret = lzma_code(&strm, LZMA_FINISH); 

        if (strm.avail_out == 0 || ret == LZMA_STREAM_END) 
        {
            size_t n = sizeof(buf) - strm.avail_out; 

            if (_BlockListAppend(&list, buf, n) != 0)
                goto done;

            strm.next_out = buf; 
            strm.avail_out = sizeof(buf); 
        }

        if (ret == LZMA_STREAM_END)
            break;

        if (ret != LZMA_OK)
            goto done;
    }

    if (_BlockListMerge(&list, (void**)outData, outSize) != 0)
        goto done;

    rc = 0;

done:

    lzma_end(&strm);

    _BlockListRelease(&list);

    return rc;
}

int lzmaextras_decompress(
    const unsigned char* inData,
    unsigned long inSize,
    unsigned char** outData,
    unsigned long* outSize)
{
    int rc = -1;
    lzma_stream strm = LZMA_STREAM_INIT; 
    uint8_t buf[4096];
    const size_t MAX_SIZE = 0xFFFFFFFF;
    BlockList list = BLOCKLIST_INITIALIZER;

    if (outData)
        *outData = NULL;

    if (outSize)
        *outSize = 0;

    /* Check for null parameters */
    if (!inData || !outData || !outSize)
        goto done;

    /* Initialize the input stream */
    strm.next_in = inData;
    strm.avail_in = inSize;

    /* Initialize the decoder stream */
    if (lzma_stream_decoder(&strm, MAX_SIZE, LZMA_CONCATENATED) != LZMA_OK)
        goto done;

    /* Initialize the output stream */
    strm.next_out = buf; 
    strm.avail_out = sizeof(buf); 

    while (strm.avail_in)
    {
        lzma_ret ret;

        ret = lzma_code(&strm, LZMA_FINISH); 

        if (_BlockListAppend(&list, buf, sizeof(buf) - strm.avail_out) != 0)
            goto done;

        strm.next_out = buf; 
        strm.avail_out = sizeof(buf); 
        
        if (ret == LZMA_STREAM_END) 
            break; 

        if (ret != LZMA_OK)
            goto done;
    }

    if (_BlockListMerge(&list, (void**)outData, outSize) != 0)
        goto done;

    rc = 0;

done:

    lzma_end(&strm);

    _BlockListRelease(&list);

    return rc;
} 

int lzmaextras_test_compress_decompress(
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
    if (lzmaextras_compress(
        data, 
        size, 
        &compressedData, 
        &compressedSize) != 0)
    {
        goto done;
    }

    /* Decompress */
    if (lzmaextras_decompress(
        compressedData, 
        compressedSize, 
        &decompressedData, 
        &decompressedSize) != 0)
    {
        goto done;
    }

    if (decompressedSize != size)
        goto done;

    if (memcmp(data, decompressedData, size) != 0)
        goto done;

    rc = 0;

done:

    if (compressedData)
        _Free(compressedData);

    if (decompressedData)
        _Free(decompressedData);

    return rc;
}

int lzmaextras_test(
    const unsigned char* data,
    unsigned long size)
{
    static unsigned char _magic[] = { 0xFD, 0x37, 0x7A, 0x58, 0x5A, 0x00 };

    if (size >= sizeof(_magic) && memcmp(data, _magic, sizeof(_magic)) == 0)
        return 0;

    return -1;
}

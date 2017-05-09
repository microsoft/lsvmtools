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
#ifndef _mbutils_heap_h
#define _mbutils_heap_h

#include "config.h"
#include "eficommon.h"
#include "inline.h"

/*
 * Maintains a heap of dynamically allocated objects that may be released 
 * all at once by calling HeapFree().
 */

#define HEAP_SIZE 32

#define HEAP_INITIALIZER { 0 }

typedef struct _Heap
{
    UINTN size;
    void* data[HEAP_SIZE];
}
Heap;

INLINE int HeapAdd(Heap* heap, void* ptr)
{
    if (heap->size == HEAP_SIZE)
        return -1;

    heap->data[heap->size++] = ptr;
    return 0;
}

void HeapFree(Heap* heap);

#endif /* _mbutils_heap_h */

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
#ifndef _strarr_h
#define _strarr_h

#include "config.h"
#include "eficommon.h"

#define STRARR_INITIALIZER { NULL, 0, 0 }

typedef struct _StrArr
{
    char** data;
    UINTN size;
    UINTN capacity;
}
StrArr;

void StrArrRelease(
    StrArr* self);

int StrArrAppend(
    StrArr* self,
    const char* data);

int StrArrAppendBorrow(
    StrArr* self,
    char* data);

void StrArrSort(
    StrArr* self);

/* Return ((UINTN)-1) if not found */
UINTN StrArrFind(
    const StrArr* self,
    const char* str);

int StrArrRemove(
    StrArr* self,
    UINTN index);

int StrSplit(
    const char* str,
    const char* delim,
    StrArr* arr);

#endif /* _strarr_h */

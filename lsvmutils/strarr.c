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
#include "strarr.h"
#include "strings.h"
#include "alloc.h"

static UINTN CAPACITY = 32;

void StrArrRelease(
    StrArr* self)
{
    UINTN i;

    for (i = 0; i < self->size; i++)
        Free(self->data[i]);

    Free(self->data);

    Memset(self, 0, sizeof(StrArr));
}

int StrArrAppend(
    StrArr* self,
    const char* str)
{
    /* Increase the capacity */
    if (self->size == self->capacity)
    {
        UINTN capacity;
        void* data;

        if (self->capacity)
            capacity = self->capacity * 2;
        else
            capacity = CAPACITY;

        if (!(data = Realloc(
            self->data, 
            sizeof(char*) * self->capacity, /* oldSize */
            sizeof(char*) * capacity)))  /* newSize */
        {
            return -1;
        }

        self->data = data;
        self->capacity = capacity;
    }

    /* Append the new string */
    {
        char* newStr;
        
        if (str)
        {
            if (!(newStr = Strdup(str)))
                return -1;
        }
        else
        {
            newStr = NULL;
        }

        self->data[self->size++] = newStr;
    }

    return 0;
}

int StrArrAppendBorrow(
    StrArr* self,
    char* str)
{
    /* Increase the capacity */
    if (self->size == self->capacity)
    {
        UINTN capacity;
        void* data;

        if (self->capacity)
            capacity = self->capacity * 2;
        else
            capacity = CAPACITY;

        if (!(data = Realloc(
            self->data, 
            sizeof(char*) * self->capacity, /* oldSize */
            sizeof(char*) * capacity)))  /* newSize */
        {
            return -1;
        }

        self->data = data;
        self->capacity = capacity;
    }

    /* Append the new string */
    self->data[self->size++] = str;

    return 0;
}

void StrArrSort(
    StrArr* self)
{
    UINTN i;
    UINTN j;
    UINTN n;

    if (self->size == 0)
        return;

    n = self->size - 1;

    for (i = 0; i < self->size - 1; i++)
    {
        BOOLEAN swapped = FALSE;

        for (j = 0; j < n; j++)
        {
            if (Strcmp(self->data[j], self->data[j+1]) > 0)
            {
                char* tmp = self->data[j];
                self->data[j] = self->data[j + 1];
                self->data[j + 1] = tmp;
                swapped = TRUE;
            }
        }

        if (!swapped)
            break;

        n--;
    }
}

UINTN StrArrFind(
    const StrArr* self,
    const char* str)
{
    UINTN i;

    for (i = 0; i < self->size; i++)
    {
        if (Strcmp(self->data[i], str) == 0)
            return i;
    }

    return (UINTN)-1;
}

int StrArrRemove(
    StrArr* self,
    UINTN index)
{
    UINTN i;

    /* Check for out of bounds */
    if (index >= self->size)
        return -1;

    Free(self->data[index]);

    /* Remove the element */
    for (i = index; i < self->size - 1; i++)
    {
        self->data[i] = self->data[i+1];
    }

    self->size--;
    return 0;
}

int StrSplit(
    const char* str,
    const char* delim,
    StrArr* arr)
{
    int rc = -1;
    char* copy = NULL;

    if (arr)
        StrArrRelease(arr);

    /* Check parameters */
    if (!str || !delim || !arr)
        goto done;

    /* Make a copy of the string */
    if (!(copy = Strdup(str)))
        goto done;

    /* For each element */
    {
        char* p;
        char* save = NULL;

        for (p = Strtok(copy, delim, &save); p; p = Strtok(NULL, delim, &save))
        {
            if (StrArrAppend(arr, p) != 0)
                goto done;
        }
    }

    rc = 0;

done:

    if (copy)
        Free(copy);

    if (rc != 0)
        StrArrRelease(arr);

    return rc;
}

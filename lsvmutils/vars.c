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
#include "vars.h"
#include "strings.h"
#include "alloc.h"

const Var* VarsFind(const Vars* vars, const char* name)
{
    UINTN i;

    if (!vars || !name)
        return NULL;

    for (i = 0; i < vars->size; i++)
    {
        if (Strcmp(vars->data[i].name, name) == 0)
            return &vars->data[i];
    }

    return NULL;
}

int VarsLoad(
    const Vars* vars, 
    const char* name,
    UINTN extraBytes,
    unsigned char** data,
    UINTN* size)
{
    int rc = -1;
    const Var* var;

    if (data)
        *data = NULL;

    if (size)
        *size = 0;

    if (!vars || !name || !data || !size)
        goto done;

    if (!(var = VarsFind(vars, name)))
        goto done;

    if (!(*data = (unsigned char*)Malloc(var->size + extraBytes)))
        goto done;

    Memcpy(*data, var->data, var->size);
    *size = var->size;

    rc = 0;

done:
    return rc;
}

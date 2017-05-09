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
#ifndef _faults_h
#define _faults_h

#include "config.h"
#include <lsvmutils/eficommon.h>
#include <lsvmutils/inline.h>

/*
**==============================================================================
**
** defined(ENABLE_FAULTS)
**
**==============================================================================
*/

#if defined(ENABLE_FAULTS)

BOOLEAN CheckFault(const char* name);

int AppendFault(const char* name);

#endif /* defined(ENABLE_FAULTS) */

/*
**==============================================================================
**
** !defined(ENABLE_FAULTS)
**
**==============================================================================
*/

#if !defined(ENABLE_FAULTS)

INLINE BOOLEAN CheckFault(const char* name)
{
    return FALSE;
}

INLINE int AppendFault(const char* name)
{
    return 0;
}

#endif /* !defined(ENABLE_FAULTS) */

#endif /* _faults_h */

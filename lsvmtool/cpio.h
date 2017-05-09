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
#ifndef _cpio_h
#define _cpio_h

#include "config.h"
#include <lsvmutils/cpio.h>

int CPIOAddFileByPath(
    const char* cpioInPath,
    const char* cpioOutPath,
    const char* newPath,
    const char* filePath,
    unsigned int mode);

int CPIORemoveFileByPath(
    const char* cpioInPath,
    const char* cpioOutPath,
    const char* path);

int CPIODumpFileByPath(
    const char* cpioPath);
        
int CPIOCheckFileByPath(
    const char* cpioPath,
    const char* path);

/* Splits a file based off the CPIO terminating header. */
int CPIOSplitFileByPath(
    const char* cpioPath);

int CPIOMergeFilesByPath(
    const char* cpioOut,
    const char* const files[],
    size_t numFiles);

int CPIOGetFileByPath(
    const char* cpioPath,
    const char* src,
    const char* dest);

#endif /* _cpio_h */

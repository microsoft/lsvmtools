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
#include <stdio.h>
#include <lsvmutils/utils.h>
#include <lsvmutils/strings.h>

int ext2_main(int argc, const char* argv[]);

#if defined(ENABLE_VFAT)
int vfat_main(int argc, const char* argv[]);
#endif /* defined(ENABLE_VFAT) */

int lsvmtool_main(int argc, const char* argv[]);

int cpio_main(int argc, const char* argv[]);

int main(int argc, const char* argv[])
{
    char programName[PATH_MAX];

    if (argc < 1)
        return 1;

    GetProgramName(argv[0], programName);

    if (Strcmp(programName, "ext2") == 0)
    {
        return ext2_main(argc, argv);
    }
#if defined(ENABLE_VFAT)
    if (Strcmp(programName, "vfat") == 0)
    {
        return vfat_main(argc, argv);
    }
#endif /* defined(ENABLE_VFAT) */
    else if (Strcmp(programName, "lsvmcpio") == 0)
    {
        return cpio_main(argc, argv);
    }
    else if (Strcmp(programName, "lsvmtool") == 0)
    {
        return lsvmtool_main(argc, argv);
    }
    else
    {
        fprintf(stderr, "%s: unknown command: %s\n", argv[0], programName);
        return 1;
    }

    /* Unreachable! */
    return 0;
}

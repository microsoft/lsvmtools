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
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "strings.h"

char *StrcpySafe(
    char *dest, 
    size_t size, 
    const char *src)
{
#if defined(__linux__) || defined(BUILD_EFI)
    return strcpy(dest, src);
#endif

#if defined(_WIN32)
    strcpy_s(dest, size, src);
    return dest;
#endif
}

#if 0
char *StrncatSafe(
    char *dest, 
    size_t size, 
    const char *src, 
    size_t count)
{
#if defined(__linux__) || defined(BUILD_EFI)
    return strncat(dest, src, count);
#endif

#if defined(_WIN32)
    strncat_s(dest, size, src, count);
    return dest;
#endif
}
#endif

FILE* Fopen(
    const char* path,
    const char* mode)
{
#if defined(__linux__) || defined(BUILD_EFI)
    return fopen(path, mode); 
#endif 

#if defined(_WIN32)
    FILE* is = NULL;
    if (fopen_s(&is, path, mode) != 0)
        return NULL;
    return is;
#endif
}

#if !defined(BUILD_EFI)
char* Dupenv(
    const char* name)
{
#if defined(__linux__)
    const char* s = getenv(name);
    if (!s)
        return NULL;
    return strdup(s);
#endif

#if defined(_WIN32)
    char* p = NULL;
    size_t n;
    if (_dupenv_s(&p, &n, name) != 0)
        return NULL;
    return p;
#endif
}
#endif /* !defined(BUILD_EFI) */

const char* Basename(
    const char* path)
{
    const char* p = path + Strlen(path);

#if defined(__linux__) || defined(BUILD_EFI)
    while (p != path && p[-1] != '/')
        p--;
#endif

#if defined(_WIN32)
    while (p != path && p[-1] != '/' && p[-1] != '\\')
        p--;
#endif
    return p;
}

#if !defined(BUILD_EFI)
void GetProgramName(
    const char* path,
    char programName[PATH_MAX])
{
    char tmp[PATH_MAX];
    const char* p;

    /* Make a copy of the path that we can modify */
    Strlcpy(tmp, path, PATH_MAX);
   
    /* Find the basename (part after optional slash) */
    p = Basename(tmp);

#if defined(_WIN32)
    /* Remove the extension (if any) */
    {
        char* ext = strchr(p, '.');

        if (ext && (Strcmp(ext, ".exe") == 0 || Strcmp(ext, ".EXE") == 0))
            *ext = '\0';
    }
#endif /* defined(_WIN32) */

    Strlcpy(programName, p, PATH_MAX);
}
#endif /* !defined(BUILD_EFI) */

/* Expand environment variables like these: ${MYENV} and ${X} */
int ExpandEnvVars(
    char* buf,
    UINTN bufSize,
    const char* str)
{
    int rc = -1;
    const char* p;

    if (!buf || bufSize == 0 || !str)
        goto done;

    /* Null terminate output buffer */
    buf[0] = '\0';

    for (p = str; *p; )
    {
        /* Check for variable expression of the form "${...}" */
        if (p[0] == '$' && p[1] == '{')
        {
            char name[64];

            /* Set 'name' to the variable name given by "${...}" expression */
            {
                const char* start;

                p += 2;
                start = p;

                while (*p && *p != '}')
                    p++;

                if (*p != '}')
                    goto done;

                if (p - start >= sizeof(name))
                    goto done;


                Memcpy(name, start, p - start);
                name[p - start] = '\0';
                p++;
            }

            /* Get variable value and concatenate it to the output buffer */
            {
                char* var;

                if (!(var = Dupenv(name)))
                    goto done;

                if (Strlcat(buf, var, bufSize) >= bufSize)
                {
                    free(var);
                    goto done;
                }

                free(var);
            }
        }
        else
        {
            char tmp[2];

            tmp[0] = *p++;
            tmp[1] = '\0';

            if (Strlcat(buf, tmp, bufSize) >= bufSize)
                goto done;
        }
    }

    rc = 0;

done:
    return rc;
}

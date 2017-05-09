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
#include "conf.h"
#include "strings.h"
#include "error.h"
#include "alloc.h"

static const char* _GetLine(const char** pp, const char* end)
{
    const char* p = *pp;
    const char* start = p;

    if (p == end)
        return NULL;

    while (p != end && *p++ != '\n')
        ;

    *pp = p;
        
    return start;
}

static const char* _SkipIdentifier(const char* p, const char* end)
{
    if (p == end)
        return p;

    if (!Isalpha(*p) || *p == '_')
        return p;

    p++;

    while (p != end && Isalnum(*p))
        p++;

    return p;
}

static const char* _SkipWhitespace(
    const char* p,
    const char* end)
{
    while (p != end && Isspace(*p))
        p++;

    return p;
}

int ParseConf(
    const char* text,
    unsigned long textSize,
    ConfCallback callback,
    void* callbackData,
    unsigned int* errorLine,
    Error* err)
{
    int status = 0;
    const char* line;
    const char* textEnd;
    unsigned int lineNum = 0;
    char* namePtr = NULL;
    char* valuePtr = NULL;

    /* Check parameters */
    if (!text || !textSize || !errorLine || !err)
    {
        SetErr(err, TCS("invalid parameter"));
        status = -1;
        goto done;
    }

    /* Set pointer to the end of the text */
    textEnd = text + textSize;

    /* Clear error state */
    *errorLine = 0;
    ClearErr(err);

    /* Process lines of the format NAME=SHA1:SHA256 */
    while ((line = _GetLine(&text, textEnd)))
    {
        const char* p = line;
        const char* end = text;
        const char* name = NULL;
        UINTN nameLen = 0;
        const char* value = NULL;
        UINTN valueLen = 0;

        /* Increment the line number */
        lineNum++;

        /* Strip horizontal whitespace */
        p = _SkipWhitespace(p, end);

        /* Skip blank lines and comment lines */
        if (p == end || *p == '#')
            continue;

        /* Remove trailing whitespace */
        while (end != p && Isspace(end[-1]))
            end--;

        /* Recognize the name: [A-Za-z_][A-Za-z_0-9] */
        {
            const char* start = p;

            p = _SkipIdentifier(p, end);

            if (p == start)
            {
                SetErr(err, TCS("expected name"));
                status = -1;
                goto done;
            }

            /* Save the name */
            name = start;
            nameLen = p - start;
        }

        /* Expect a '=' */
        {
            p = _SkipWhitespace(p, end);

            if (p == end || *p++ != '=')
            {
                SetErr(err, TCS("syntax error: expected '='"));
                status = -1;
                goto done;
            }

            p = _SkipWhitespace(p, end);
        }

        /* Get the value */
        {
            value = p;
            valueLen = end - p;
        }

        /* Invoke the callback */
        if (callback)
        {
            if (!(namePtr = Strndup(name, nameLen)))
            {
                SetErr(err, TCS("out of memory"));
                status = -1;
                goto done;
            }

            if (!(valuePtr = Strndup(value, valueLen)))
            {
                SetErr(err, TCS("out of memory"));
                status = -1;
                goto done;
            }

            if ((*callback)(namePtr, valuePtr, callbackData, err) != 0)
            {
                status = -1;
                goto done;
            }

            Free(namePtr);
            namePtr = NULL;
            Free(valuePtr);
            valuePtr = NULL;
        }
    }

done:

    if (status != 0)
        *errorLine = lineNum;

    if (namePtr)
        Free(namePtr);

    if (valuePtr)
        Free(valuePtr);

    return status;
}

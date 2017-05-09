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
#include "grubcfg.h"
#include "strings.h"
#include "print.h"
#include "strarr.h"
#include "buf.h"
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

static const char* _SkipWhitespace(
    const char* p,
    const char* end)
{
    while (p != end && Isspace(*p))
        p++;

    return p;
}

#if 0
int GrubcfgFindInitrdPaths(
    const void* data,
    const UINTN size,
    StrArr* paths)
{
    int rc = -1;
    const char* text = data;
    const char* textEnd = data + size;
    const char* line;

    /* Check for bad parameters */
    if (!data || !size || !paths)
        goto done;

    /* Clear the array */
    StrArrRelease(paths);

    /* Process line by line */
    while ((line = _GetLine(&text, textEnd)))
    {
        const char* p = line;
        const char* end = text;
        BOOLEAN found = FALSE;

        /* Strip horizontal whitespace */
        p = _SkipWhitespace(p, end);

        /* Skip blank lines and comment lines */
        if (p == end || *p == '#')
            continue;

        /* Remove trailing whitespace */
        while (end != p && Isspace(end[-1]))
            end--;

        /* Skip blank lines */
        if (p == end)
            continue;

        /* Check for "initrd" and "initrdefi" */
        if (Strncmp(p, "initrd", 6) == 0 && Isspace(p[6]))
        {
            p += 7;
            found = TRUE;
        }
        else if (Strncmp(p, "initrdefi", 9) == 0 && Isspace(p[9]))
        {
            p += 10;
            found = TRUE;
        }

        if (!found)
            continue;

        /* Skip over whitespace leading up to initrd name */
        {
            p = _SkipWhitespace(p, end);

            if (*p == '\0')
                continue;
        }

        if (end - p >= PATH_MAX)
        {
            /* If too long */
            goto done;
        }

        /* Append this path to the array */
        {
            char path[PATH_MAX];
            Memcpy(path, p, end - p);
            path[end - p] = '\0';

            /* Insert if not a duplicate */
            if (StrArrFind(paths, path) == (UINTN)-1)
            {
                if (StrArrAppend(paths, path, TRUE) != 0)
                    goto done;
            }
        }
    }

    if (paths->size == 0)
        goto done;

    rc = 0;

done:

    if (rc != 0)
        StrArrRelease(paths);

    return rc;
}
#endif

int GrubcfgPatch(
    void* data,
    const UINTN size)
{
    int rc = -1;
    const char* text = data;
    const char* textEnd = data + size;
    const char* line;

    /* Check for bad parameters */
    if (!data || !size)
        goto done;

    /* Process line by line */
    while ((line = _GetLine(&text, textEnd)))
    {
        const char* p = line;
        const char* end = text;
        UINTN len;
        static const char* commands[] =
        {
            "insmod cryptodisk",
            "insmod luks",
            "insmod gcry_rijndael",
            "insmod gcry_sha1"
        };
        const UINTN ncommands = ARRSIZE(commands);
        UINTN i;

        /* Strip horizontal whitespace */
        p = _SkipWhitespace(p, end);

        /* Skip blank lines and comment lines */
        if (p == end || *p == '#')
            continue;

        /* Remove trailing whitespace */
        while (end != p && Isspace(end[-1]))
            end--;

        /* Save length of line */
        len = end - p;

        /* Remove loading of cryptodisk module */
        for (i = 0; i < ncommands; i++)
        {
            const char* s = commands[i];
            UINTN n = Strlen(commands[i]);

            if (len >= n && Memcmp(p, s, n) == 0)
                Memset((char*)p, '#', n);
        }
    }

    rc = 0;

done:

    return rc;
}

typedef struct _GRUBCommand GRUBCommand;

struct _GRUBCommand
{
    UINTN line;
    UINTN argc;
    char** argv;
};

static int _GetToken(const char** text, const char** tok)
{
    const char* p = *text;

    /* Initialzie output parameter */
    *tok = NULL;

    /* Skip over horizontal whitespace */
    while (*p && Isspace(*p) && *p != '\n')
        p++;

    /* Ignore comments */
    if (*p == '#')
    {
        while (*p && *p != '\n')
            p++;
    }

    /* Handle end-of-file */
    if (*p == '\0')
    {
        *text = p;
        return '\0';
    }

    /* Handle '\n' */
    if (*p == '\n')
    {
        *text = p + 1;
        return '\n';
    }

    /* Handle ';' */
    if (*p == ';')
    {
        *text = p + 1;
        return ';';
    }

    /* Get the next agument */
    {
        Buf buf = BUF_INITIALIZER;

        /* Build the output string */
        while (*p && !Isspace(*p) && *p != ';')
        {
            if (*p == '"' || *p == '\'')
            {
                char c = *p++;

                while (*p && *p != c)
                {
                    if (*p == '\\' && *p != '\0')
                    {
                        BufAppend(&buf, &p[1], sizeof(char));
                        p += 2;
                    }
                    else
                    {
                        BufAppend(&buf, p, sizeof(char));
                        p++;
                    }
                }

                if (*p == c)
                    p++;
            }
            else
            {
                BufAppend(&buf, p, sizeof(char));
                p++;
            }
        }

        /* Zero-terminate the string */
        {
            const char eos = '\0';
            BufAppend(&buf, &eos, sizeof(char));
        }

        *tok = (char*)buf.data;
        *text = p;
    }

    return 'I';
}

static int _Parse(
    const char* data,
    GRUBCommand** commandsOut,
    UINTN* numCommandsOut)
{
    int rc = -1;
    const char* p = data;
    StrArr line = STRARR_INITIALIZER;
    UINTN lineno = 1;
    GRUBCommand* commands = NULL;
    UINTN numCommands = 0;

    if (commandsOut)
        *commandsOut = NULL;

    if (numCommandsOut)
        *numCommandsOut = 0;

    /* Reject null parameters */
    if (!data || !commandsOut || !numCommandsOut)
        goto done;

    /* Parse the tokens */
    {
        int rc;
        const char* tok = NULL;

        while ((rc = _GetToken(&p, &tok)) != '\0')
        {
            if (rc == '\n' || rc == ';')
            {
                if (line.size != 0)
                {
                    GRUBCommand cmd;

                    cmd.line = lineno;
                    cmd.argc = line.size;
                    cmd.argv = (char**)line.data;

                    if (!(commands = (GRUBCommand*)Realloc(
                        commands,
                        sizeof(GRUBCommand) * numCommands,
                        sizeof(GRUBCommand) * (numCommands + 1))))
                    {
                        goto done;
                    }

                    Memcpy(&commands[numCommands++], &cmd, sizeof(cmd));
                    Memset(&line, 0, sizeof(line));
                }

                if (rc == '\n')
                    lineno++;
            }
            else
            {
                if (StrArrAppend(&line, tok) != 0)
                {
                    goto done;
                }
            }
        }
    }

    rc = 0;

done:

    StrArrRelease(&line);

    if (rc != 0)
    {
        /* ATTN: release here! */
    }
    else
    {
        *commandsOut = commands;
        *numCommandsOut = numCommands;
    }

    return rc;
}

INLINE void _DumpCommand(const GRUBCommand* cmd)
{
    UINTN i;

    PRINTF("=== GRUBCommand: line=%ld\n", (unsigned long)cmd->line);

    for (i = 0; i < cmd->argc; i++)
    {
        PRINTF("%s\n", cmd->argv[i]);
    }
}

INLINE void _Dump(
    const GRUBCommand* commands,
    UINTN numCommands)
{
    UINTN i;

    for (i = 0; i < numCommands; i++)
    {
        _DumpCommand(&commands[i]);
    }
}

static void _FreeCommand(GRUBCommand* cmd)
{
    UINTN i;

    for (i = 0; i < cmd->argc; i++)
    {
        Free(cmd->argv[i]);
    }

    Free(cmd->argv);
}

static void _Release(
    GRUBCommand* commands,
    UINTN numCommands)
{
    UINTN i;

    (void)_FreeCommand;

    for (i = 0; i < numCommands; i++)
    {
        _FreeCommand(&commands[i]);
    }

    Free(commands);
}

INLINE void _Indent(UINTN depth)
{
    UINTN i;
    for (i = 0; i < depth; i++)
    {
        PRINTF0("    ");
    }
}

typedef struct _Element
{
    UINTN index;
    const GRUBCommand* command;
    char initrd[PATH_MAX];
}
Element;

static BOOLEAN _Match(
    const StrArr* defaultValue,
    BOOLEAN hasDefault,
    Element* stack,
    UINTN top,
    char matched[PATH_MAX])
{
    UINTN i;
    UINTN matches = 0;

    if (defaultValue->size != top)
        return FALSE;

    *matched = '\0';

    for (i = 0; i < top; i++)
    {
        U64tostrBuf buf;
        const GRUBCommand* cmd = stack[i].command;

        if (!cmd || cmd->argc < 3)
            continue;

#if 0
        PRINTF("LHS{%s}\n", U64tostr(&buf, stack[i].index));
        PRINTF("RHS{%s}\n",  defaultValue->data[i]);
#endif

#if 0
        PRINTF("LHS{%s}\n", cmd->argv[cmd->argc-1]);
        PRINTF("RHS{%s}\n",  defaultValue->data[i]);
#endif

        if (!hasDefault)
        {
            Strlcat(matched, U64tostr(&buf, stack[i].index), PATH_MAX);
            matches++;
        }
        else if (Strcmp(U64tostr(&buf, stack[i].index), defaultValue->data[i]) == 0)
        {
            Strlcat(matched, defaultValue->data[i], PATH_MAX);
            matches++;
        }
        else if (Strcmp(cmd->argv[1], defaultValue->data[i]) == 0)
        {
            Strlcat(matched, defaultValue->data[i], PATH_MAX);
            matches++;
        }
        else if (Strcmp(cmd->argv[cmd->argc-2], defaultValue->data[i]) == 0)
        {
            Strlcat(matched, defaultValue->data[i], PATH_MAX);
            matches++;
        }

        if (matches != top)
            Strlcat(matched, ">", PATH_MAX);
    }

    if (matches != top)
        return FALSE;

    return TRUE;
}

static int _ResolveInitrd(
    const GRUBCommand* commands,
    UINTN numCommands,
    char matched[PATH_MAX],
    char title[PATH_MAX],
    char path[PATH_MAX])
{
    int rc = -1;
    UINTN i;
    const char* defaultValue = NULL;
    StrArr arr = STRARR_INITIALIZER;
    const UINTN STACK_SIZE = 16;
    Element stack[STACK_SIZE];
    UINT32 top = 0;
    BOOLEAN hasDefault = TRUE;

    if (path)
        *path = '\0';

    /* Check parameters */
    if (!commands || !numCommands || !path)
        goto done;

    /* Clear the stack */
    Memset(stack, 0, sizeof(stack));

    /* Look for "set default=value", where value is not a variable */
    for (i = 0; i < numCommands; i++)
    {
        const GRUBCommand* cmd = &commands[i];

        if (cmd->argc < 2)
            continue;

        if (Strcmp(cmd->argv[0], "set") == 0 &&
            Strncmp(cmd->argv[1], "default=", 8) == 0 &&
            cmd->argv[1][8] != '$')
        {
            defaultValue = cmd->argv[1] + 8;
            break;
        }
    }

    /* If not found, then fallback on the "0" menu entry */
    if (!defaultValue)
    {
        defaultValue = "0";
        hasDefault = FALSE;
    }

    /* Split 'default' value into parts */
    if (StrSplit(defaultValue, ">", &arr) != 0 || arr.size < 1)
        goto done;

#if defined(DEBUG_GRUBCFG)
    for (i = 0; i < arr.size; i++)
    {
        PRINTF("DEFAULT{%s}\n", arr.data[i]);
    }
#endif

    /* Look for matching 'menuentry' */
    for (i = 0; i < numCommands; i++)
    {
        const GRUBCommand* cmd = &commands[i];

        if (Strcmp(cmd->argv[0], "menuentry") == 0)
        {
#if defined(DEBUG_GRUBCFG)
            _Indent(top);
            PRINTF0("menuentry\n");
            _Indent(top);
            PRINTF0("{\n");
#endif

            /* Check for stack overflow */
            if (top == STACK_SIZE)
                goto done;

            /* Push element onto stack */
            stack[top].command = cmd;
            top++;
        }
        else if (Strcmp(cmd->argv[0], "submenu") == 0)
        {
#if defined(DEBUG_GRUBCFG)
            _Indent(top);
            PRINTF0("submenu\n");
            _Indent(top);
            PRINTF0("{\n");
#endif

            /* Check for stack overflow */
            if (top == STACK_SIZE)
                goto done;

            /* Push element onto stack */
            stack[top].command = cmd;
            top++;
        }
        else if (Strcmp(cmd->argv[0], "initrd") == 0 && cmd->argc == 2)
        {
            if (top && Strcmp(stack[top-1].command->argv[0], "menuentry") == 0)
                Strcpy(stack[top-1].initrd, cmd->argv[1]);
        }
        else if (Strcmp(cmd->argv[0], "initrdefi") == 0 && cmd->argc == 2)
        {
            if (top && Strcmp(stack[top-1].command->argv[0], "menuentry") == 0)
                Strcpy(stack[top-1].initrd, cmd->argv[1]);
        }
        else if (Strcmp(cmd->argv[0], "}") == 0)
        {
            if (top)
            {
                /* Reset the depth at next level */
                if (top != STACK_SIZE)
                    stack[top].index = 0;

                if (Strcmp(stack[top-1].command->argv[0], "menuentry") == 0)
                {
                    if (_Match(&arr, hasDefault, stack, top, matched))
                    {
                        Strcpy(title, stack[top-1].command->argv[1]);
                        Strcpy(path, stack[top-1].initrd);
                        rc = 0;
                        goto done;
                        PRINTF0("MATCH!!!\n");
                    }
#if defined(DEBUG_GRUBCFG)
{
UINTN j;
                    _Indent(top);
                    for (j = 0; j < top; j++)
                    {
                        PRINTF("%lu>", (unsigned long)stack[j].index);
                    }
                    PRINTF(": %s\n", stack[top-1].initrd);
}
#endif
                }

                /* Pop the stack */
                top--;
                stack[top].initrd[0] = '\0';
                stack[top].index++;

#if defined(DEBUG_GRUBCFG)
                _Indent(top);
                PRINTF0("}\n");
#endif

            }
        }

        (void)cmd;
    }

    rc = 0;

done:

    StrArrRelease(&arr);

    return rc;
}

int GRUBCfgFindInitrd(
    const char* dataIn,
    UINTN size,
    char matched[PATH_MAX],
    char title[PATH_MAX],
    char path[PATH_MAX])
{
    int rc = -1;
    GRUBCommand* commands = NULL;
    UINTN numCommands = 0;
    char* data = NULL;

    if (matched)
        *matched = '\0';

    if (title)
        *title = '\0';

    /* Reject for null parameters */
    if (!dataIn || !path || !matched || !title)
        goto done;

    /* Make a zero-terminated copy of the data */
    {
        if (!(data = (char*)Malloc(size + sizeof(char))))
            goto done;

        Memcpy(data, dataIn, size);

        data[size] = '\0';
    }

    /* Parse the file */
    if (_Parse(data, &commands, &numCommands) != 0)
        goto done;

#if 0
    _Dump(commands, numCommands);
#endif

    /* Find initrd */
    if (_ResolveInitrd(commands, numCommands, matched, title, path) != 0)
        goto done;

    rc = 0;

done:

    if (data)
        Free(data);

    if (commands)
        _Release(commands, numCommands);

    return rc;
}

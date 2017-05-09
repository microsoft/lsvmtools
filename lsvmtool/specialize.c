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
#include "specialize.h"
#include <errno.h>
#include <unistd.h>
#include <lsvmutils/file.h>
#include <lsvmutils/error.h>
#include <lsvmutils/conf.h>
#include <lsvmutils/alloc.h>
#include <lsvmutils/strarr.h>
#include <lsvmutils/exec.h>
#include <lsvmutils/pass.h>
#include <lsvmutils/buf.h>

#define MAX_USERS 128
#define IPV4_MAX_STR_SIZE 15 /* XXX.XXX.XXX.XXX */ 

static int _SplitFields(
    const char* str,
    StrArr* arr)
{
    int rc = 0;
    const char* p = str;
    const char* start = p;

    for (;;)
    {
        if (*p == ':' || *p == '\0')
        {
            char* field;

            if (!(field = Strndup(start, p - start)))
                goto done;

            if (StrArrAppendBorrow(arr, field) != 0)
            {
                Free(field);
                goto done;
            }

            if (!*p)
                break;

            start = p + 1;
        }

        p++;
    }

    rc = 0;

done:
    return rc;
}

INLINE void _DumpArgs(
    int argc,
    const char* argv[])
{
    UINTN i;

    for (i = 0; i < argc; i++)
    {
        printf("{%s}\n", argv[i]);
    }

    printf("\n");
}

static int _JoinFields(const StrArr* arr, Buf* buf)
{
    int rc = -1;
    UINTN i;

    /* Reject null parameters */
    if (!arr || !buf)
        goto done;

    /* Clear the data */
    BufRelease(buf);

    /* Write all the fields to the new file */
    for (i = 0; i < arr->size; i++)
    {
        BufAppend(buf, arr->data[i], Strlen(arr->data[i]));

        if (i + 1 == arr->size)
            BufAppend(buf, "\n", 1);
        else
            BufAppend(buf, " ", 1);
    }

    BufAppend(buf, "\0", 1);

    rc = 0;

done:
    return rc;
}

static int _ReadLines(
    const char* path,
    StrArr* lines)
{
    int rc = -1;
    FILE* is = NULL;
    char buf[1024];

    if (!path || !lines)
        goto done;

    if (!(is = fopen(path, "r")))
        goto done;

    StrArrRelease(lines);

    while (fgets(buf, sizeof(buf), is) != NULL)
    {
        if (StrArrAppend(lines, buf) != 0)
        {
            goto done;
        }
    }

    rc = 0;

done:

    if (is)
        fclose(is);

    return rc;
}

static int _WriteLines(
    const char* path,
    const StrArr* lines)
{
    int rc = -1;
    FILE* os = NULL;
    UINTN i;

    if (!path || !lines)
        goto done;

    if (!(os = fopen(path, "w")))
        goto done;

    for (i = 0; i < lines->size; i++)
    {
        if (fprintf(os, "%s", lines->data[i]) != strlen(lines->data[i]))
            goto done;
    }

    rc = 0;

done:

    if (os)
        fclose(os);

    return rc;
}

static int _Sethostname(
    const char* hostname,
    Error* err)
{
    int rc = -1;
    char oldhostname[256];
    StrArr arr = STRARR_INITIALIZER;
    StrArr lines = STRARR_INITIALIZER;
    FILE* is = NULL;
    FILE* os = NULL;

    /* Check parameters */
    if (!hostname || !err)
    {
        SetErr(err, "null parameter");
        goto done;
    }

    /* Get the old hostname */
    if (gethostname(oldhostname, sizeof(oldhostname)) != 0)
    {
        SetErr(err, "gethostname");
        goto done;
    }

    /* Set the new hostname */
    if (sethostname(hostname, Strlen(hostname)) != 0)
    {
        SetErr(err, "sethostname");
        goto done;
    }

    /* Update "/etc/hostname" if it exists */
    if (access("/etc/hostname", F_OK) == 0)
    {
        if ((is = fopen("/etc/hostname", "w")) == NULL)
        {
            SetErr(err, "failed to open /etc/hostname");
            goto done;
        }

        if (fprintf(is, "%s\n", hostname) != Strlen(hostname) + 1)
        {
            SetErr(err, "failed to write /etc/hostname");
            goto done;
        }

        fclose(is);
        is = NULL;
    }

    /* Update "/etc/hosts" file */
    if (access("/etc/hosts", F_OK) == 0)
    {
        /* Read lines of this file into memory */
        if (_ReadLines("/etc/hosts", &lines) != 0)
        {
            SetErr(err, "failed to read /etc/hosts");
            goto done;
        }

        /* Patch this file by replacing old hostname with new hostname */
        {
            UINTN i;

            if ((os = fopen("/etc/hosts", "w")) == NULL)
            {
                SetErr(err, "failed to open /etc/hosts");
                goto done;
            }

            for (i = 0; i < lines.size; i++)
            {
                const char* line = lines.data[i];
                UINTN j;

                /* Copy comments directly to new file */
                if (line[0] == '#')
                {
                    fprintf(os, "%s", line);
                    continue;
                }

                /* Split line into fields */
                if (StrSplit(line, " \n\t", &arr) != 0)
                {
                    SetErr(err, "failed to split string");
                    goto done;
                }

                /* Copy empty lines to output */
                if (arr.size == 0)
                {
                    fprintf(os, "\n");
                    continue;
                }

                /* Write all the fields to the new file */
                for (j = 0; j < arr.size; j++)
                {
                    const char* field;

                    /* Translate the old hostname to new hostname */
                    if (j == 1 && Strcmp(arr.data[j], oldhostname) == 0)
                        field = hostname;
                    else
                        field = arr.data[j];

                    if (fprintf(os, "%s", field) != Strlen(field))
                    {
                        SetErr(err, "failed to write /etc/hosts.new");
                        goto done;
                    }

                    if (j + 1 == arr.size)
                        fprintf(os, "\n");
                    else
                        fprintf(os, " ");
                }
            }
        }
    }

/* 
ATTN: copy hosts.new to hosts 
*/

    rc = 0;

done:

    if (is)
        fclose(is);

    if (os)
        fclose(os);

    StrArrRelease(&lines);
    StrArrRelease(&arr);

    return rc;
}

static int _Useradd(
    const char* username,
    const char* uid,
    const char* password,
    const char* group,
    Error* err)
{
    int rc = -1;
    const char* argv[16];
    int argc = 0;
    int status;
    char encpass[256];

    /* Check parameters */
    if (!username || !uid || !password || !group || !err)
    {
        SetErr(err, "null parameter");
        goto done;
    }

    /* Encrypt the password */
    if (*password)
    {
        if (CryptPassword(encpass, sizeof(encpass), password) != 0)
        {
            SetErr(err, "failed to encrypt password");
            goto done;
        }
    }

    /* Build argument list */
    {
        argv[argc++] = "/usr/sbin/useradd";
        argv[argc++] = username;

        if (*uid)
        {
            argv[argc++] = "-u";
            argv[argc++] = uid;
        }

        if (*password)
        {
            argv[argc++] = "-p";
            argv[argc++] = encpass;
        }

        if (*group)
        {
            argv[argc++] = "-g";
            argv[argc++] = group;
        }

        /* Create the home directory */
        argv[argc++] = "-m";

        argv[argc] = NULL;
    }

    /* Execute program */
    if (Exec(argc, argv, &status, err) != 0)
        goto done;

    if (status != 0)
    {
        SetErr(err, "bad status: %d", status);
        goto done;
    }

    rc = 0;

done:
    return rc;
}

static int _Userdel(
    const char* username,
    Error* err)
{
    int rc = -1;
    const char* argv[16];
    int argc = 0;
    int status;

    /* Check parameters */
    if (!username || !err)
    {
        SetErr(err, "null parameter");
        goto done;
    }

    /* Build argument list */
    argv[argc++] = "/usr/sbin/userdel";
    argv[argc++] = username;
    argv[argc] = NULL;

    /* Execute program */
    if (Exec(argc, argv, &status, err) != 0)
        goto done;

    if (status != 0)
    {
        SetErr(err, "bad status: %d", status);
        goto done;
    }

    rc = 0;

done:
    return rc;
}

static int _Groupadd(
    const char* group,
    const char* gid,
    Error* err)
{
    int rc = -1;
    const char* argv[16];
    int argc = 0;
    int status;

    /* Check parameters */
    if (!group || !gid || !err)
    {
        SetErr(err, "null parameter");
        goto done;
    }

    /* Build argument list */
    {
        argv[argc++] = "/usr/sbin/groupadd";
        argv[argc++] = group;

        if (*gid)
        {
            argv[argc++] = "-g";
            argv[argc++] = gid;
        }

        argv[argc] = NULL;
    }

    /* Execute program */
    if (Exec(argc, argv, &status, err) != 0)
        goto done;

    if (status != 0)
    {
        SetErr(err, "bad status: %d", status);
        goto done;
    }

    rc = 0;

done:
    return rc;
}

static int _Groupdel(
    const char* group,
    Error* err)
{
    int rc = -1;
    const char* argv[16];
    int argc = 0;
    int status;

    /* Check parameters */
    if (!group || !err)
    {
        SetErr(err, "null parameter");
        goto done;
    }

    /* Build argument list */
    argv[argc++] = "/usr/sbin/groupdel";
    argv[argc++] = group;
    argv[argc] = NULL;

    /* Execute program */
    if (Exec(argc, argv, &status, err) != 0)
        goto done;

    if (status != 0)
    {
        SetErr(err, "bad status: %d", status);
        goto done;
    }

    rc = 0;

done:
    return rc;
}

static int _Chpasswd(
    const char* username,
    const char* password,
    Error* err)
{
    int rc = -1;
    const char* argv[16];
    int argc = 0;
    int status;
    Buf buf = BUF_INITIALIZER;

    /* Check parameters */
    if (!username || !password || !err)
    {
        SetErr(err, "null parameter");
        goto done;
    }

    /* Build argument list */
    argv[argc++] = "/usr/sbin/chpasswd";
    argv[argc] = NULL;

    /* Format the standard input: "username:password" */
    BufAppend(&buf, username, Strlen(username));
    BufAppend(&buf, ":", 1);
    BufAppend(&buf, password, Strlen(password));

    printf("STDIN{%.*s}\n", (int)buf.size, (char*)buf.data);

    /* Execute program */
    if (ExecWithStdin(buf.data, buf.size, argc, argv, &status, err) != 0)
        goto done;

    if (status != 0)
    {
        SetErr(err, "bad status: %d", status);
        goto done;
    }

    rc = 0;

done:
    return rc;
}

static int _SetnetworkUbuntu(
    const char* address,
    const char* netmask,
    const char* gateway,
    Error* err)
{
    int rc = -1;
    StrArr arr = STRARR_INITIALIZER;
    StrArr lines = STRARR_INITIALIZER;
    Buf buf = BUF_INITIALIZER;
    FILE* os = NULL;
    const char infile[] = "/etc/network/interfaces";
    const char outfile[] = "/etc/network/interfaces";

    /* Check parameters */
    if (!address || !netmask || !gateway || !err)
    {
        SetErr(err, "null parameter");
        goto done;
    }

    /* Fail if this file does not exist */
    if (access(infile, F_OK) != 0)
    {
        SetErr(err, "failed to open %s", infile);
        goto done;
    }

    /* Read lines of this file into memory */
    if (_ReadLines(infile, &lines) != 0)
    {
        SetErr(err, "failed to read %s", infile);
        goto done;
    }

    /* Open the new file */
    if (!(os = fopen(outfile, "w")))
    {
        SetErr(err, "failed to open %s", outfile);
        goto done;
    }

    /* Trim lines that refer to "lo" and "eth0" interfaces */
    {
        UINTN i;

        /* For each line */
        for (i = 0; i < lines.size; )
        {
            const char* line = lines.data[i];

            /* Remove comment lines */
            if (line[0] == '#')
            {
                StrArrRemove(&lines, i);
                continue;
            }

            /* Split line into fields */
            if (StrSplit(line, " \n\t", &arr) != 0)
            {
                SetErr(err, "failed to split string");
                goto done;
            }

            /* Remove empty lines */
            if (arr.size == 0)
            {
                StrArrRemove(&lines, i);
                continue;
            }

            /* Remove "auto lo" */
            if (arr.size >= 2 && 
                Strcmp(arr.data[0], "auto") == 0 &&
                Strcmp(arr.data[1], "lo") == 0)
            {
                StrArrRemove(&lines, i);
                continue;
            }

            /* Remove "iface lo" */
            if (arr.size >= 2 && 
                Strcmp(arr.data[0], "iface") == 0 &&
                Strcmp(arr.data[1], "lo") == 0)
            {
                StrArrRemove(&lines, i);
                continue;
            }

            /* Remove "auto eth0" */
            if (arr.size >= 2 && 
                Strcmp(arr.data[0], "auto") == 0 &&
                Strcmp(arr.data[1], "eth0") == 0)
            {
                StrArrRemove(&lines, i);
                continue;
            }

            /* Remove "iface eth0" */
            if (arr.size >= 2 && 
                Strcmp(arr.data[0], "iface") == 0 &&
                Strcmp(arr.data[1], "eth0") == 0)
            {
                StrArrRemove(&lines, i);
                continue;
            }

            /* Remove "address" */
            if (arr.size >= 1 && 
                Strcmp(arr.data[0], "address") == 0)
            {
                StrArrRemove(&lines, i);
                continue;
            }

            /* Remove "netmask" */
            if (arr.size >= 1 && 
                Strcmp(arr.data[0], "netmask") == 0)
            {
                StrArrRemove(&lines, i);
                continue;
            }

            /* Remove "gateway" */
            if (arr.size >= 1 && 
                Strcmp(arr.data[0], "gateway") == 0)
            {
                StrArrRemove(&lines, i);
                continue;
            }

            i++;
        }
    }

    /* Append "auto lo" */
    if (StrArrAppend(&lines, "auto lo\n") != 0)
    {
        SetErr(err, "out of memory");
        goto done;
    }

    /* Append "iface lo inet loopback" */
    if (StrArrAppend(&lines, "iface lo inet loopback\n") != 0)
    {
        SetErr(err, "out of memory");
        goto done;
    }

    /* Append "auto eth0" */
    if (StrArrAppend(&lines, "auto eth0\n") != 0)
    {
        SetErr(err, "out of memory");
        goto done;
    }

    /* Append new lines */
    if (*address)
    {
        if (StrArrAppend(&lines, "iface eth0 inet static\n") != 0)
        {
            SetErr(err, "out of memory");
            goto done;
        }

        StrArrRelease(&arr);

        if (StrArrAppend(&arr, "address") != 0)
        {
            SetErr(err, "out of memory");
            goto done;
        }

        if (StrArrAppend(&arr, (char*)address) != 0)
        {
            SetErr(err, "out of memory");
            goto done;
        }

        if (_JoinFields(&arr, &buf) != 0)
        {
            SetErr(err, "out of memory");
            goto done;
        }

        if (StrArrAppend(&lines, buf.data) != 0)
        {
            SetErr(err, "out of memory");
            goto done;
        }
    }
    else
    {
        if (StrArrAppend(&lines, "iface eth0 inet dhcp\n") != 0)
        {
            SetErr(err, "out of memory");
            goto done;
        }
    }

    if (*netmask)
    {
        StrArrRelease(&arr);

        if (StrArrAppend(&arr, "netmask") != 0)
        {
            SetErr(err, "out of memory");
            goto done;
        }

        if (StrArrAppend(&arr, (char*)netmask) != 0)
        {
            SetErr(err, "out of memory");
            goto done;
        }

        if (_JoinFields(&arr, &buf) != 0)
        {
            SetErr(err, "out of memory");
            goto done;
        }

        if (StrArrAppend(&lines, buf.data) != 0)
        {
            SetErr(err, "out of memory");
            goto done;
        }
    }

    if (*gateway)
    {
        StrArrRelease(&arr);

        if (StrArrAppend(&arr, "gateway") != 0)
        {
            SetErr(err, "out of memory");
            goto done;
        }

        if (StrArrAppend(&arr, (char*)gateway) != 0)
        {
            SetErr(err, "out of memory");
            goto done;
        }

        if (_JoinFields(&arr, &buf) != 0)
        {
            SetErr(err, "out of memory");
            goto done;
        }

        if (StrArrAppend(&lines, buf.data) != 0)
        {
            SetErr(err, "out of memory");
            goto done;
        }
    }

    /* Write these lines to 'outfile' */
    if (_WriteLines(outfile, &lines) != 0)
    {
        SetErr(err, "failed to write %s", outfile);
        goto done;
    }

    rc = 0;

done:

    if (os)
        fclose(os);

    StrArrRelease(&lines);
    StrArrRelease(&arr);
    BufRelease(&buf);

    return rc;
}

static int _SetnetworkOther(
    const char* address,
    const char* netmask,
    const char* gateway,
    const char* file,
    Error* err)
{
    int rc = -1;
    StrArr lines = STRARR_INITIALIZER; 
    char buf[32]; 

    if (!address || !netmask || !gateway || !err)
    {
        SetErr(err, "null parameter");
        goto done;
    }
    
    if (access(file, F_OK) != 0)
    {
        if (errno != ENOENT)
        {
            /* Some error aside from file does not exist. */
            SetErr(err, "error accessing existing network file");
            goto done;
        }
        
        /*
         * Remaining case is that the file doesn't exist. In this case, we don't do
         * anything. The next part will write in the proper K=V values.
         */
    }
    else
    {
        /* File exists, so read into memory. */
        if (_ReadLines(file, &lines) != 0)
        {
            SetErr(err, "failed to read %s", file);
            goto done;
        }
    }

    /* 
     * Trim lines that appear for:
     *  - DEVICE
     *  - ONBOOT
     *  - BOOTPROTO
     *  - IPADDR, NETMASK, GATEWAY
     */   
    {
        UINTN i;

        for (i = 0; i < lines.size; )
        {
            const char *line = lines.data[i];
 
            if (strstr(line, "DEVICE") == line)
            {
                StrArrRemove(&lines, i);
                continue;
            }
            else if (strstr(line, "ONBOOT") == line)
            {
                StrArrRemove(&lines, i);
                continue;
            }
            else if (strstr(line, "BOOTPROTO") == line)
            {
                StrArrRemove(&lines, i);
                continue;
            }
            else if (strstr(line, "IPADDR") == line)
            {
                StrArrRemove(&lines, i);
                continue;
            }
            else if (strstr(line, "NETMASK") == line)
            {
                StrArrRemove(&lines, i);
                continue;
            }
            else if (strstr(line, "GATEWAY") == line)
            {
                StrArrRemove(&lines, i);
                continue;
            }

            i++;
        }
    }    
 
    /* Append DEVICE and ONBOOT */
    if (StrArrAppend(&lines, "DEVICE=eth0\n") != 0)
        goto out_of_memory;

    if (StrArrAppend(&lines, "ONBOOT=yes\n") != 0)
        goto out_of_memory;

    /* Set static ip or dhcp */
    if (*address)
    {
        if (StrArrAppend(&lines, "BOOTPROTO=static\n") != 0)
            goto out_of_memory;
       
        rc = snprintf(buf, sizeof(buf), "IPADDR=%s\n", address);
        if (rc < 0 || rc >= sizeof(buf))
        {
            rc = -1;
            SetErr(err, "invalid Ipv4 address");
            goto done;
        }

        if (StrArrAppend(&lines, buf) != 0)
            goto out_of_memory;
    }
    else
    {
        if (StrArrAppend(&lines, "BOOTPROTO=dhcp\n") != 0)
            goto out_of_memory;
    }

    /* Set netmask */
    if (*netmask)
    {
        rc = snprintf(buf, sizeof(buf), "NETMASK=%s\n", netmask);
        if (rc < 0 || rc >= sizeof(buf))
        {
            rc = -1;
            SetErr(err, "invalid Ipv4 netmask");
            goto done;
        }
     
        if (StrArrAppend(&lines, buf) != 0)
            goto out_of_memory;
    }

    /* Set gateway */
    if (*gateway)
    {
        rc = snprintf(buf, sizeof(buf), "GATEWAY=%s\n", gateway);
        if (rc < 0 || rc >= sizeof(buf))
        {
            rc = -1;
            SetErr(err, "invalid Ipv4 netmask");
            goto done;
        }
     
        if (StrArrAppend(&lines, buf) != 0)
            goto out_of_memory;
    }

    /* Write the output to the ethernet file. */
    if (_WriteLines(file, &lines) != 0)
    {
        SetErr(err, "failed to write %s", file);
        goto done;
    }

    rc = 0;
    goto done;

out_of_memory:
    rc = -1;
    SetErr(err, "out of memory");

done:
    StrArrRelease(&lines);
    return rc;
}


static int _Setnetwork(
    const char* address,
    const char* netmask,
    const char* gateway,
    Error* err)
{
    int rc = -1;

    /* Check parameters */
    if (!address || !netmask || !gateway || !err)
    {
        SetErr(err, "null parameter");
        goto done;
    }

    /* Ubuntu */
    if (access("/etc/network/interfaces", F_OK) == 0)
    {
        return _SetnetworkUbuntu(address, netmask, gateway, err);
    } 
    
    /* CentOS */
    if (access("/etc/sysconfig/network-scripts", F_OK) == 0)
    {
        return _SetnetworkOther(address, netmask, gateway, "/etc/sysconfig/network-scripts/ifcfg-eth0", err);
    } 

    /* SUSE */
    if (access("/etc/sysconfig/network", F_OK) == 0)
    {
        return _SetnetworkOther(address, netmask, gateway, "/etc/sysconfig/network/ifcfg-eth0", err);
    }

    /* Unsupported distro */
    SetErr(err, "unsupported distro");
    rc = -1;

done:
    return rc;
}

static int _confCallback(
    const char* name, 
    const char* value,
    void* callbackData,
    Error* err)
{
    int rc = -1;
    StrArr arr = STRARR_INITIALIZER;

    /* Check for null parameters */
    if (!name || !value)
        goto done;

    /* Print status message */
    printf("Processing command: %s\n", name);

    /* Split the value into fields */
    if (_SplitFields(value, &arr) != 0)
    {
        SetErr(err, "failed to split fields");
        goto done;
    }

    /* Handle this command */
    if (Strcmp(name, "sethostname") == 0)
    {
        /* Expected four fields */
        if (arr.size != 1)
        {
            SetErr(err, "wrong number of fields");
            goto done;
        }

        /* Add the user */
        if (_Sethostname(
            arr.data[0], /* hostname */
            err) != 0)
        {
            goto done;
        }
    }
    else if (Strcmp(name, "useradd") == 0)
    {
        /* Expected four fields */
        if (arr.size != 4)
        {
            SetErr(err, "wrong number of fields");
            goto done;
        }

        /* Add the user */
        if (_Useradd(
            arr.data[0], /* username */
            arr.data[1], /* uid */
            arr.data[2], /* password */
            arr.data[3], /* group */
            err) != 0)
        {
            goto done;
        }
    }
    else if (Strcmp(name, "userdel") == 0)
    {
        /* Expected four fields */
        if (arr.size != 1)
        {
            SetErr(err, "wrong number of fields");
            goto done;
        }

        /* Add the user */
        if (_Userdel(
            arr.data[0], /* username */
            err) != 0)
        {
            goto done;
        }
    }
    else if (Strcmp(name, "groupadd") == 0)
    {
        /* Expected four fields */
        if (arr.size != 2)
        {
            SetErr(err, "wrong number of fields");
            goto done;
        }

        /* Add the group */
        if (_Groupadd(
            arr.data[0], /* group */
            arr.data[1], /* gid */
            err) != 0)
        {
            goto done;
        }
    }
    else if (Strcmp(name, "groupdel") == 0)
    {
        /* Expected four fields */
        if (arr.size != 1)
        {
            SetErr(err, "wrong number of fields");
            goto done;
        }

        /* Add the group */
        if (_Groupdel(
            arr.data[0], /* group */
            err) != 0)
        {
            goto done;
        }
    }
    else if (Strcmp(name, "chpasswd") == 0)
    {
        /* Expected four fields */
        if (arr.size != 2)
        {
            SetErr(err, "wrong number of fields");
            goto done;
        }

        /* Add the group */
        if (_Chpasswd(
            arr.data[0], /* username */
            arr.data[1], /* password */
            err) != 0)
        {
            goto done;
        }
    }
    else if (Strcmp(name, "setnetwork") == 0)
    {
        /* Expected four fields */
        if (arr.size != 3)
        {
            SetErr(err, "wrong number of fields");
            goto done;
        }

        /* Add the group */
        if (_Setnetwork(
            arr.data[0], /* address */
            arr.data[1], /* netmask */
            arr.data[2], /* gateway */
            err) != 0)
        {
            SetErr(err, "failed to set network");
            goto done;
        }
    }
    else
    {
        SetErr(err, "unknown option: %s", name);
        goto done;
    }

    rc = 0;

done:

    StrArrRelease(&arr);

    return rc;
}


static BOOLEAN _CheckName(
    const CHAR16* data,
    UINT32 numElem,
    const char* expected)
{
    UINT32 i;

    if (Strlen(expected) != numElem)
        return FALSE;

    for (i = 0; i < numElem; i++)
    {
        if (data[i] != (CHAR16) expected[i])
            return FALSE;
    }
    return TRUE;
}

static int _ParseSpecFiles(
    SPECIALIZATION_CLEAR_DATA_HEADER* hdr,
    const unsigned char* data,
    size_t size,
    Error* err)
{
    const unsigned char* dataCur;
    UINT32 i;
    unsigned int errorLine;
    int rc = -1;

    dataCur = data;
    for (i = 0; i < hdr->FileCount; i++)
    {
        SPECIALIZATION_CLEAR_DATA_FILE_ENTRY* entry;
        UINT32 max;
        BOOLEAN isSpecFile = FALSE;
	
        /* First validate inputs. */
        if (dataCur + SPECIALIZATION_CLEAR_DATA_FILE_ENTRY_SIZE > data + size)
        {
            SetErr(err, "Invalid spec file size");
            goto done;
        }

        /* Right now, we only check the unattend or autounattend file. */
        entry = (SPECIALIZATION_CLEAR_DATA_FILE_ENTRY*) dataCur; 

        isSpecFile = _CheckName(
                         (CHAR16*) (dataCur + entry->FileNameOffset),
                         entry->FileNameSize / sizeof(CHAR16),
                         SPEC_UNATTEND_FILENAME);

        isSpecFile = isSpecFile ||
                     _CheckName(
                         (CHAR16*) (dataCur + entry->FileNameOffset),
                         entry->FileNameSize / sizeof(CHAR16),
                         SPEC_UNATTEND_FILENAME_ALTERNATE);

        /* Found the specialization file. Now, parse + execute. */
        if (isSpecFile)
        {
            if (ParseConf(
                (const char*) (dataCur + entry->FilePayloadOffset),
                entry->FilePayloadSize,
                _confCallback,
                NULL,
                &errorLine,
                err) != 0)
            {
                SetErr(err, "(%u): %s", errorLine, err->buf);
                goto done;
            }
            break;
        }

        /* Advance to next entry. */
        max = entry->FilePayloadOffset + entry->FilePayloadSize;
        if (max < entry->FileNameOffset + entry->FileNameSize)
        {
            max = entry->FileNameOffset + entry->FileNameSize;
        }
        dataCur += max;
    }
    rc = 0;

done:
    return rc;
}

int Specialize(
    const char* path,
    Error* err)
{
    int rc = -1;
    unsigned char* data = NULL;
    size_t size;
    SPECIALIZATION_CLEAR_DATA_HEADER* hdr;

    /* Reject null parameters */
    if (!path || !err)
        goto done;

    /* Clear the error */
    ClearErr(err);

    /* Read the file into memory */
    if (LoadFile(path, 1, &data, &size) != 0)
    {
        SetErr(err, "failed to read file: %s", path);
        goto done;
    }

    /* Check size of data */
    if (size < sizeof(SPECIALIZATION_CLEAR_DATA_HEADER))
    {
        SetErr(err, "spec file is too small: %zu", size);
        goto done;
    }

    /* Now parse the spec files. */
    hdr = (SPECIALIZATION_CLEAR_DATA_HEADER*) data;
    if (_ParseSpecFiles(hdr, data + sizeof(*hdr), size - sizeof(*hdr), err) != 0)
    {
        SetErr(err, "%s: %s", path, err->buf);
        goto done;
    }

    rc = 0;

done:

    if (data)
        Free(data);

    return rc;
}

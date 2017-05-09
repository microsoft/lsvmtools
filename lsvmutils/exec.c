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
#include "exec.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/wait.h> 

int Exec(
    int argc, 
    const char* argv[],
    int* status,
    Error* err)
{
    pid_t pid;
    int rc = -1;

    /* Initialize status */
    if (status)
        *status = 0;

    /* Reject null parameters */
    if (!argc || !argc || !status)
    {
        SetErr(err, "null arg");
        goto done;
    }

    /* Create a child to execute argv[0] */
    if ((pid = fork()) < 0)
    {
        SetErr(err, "fork() failed");
        goto done;
    }

    if (pid > 0) /* Parent */
    {
        int tmp;

        /* Wait for child to finish */
        if (wait(&tmp) != pid)
        {
            SetErr(err, "wait() failed");
            goto done;
        }

        /* If status is bad */
        *status = WEXITSTATUS(tmp);
    }
    else if (pid == 0) /* Child */
    {
        execv(argv[0], (char**)argv);
        exit(1);
    }

    rc = 0;

done:
    return rc;
}

int ExecWithStdin(
    const void* data, /* write this to standard input of child */
    size_t size, /* size of data */
    int argc,
    const char* argv[],
    int* status,
    Error* err)
{
    pid_t pid;
    int fd[2];
    int rc = 0;

    /* Initialize output status parameter */
    if (status)
        *status = 0;

    if (!data || !argv || !status || !err)
        return -1;

    /* Create a pipe */
    if (pipe(fd) != 0)
    {
	SetErr(err, "pipe() failed\n");
	rc = 1;
	goto done;
    }

    /* Create a child to execute cryptsetup */
    if ((pid = fork()) < 0)
    {
	SetErr(err, "fork() failed\n");
	rc = 1;
	goto done;
    }

    if (pid > 0) /* Parent */
    {
	int tmp;

	close(fd[0]);

	if (write(fd[1], data, size) != size)
	{
	    SetErr(err, "write() failed\n");
	    goto done;
	}

	close(fd[1]);

	if (wait(&tmp) != pid)
	{
	    SetErr(err, "wait() failed\n");
	    goto done;
	}

        *status = WEXITSTATUS(tmp);
    }
    else if (pid == 0) /* Child */
    {
	dup2(fd[0], 0);
	close(fd[1]);
	execv(argv[0], (char**)argv);
	exit(1);
    }

done:
    return rc;
}

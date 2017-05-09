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
#include <lsvmutils/buf.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <inttypes.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

#ifdef DEBUG
#define PRINTLOG printf("%s(%d): %s(): %s\n", __FILE__, __LINE__, __FUNCTION__, strerror(errno)); 
#else
#define PRINTLOG
#endif

#define GOTO(LABEL) \
do \
{ \
    PRINTLOG \
    goto LABEL; \
} while (0)

static const char *WATCH_DIR = "/run/systemd/ask-password/";
static const char *PASS_PREFIX = "ask."; 
static const char *INI_SECTION = "[Ask]\n";
static const char *INI_MESSAGE = "Message=";
static const char *INI_SOCKET = "Socket=";
static const char *BOOT_DRIVE_NAME = "boot";

static void dump_inotify_event(struct inotify_event *event)
{
#ifdef DEBUG
    printf("wd: %d\n", event->wd);
    printf("mask: %" PRIu32 "\n", event->mask);
    printf("cookie: %" PRIu32 "\n", event->cookie);
    printf("len: %" PRIu32 "\n", event->len);
    printf("name: %s\n", event->name);
#endif
}

static char *retrieve_password(int is_boot, const char *bootkey, const char *rootkey)
{
    char *password = NULL;
    const char *keyname = NULL;
    struct stat file_info;
    int pass_fd = -1;

    keyname = is_boot ? bootkey : rootkey;
    
    if (stat(keyname, &file_info) != 0)
        GOTO(err);

    password = (char*) malloc(file_info.st_size + 1);
    if (password == NULL)
        GOTO(err);

    pass_fd = open(keyname, O_RDONLY);
    if (pass_fd == -1)
        GOTO(err);

    if (read(pass_fd, password, file_info.st_size) != file_info.st_size)
        GOTO(err); 
    
    password[file_info.st_size] = '\0';
    goto done;

err:
    if (password)
        free(password);
    password = NULL;

done: 
    if (pass_fd != -1)
        close(pass_fd);
    return password;
}

static int send_password(const char *pass, const char *socket_name)
{
    int sock_fd = -1;
    char *packet = NULL; 
    ssize_t packet_length = 0;
    struct sockaddr_un addr;
    int err = -1;

    /* Create the password response packet. */
    packet_length = strlen(pass) + 2;
    packet = malloc(packet_length);
    if (packet == NULL)
        GOTO(done);

    packet[0] = '+';
    printf("%s\n", pass);
    memcpy(packet+1, pass, packet_length-1);

    /* Send the packet. */
    sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock_fd == -1)
        GOTO(done);

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_name, sizeof(addr.sun_path));

    if (sendto(sock_fd, packet, packet_length, 0, (struct sockaddr*) &addr, sizeof(struct sockaddr_un)) < 0)
        GOTO(done);

    err = 0;

done:    
    if (packet)
        free(packet);
    if (sock_fd != -1)
        close(sock_fd);
    return err;
}

static int search_for_ask(FILE *fp)
{
    char *lineptr = NULL;
    size_t n = 0;        
    int err = 0;

    do 
    {
        if (getline(&lineptr, &n, fp) == -1)
        {
            err = -1;
            break;
        }
    } while (strcmp(lineptr, INI_SECTION) != 0);

    free(lineptr);
    return err;
}

static int parse_ask_section(FILE *fp, int *is_boot, char **socket_name)
{
    char *lineptr = NULL;
    size_t n = 0;
    char *tmp_socket_name = NULL;

    while (1)
    {
        ssize_t bytes_read;
        if ((bytes_read = getline(&lineptr, &n, fp)) == -1)
            break;

        if (strncmp(lineptr, INI_MESSAGE, strlen(INI_MESSAGE)) == 0)
		    printf("ALERT: %s", lineptr);

        if (strncmp(lineptr, INI_MESSAGE, strlen(INI_MESSAGE)) == 0
            && strstr(lineptr, BOOT_DRIVE_NAME) != NULL)
        {
            *is_boot = 1;
        }

        if (strncmp(lineptr, INI_SOCKET, strlen(INI_SOCKET)) == 0)
        {
            size_t off = strlen(INI_SOCKET);
            size_t size = bytes_read - off;

            /* Should have only 1 socket field. */
            if (tmp_socket_name != NULL)
                break;

            tmp_socket_name = (char*) malloc(size);
            if (!tmp_socket_name)
                break;

            memcpy(tmp_socket_name, lineptr + off, size);

            /* Change ending \n to a NULL. */
            tmp_socket_name[size-1] = '\0';
        }
    }
    
    free(lineptr);
    
    if (!feof(fp))
    {
        free(tmp_socket_name);
        return -1;
    }

    *socket_name = tmp_socket_name;
    return 0;
}

static int handle_and_send_pass(const char *socket_name, int is_boot, const char *bootkey, const char *rootkey)
{
    char *pass = NULL;
    int err = -1;

    pass = retrieve_password(is_boot, bootkey, rootkey);
    if (pass == NULL)
        return err;

    err = send_password(pass, socket_name);
    free(pass);
    return err;
}

static int process_password_request(struct inotify_event *event, const char *bootkey, const char *rootkey)
{
    FILE *fp = NULL;
    char *path = NULL;
    int err = -1;
    char *socket_name = NULL;
    int is_boot = 0;

    dump_inotify_event(event);

    /* Ignore anything that isn't ask.XXXX */
    if (strncmp(event->name, PASS_PREFIX, strlen(PASS_PREFIX)) != 0)
        return 0;

    /* Create the path to file and open it. */
    path = (char*) malloc(strlen(WATCH_DIR) + event->len);
    if (!path)
        GOTO(done);    
    memcpy(path, WATCH_DIR, strlen(WATCH_DIR));
    memcpy(path + strlen(WATCH_DIR), event->name, event->len);

    fp = fopen(path, "r");
    if (!fp)
        GOTO(done);  

    /* Save Socket= Field. */ 
    err = search_for_ask(fp);
    if (err)
        GOTO(done);

    err = parse_ask_section(fp, &is_boot, &socket_name);
    if (err || socket_name == NULL)
        GOTO(done);

    /* Finally, execute the unseal and send the password. */
    err = handle_and_send_pass(socket_name, is_boot, bootkey, rootkey);
    if (err)
        GOTO(done);

done:
    if (path)
        free(path);
    if (socket_name)
        free(socket_name);
    if (fp)
        fclose(fp);
    return err;
}

static int buf_contains_str(BufPtr *seen, const char *name, uint32_t namelen)
{
    UINTN i = 0;
    for (; i < seen->size; i++)
    {
        if (strncmp(name, (char*) seen->data[i], namelen) == 0)
            return 1;
    }

    return 0;
}

static int read_password_request(int fd, BufPtr *seen, const char *bootkey, const char *rootkey)
{
    uint8_t buf[sizeof(struct inotify_event) + NAME_MAX + 1];
    uint8_t *p = buf;
    int bytes_read;
    int err = -1;

    bytes_read = read(fd, buf, sizeof(buf));
    if (bytes_read == -1)
        GOTO(done);

    while (p < buf + bytes_read)
    {
        struct inotify_event *event = (struct inotify_event*) p;
        char *tmp = NULL;

        p += sizeof(struct inotify_event) + event->len;
        
        /* Check in buffer & skip duplicates. */
        if (buf_contains_str(seen, event->name, event->len))
            continue;

        tmp = (char*) malloc(event->len);
        if (tmp == NULL)
            GOTO(done);
    
        memcpy(tmp, event->name, event->len);   
        if (BufPtrAppend(seen, (const void **) &tmp, 1))
        {
            free(tmp);
            GOTO(done);
        }

        process_password_request(event, bootkey, rootkey);
    }
    err = 0;

done:
    return err;
}

static int watch_password_requests(int fd, const char *bootkey, const char *rootkey)
{
    /* Watch directory with timeout. */
    int err;
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;    
    BufPtr seen = BUF_PTR_INITIALIZER;

    do 
    {
        err = poll(&pfd, 1, TIMEOUT);
        if (err > 0)
        {
            read_password_request(fd, &seen, bootkey, rootkey);
            /*if (func_err)
            {
                err = func_err;
                goto done;
            }*/
        }
    } while (err > 0);

//done:
    {
        UINTN i;
        for (i = 0; i < seen.size; i++)
            free(seen.data[i]);
        BufPtrRelease(&seen);
    }
    return err;
}

static int update_directory(const char *dir)
{
    DIR *dir_fd = NULL;
    struct dirent *file;
    int err = -1;

    dir_fd = opendir(dir);
    if (dir_fd == NULL)
        GOTO(done);

    while ((file = readdir(dir_fd)))
    {
        int file_fd = -1;
        char path[PATH_MAX];
 
        // Only pay attention to ask. requests
        if (strncmp(file->d_name, PASS_PREFIX, strlen(PASS_PREFIX)) != 0)
            continue;

        strncpy(path, dir, PATH_MAX - NAME_MAX);
        strncat(path, file->d_name, NAME_MAX);
 
        file_fd = open(path, O_RDWR);
        if (file_fd == -1)
            GOTO(done);

        close(file_fd);               
    }
    err = 0;

done:
    if (dir_fd)
        closedir(dir_fd);
    return err;
}

int main(int argc, char **argv)
{
    int fd = -1;
    int err = -1;

    if (argc != 3)
    {
        printf("Usage: %s bootkey-path rootkey-path.\n", argv[0]);
        return -1;
    }

    fd = inotify_init();
    if (fd == -1)
        GOTO(done);

    if (inotify_add_watch(fd, WATCH_DIR, IN_CLOSE_WRITE | IN_MOVED_TO) == -1)
        GOTO(done);

    /* Update all the files now that we are watching the directory */
    err = update_directory(WATCH_DIR);
    if (err)
        GOTO(done);
    
    err = watch_password_requests(fd, argv[1], argv[2]);
    if (err)
        GOTO(done);

done:
    if (fd != -1)
        close(fd);
    return err;    	
}

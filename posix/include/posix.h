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
#ifndef posix_h
#define posix_h

#define POSIXEFI_INLINE static __inline__

#if !defined(POSIXEFI_SUPPRESS_DEFINITIONS)

#define assert posix_assert
#define tolower posix_tolower
#define isupper posix_isupper
#define isalpha posix_isalpha
#define isdigit posix_isdigit
#define isalnum posix_isalnum
#define isxdigit posix_isxdigit
#define isspace posix_isspace
#define isgraph posix_isgraph
#define RTLD_NOW posix_RTLD_NOW
#define RTLD_LAZY posix_RTLD_LAZY
#define Dl_info posix_Dl_info
#define dlopen posix_dlopen
#define dlsym posix_dlsym
#define dlerror posix_dlerror
#define dlclose posix_dlclose
#define dladdr posix_dladdr
#define EINVAL posix_EINVAL
#define ENOMEM posix_ENOMEM
#define ENOENT posix_ENOENT
#define errno posix_errno
#define strerror posix_strerror
#define ULONG_MAX posix_ULONG_MAX
#define LONG_MAX posix_LONG_MAX
#define INT_MAX posix_INT_MAX
#define offsetof posix_offsetof
#define BUFSIZ posix_BUFSIZ
#define FILE posix_FILE
#define stdout posix_stdout
#define stderr posix_stderr
#define printf posix_printf
#define fprintf posix_fprintf
#define vsprintf posix_vsprintf
#define sscanf posix_sscanf
#define vfprintf posix_vfprintf
#define sprintf posix_sprintf
#define fputs posix_fputs
#define putc posix_putc
#define fopen posix_fopen
#define fclose posix_fclose
#define fread posix_fread
#define fwrite posix_fwrite
#define feof posix_feof
#define fseek posix_fseek
#define ferror posix_ferror
#define ftell posix_ftell
#define fflush posix_fflush
#define fgets posix_fgets
#define malloc posix_malloc
#define calloc posix_calloc
#define realloc posix_realloc
#define free posix_free
#define qsort posix_qsort
#define getenv posix_getenv
#define abort posix_abort
#define atoi posix_atoi
#define strtoul posix_strtoul
#define exit posix_exit
#define strtol posix_strtol
#define memset posix_memset
#define memcpy posix_memcpy
#define memmove posix_memmove
#define memcmp posix_memcmp
#define strlen posix_strlen
#define strcmp posix_strcmp
#define strncmp posix_strncmp
#define strcpy posix_strcpy
#define strcat posix_strcat
#define strncpy posix_strncpy
#define strncasecmp posix_strncasecmp
#define strcasecmp posix_strcasecmp
#define strchr posix_strchr
#define strrchr posix_strrchr
#define strstr posix_strstr
#define memchr posix_memchr
#define strdup posix_strdup
#define time_t posix_time_t
#define timeval posix_timeval
#define tm posix_tm
#define time posix_time
#define gmtime_r posix_gmtime_r
#define tm posix_tm
#define pid_t posix_pid_t
#define uid_t posix_uid_t
#define gid_t posix_gid_t
#define off_t posix_off_t
#define getpid posix_getpid
#define getuid posix_getuid
#define geteuid posix_geteuid
#define getgid posix_getgid
#define getegid posix_getegid
#define localtime posix_localtime

#define size_t posix_size_t
#define ssize_t posix_ssize_t
#define uint8_t posix_uint8_t
#define int8_t posix_int8_t
#define uint16_t posix_uint16_t
#define int16_t posix_int16_t
#define uint32_t posix_uint32_t
#define int32_t posix_int32_t
#define uint64_t posix_uint64_t
#define int64_t posix_int64_t
#define uintptr_t posix_uintptr_t
#define bool posix_bool

#define open posix_open
#define lseek posix_lseek
#define read posix_read
#define write posix_write
#define close posix_close
#if !defined(BUILD_EFI)
# define va_list posix_va_list
# define va_start posix_va_start
# define va_end posix_va_end
#endif

#endif /* !defined(POSIXEFI_SUPPRESS_DEFINITIONS) */

#if defined(BUILD_EFI)
# define posix_va_list va_list
#endif

#endif /* posix_h */

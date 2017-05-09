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
#ifndef _linux_h
#define _linux_h

#include "config.h"
#include <lsvmutils/eficommon.h>

#define BOOT_PARAMS_EDD_MBR_SIG_MAX 16
#define BOOT_PARAMS_E820MAX 128
#define BOOT_PARAMS_EDDMAXNR 6

#define MINIMUM_SUPPORTED_VERSION 0x020B

#if defined(__linux__) && !defined(__x86_64)
# error "only X86-64-bit supported"
#endif

#if defined(_WIN32)
# pragma pack(1)
#endif

typedef struct boot_params boot_params_t;
typedef struct setup_header setup_header_t;

struct setup_header
{
#   define DEFAULT_SETUP_SECTS 4
#   define MAX_SETUP_SECTS 64
    UINT8 setup_sects;
    UINT16 root_flags;
    UINT32 syssize;
    UINT16 ram_size;
    UINT16 vid_mode;
    UINT16 root_dev;
#   define BOOT_FLAG 0xAA55
    UINT16 boot_flag;
    UINT16 jump;
#   define HEADER_MAGIC 0x53726448
    UINT32 header;
    UINT16 version;
    UINT32 realmode_swtch;
    UINT16 start_sys_seg;
    UINT16 kernel_version;
    UINT8 type_of_loader;
#   define LOADED_HIGH (1 << 0)
#   define KASLR_FLAG (1 << 1)
#   define QUIET_FLAG (1 << 5)
#   define KEEP_SEGMENTS (1 << 6)
#   define CAN_USE_HEAP (1 << 7)
    UINT8 loadflags;
    UINT16 setup_move_size;
    UINT32 code32_start;
    UINT32 ramdisk_image;
    UINT32 ramdisk_size;
    UINT32 bootsect_kludge;
    UINT16 heap_end_ptr;
    UINT8 ext_loader_ver;
    UINT8 ext_loader_type;
    UINT32 cmd_line_ptr;
    UINT32 initrd_addr_max;
    UINT32 kernel_alignment;
    UINT8 relocatable_kernel;
    UINT8 min_alignment;
    UINT16 xloadflags;
    UINT32 cmdline_size;
    UINT32 hardware_subarch;
    UINT64 hardware_subarch_data;
    UINT32 payload_offset;
    UINT32 payload_length;
    UINT64 setup_data;
#   define BZIMAGE_ADDRESS 0x100000
    UINT64 pref_address;
    UINT32 init_size;
    UINT32 handover_offset;
}
PACKED;

struct boot_params
{
    UINT8 __pad1[0x01F1];
#   define SETUP_OFFSET 0x01F1
    setup_header_t setup;
    /* Pad out to two sectors */
    UINT8 __pad2[1024 - sizeof(setup_header_t) - 0x01F1];
}
PACKED;

#if defined(_WIN32)
# pragma pack()
#endif

#endif /* _linux_h */

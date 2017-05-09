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
#ifndef _devpath_h
#define _devpath_h

#include "config.h"
#include <lsvmutils/inline.h>
#include <lsvmutils/eficommon.h>

#define DEVNODE_TYPE_END 0x7F
#define DEVNODE_SUBTYPE_END 0xFF

#define HARDDRIVE_TYPE 4
#define HARDDRIVE_SUBTYPE 1

#define SCSI_TYPE 3
#define SCSI_SUBTYPE 2

typedef struct _SCSIDevicePathPacked
{
    EFI_DEVICE_PATH header;
    UINT16 pun;
    UINT16 lun; 
}
__attribute__((packed))
SCSIDevicePathPacked;

typedef struct _HardDriveDevicePath
{
    EFI_DEVICE_PATH header;
    UINT32 partitionNumber;
    UINT64 partitionStart;
    UINT64 partitionSize;
    UINT8 signature[16];
    UINT8 mbrType;
    UINT8 signatureType;
}
__attribute__((packed))
HardDriveDevicePathPacked;

/* Type of this node in the device path */
INLINE UINT8 DevNodeType(const EFI_DEVICE_PATH* dp)
{
    /* Node: MSB used to indicate the path is unpacked by GNU-EFI */
    return dp->Type & 0x7F;
}

/* Type of this node in the device path */
INLINE UINT8 DevNodeSubType(const EFI_DEVICE_PATH* dp)
{
    return dp->SubType;
}

/* Length of this single node in the device path */
INLINE UINT32 DevNodeLength(const EFI_DEVICE_PATH* dp)
{
    return ((UINT32)(dp->Length[1]) << 8) | (UINT32)(dp->Length[0]);
}

/* Length of this node in the device path */
BOOLEAN IsDevNodeEnd(const EFI_DEVICE_PATH* dp);

/* Get next node in device path (or null if none left) */
const EFI_DEVICE_PATH* DevNodeNext(const EFI_DEVICE_PATH* dp);

/* Get length of the entire device path (all nodes) */
UINT32 DevPathLength(const EFI_DEVICE_PATH* dp);

/* Clone all device path nodes into dynamic memory */
EFI_DEVICE_PATH* DevPathClone(const EFI_DEVICE_PATH* dp);

/* Return TRUE if this is a harddrive node */
BOOLEAN IsHardDriveNode(const EFI_DEVICE_PATH* dp);

/* Return TRUE if this is a SCSI node */
BOOLEAN IsSCSINode(const EFI_DEVICE_PATH* dp);

/* Dump this device path to the log */
void DevPathDump(const EFI_DEVICE_PATH* dp);

/* Dump all device paths to the log (for all handles) */
void DevPathDumpAll();

/* Create a new partition device path from the first available partition */
EFI_DEVICE_PATH* DevPathCreatePseudoPartition(
    UINT64 partitionSizeInSectors,
    const EFI_GUID* guid,
    UINT64* firstLBA, 
    UINT64* lastLBA);

#endif /* _devpath_h */

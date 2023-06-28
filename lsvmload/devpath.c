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
#include "devpath.h"
#include "log.h"
#include <lsvmutils/strings.h>
#include <lsvmutils/alloc.h>
#include <lsvmutils/efibio.h>
#include <lsvmutils/guid.h>
#include "logging.h"

UINT32 DevPathLength(const EFI_DEVICE_PATH* dp)
{
    UINT32 len;

    for (len = 0; dp; dp = DevNodeNext(dp))
        len += DevNodeLength(dp);

    return len;
}

BOOLEAN IsDevNodeEnd(const EFI_DEVICE_PATH* dp)
{
    if (DevNodeType(dp) == DEVNODE_TYPE_END &&
        DevNodeSubType(dp) == DEVNODE_SUBTYPE_END)
    {
        return TRUE;
    }

    return FALSE;
}

const EFI_DEVICE_PATH* DevNodeNext(const EFI_DEVICE_PATH* dp)
{
    if (IsDevNodeEnd(dp))
        return NULL;

    return (const EFI_DEVICE_PATH*)((const UINT8*)dp + DevNodeLength(dp));
}

EFI_DEVICE_PATH* DevPathClone(const EFI_DEVICE_PATH* dp)
{
    UINT32 size = DevPathLength(dp);
    EFI_DEVICE_PATH* newdp;

    if (!(newdp = Malloc(size)))
        return NULL;

    Memcpy(newdp, dp, size);
    return newdp;
}

BOOLEAN IsHardDriveNode(const EFI_DEVICE_PATH* dp)
{
    if (DevNodeLength(dp) == sizeof(HardDriveDevicePathPacked) &&
        DevNodeType(dp) == HARDDRIVE_TYPE &&
        DevNodeSubType(dp) == HARDDRIVE_SUBTYPE)
    {
        return TRUE;
    }

    return FALSE;
}

BOOLEAN IsSCSINode(const EFI_DEVICE_PATH* dp)
{
    if (DevNodeLength(dp) == sizeof(SCSIDevicePathPacked) &&
        DevNodeType(dp) == SCSI_TYPE &&
        DevNodeSubType(dp) == SCSI_SUBTYPE)
    {
        return TRUE;
    }

    return FALSE;
}

void DevPathDump(const EFI_DEVICE_PATH* dp)
{
    CHAR16* str;

    if (!dp)
        return;

    LOGI(L"=== DEVICE PATH");

    if ((str = DevicePathToStr((EFI_DEVICE_PATH*)dp)))
        LOGI(L"%s", str);

    /* Seek to the last node */
    for (; dp; dp = DevNodeNext(dp))
    {
        LOGI(L"NODE:");
        LOGI(L"  node:Type{%X}", dp->Type);
        LOGI(L"  node:SubType{%X}", dp->SubType);
        LOGI(L"  node:Length{%d}", DevNodeLength(dp));

        if (IsSCSINode(dp))
        {
            SCSIDevicePathPacked scsi;
            Memcpy(&scsi, dp, sizeof(scsi));
            LOGI(L"  scsi:pun{%d}", scsi.pun);
            LOGI(L"  scsi:lun{%d}", scsi.lun);
        }
        else if (IsHardDriveNode(dp))
        {
            HardDriveDevicePathPacked hd;
            Memcpy(&hd, dp, sizeof(hd));
            LOGI(L"  hd:partitionNumber{%d}", (int)hd.partitionNumber);
            LOGI(L"  hd:partitionStart{%d}", (int)hd.partitionStart);
            LOGI(L"  hd:partitionSize{%d}", (int)hd.partitionSize);
            LogHexStr(L"  hd:signature", hd.signature, sizeof(hd.signature));
            LOGI(L"  hd:mbrType{%d}", (int)hd.mbrType);
            LOGI(L"  hd:signatureType{%d}", (int)hd.signatureType);
        }
    }
}

void DevPathDumpAll()
{
    EFI_HANDLE* handles = NULL;
    UINTN numHandles = 0;
    UINTN i;

    if (LocateBlockIOHandles(&handles, &numHandles) != EFI_SUCCESS)
    {
        LOGE(L"DevPathDumps() failed");
    }

    for (i = 0; i < numHandles; i++)
        DevPathDump(DevicePathFromHandle(handles[i]));

    if (handles)
        Free(handles);
}

EFI_DEVICE_PATH* DevPathCreatePseudoPartition(
    UINT64 partitionSizeInSectors,
    const EFI_GUID* guid,
    UINT64* firstLBA, 
    UINT64* lastLBA)
{
    EFI_DEVICE_PATH* result = NULL;
    EFI_HANDLE* handles = NULL;
    UINTN numHandles = 0;
    UINTN i;
    HardDriveDevicePathPacked hd;
    UINT8 _signature[16] =
    {
        0x1b, 0x00, 0xc2, 0x41, 0xee, 0xb3, 0x46, 0x34,
        0xb3, 0x3a, 0x35, 0x30, 0x6c, 0xc2, 0x63, 0xfe
    };

    Memset(&hd, 0, sizeof(hd));

    if (LocateBlockIOHandles(&handles, &numHandles) != EFI_SUCCESS)
        goto done;

    for (i = 0; i < numHandles; i++)
    {
        EFI_DEVICE_PATH* dp = DevicePathFromHandle(handles[i]);
        EFI_DEVICE_PATH* p = dp;

        for (; p; p = (EFI_DEVICE_PATH*)DevNodeNext(p))
        {
            if (IsSCSINode(p))
            {
                SCSIDevicePathPacked tmp;
                Memcpy(&tmp, p, sizeof(tmp));

                if (tmp.pun != 0 || tmp.lun != 0)
                {
                    /* Only search for a partition in SCSI(0,0): harddisk 0 */
                    break;
                }
            }
            else if (IsHardDriveNode(p))
            {
                HardDriveDevicePathPacked tmp;
                Memcpy(&tmp, p, sizeof(tmp));

                /* Assuming SCSI(PUN=0,LUN=0) */

                /* Clone the first harddrive we find */
                if (!result)
                {
                    if (!(result = DevPathClone(dp)))
                        goto done;

                    Memcpy(&hd, &tmp, sizeof(hd));
                }

                /* Save the highest partition number found so far */
                if (tmp.partitionNumber > hd.partitionNumber)
                    hd.partitionNumber = tmp.partitionNumber;

                /* Save the highest partition start */
                if (tmp.partitionStart > hd.partitionStart)
                {
                    hd.partitionStart = tmp.partitionStart;
                    hd.partitionSize = tmp.partitionSize;
                }
            }
        }
    }

    if (!result)
        goto done;

    /* Assign the next available partition number */
    hd.partitionNumber++;

    /* Assign sector start and size */
    hd.partitionStart = hd.partitionStart + hd.partitionSize;
    hd.partitionSize = partitionSizeInSectors;

    /* Change the partition's signature */
    Memcpy(hd.signature, _signature, sizeof(hd.signature));

    /* Update the harddisk node in 'result' */
    {
        EFI_DEVICE_PATH* dp;

        for (dp = result; dp; dp = (EFI_DEVICE_PATH*)DevNodeNext(dp))
        {
            if (IsHardDriveNode(dp))
                Memcpy(dp, &hd, sizeof(hd));
        }
    }

    /* Set sector of first block */
    *firstLBA = hd.partitionStart;

    /* Set sector of last block */
    *lastLBA = hd.partitionStart + hd.partitionSize - 1;

done:

    if (handles)
        Free(handles);

    return result;
}

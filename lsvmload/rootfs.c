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
#include "rootfs.h"
#include <lsvmutils/eficommon.h>
#include <lsvmutils/efiblkdev.h>
#include <lsvmutils/luksblkdev.h>
#include "luksbio.h"
#include "globals.h"
#include "log.h"

int TestRootDevice(
    EFI_HANDLE imageHandle,
    EFI_TCG2_PROTOCOL* tcg2Protocol,
    const UINT8* passphraseData,
    UINTN passphraseSize)
{
    int rc = -1;
    EFI_BIO* bio = NULL;
    Blkdev* rawdev = NULL;
    Blkdev* rootdev = NULL;

    /* Reject null parameters */
    if (!imageHandle || !tcg2Protocol || !passphraseData || !passphraseSize ||
        !globals.rootDevice)
    {
        goto done;
    }

    /* Open 'LUKS BIO' */
    if (!(bio = OpenLUKSBIO(imageHandle, globals.rootDevice)))
    {
        goto done;
    }

    /* Wrap 'LUKS BIO' in 'raw device' */
    if (!(rawdev = BlkdevFromBIO(bio)))
    {
        CloseBIO(bio);
        goto done;
    }

    LOGD(L"TestRootDevice::LUKSBlkdevFromRawBytes");
    /* Wrap 'cache device' in 'LUKS device' */
    if (!(rootdev = LUKSBlkdevFromRawBytes(
        rawdev, 
        passphraseData,
        passphraseSize)))
    {
        rawdev->Close(rawdev);
        goto done;
    }

    /* Close the device */
    rootdev->Close(rootdev);

    /* The passphrase was valid */
    rc = 0;

done:

    return rc;
}

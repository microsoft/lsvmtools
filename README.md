LSVMTools
=========

Overview
--------

The LSVMTools project provides tools for shielding Linux VMs operating
in the Microsoft(R) Windows Hyper-V environment. LSVMTools aims to protect
Linux VMs from attack while at rest and in flight. It builds on the following
technologies.

- Windows Server 2016 Guarded Fabric
- Hyper-V Shielded VMs
- UEFI
- TPM 2.0
- Linux Unified Key Setup (LUKS)
- dm-crypt

LSVMTools provides two main tools.

- **LSVMPREP** - Prepares the image for shielding
- **LSVMLOAD** - The primary boot loader for the image

### LSVMPREP

**LSVMPREP** prepares the Linux environment for shielding. After the image
is prepared, it must be templatized and provisioned as described in the
[LSVM How-To](doc/LSVM_How_To.pdf) document. LSVMPREP performs the following
steps.

- Encrypts the boot partition with a well-known passphrase
- Patches the system to automatically mount the encyrpted boot parition
- Installs LSVMLOAD on the EFI System Partition (ESP)
- TPM-seals the passphrases and stores them on the ESP
- Copies the SHIM and GRUB2 to the encrypted boot partition
- Patches the initial ramdisk configuration to get passphrases non-interactively
- Regenerates the initial ramdisks and GRUB2 configuration
- Applies any UEFI dbx updates (for black-listed boot loaders)

After these steps are performed, the image is ready to be templatized. See
the [LSVM How-To](doc/LSVM_How_To.pdf) document for details.

### LSVMLOAD

**LSVMLOAD** becomes the primary EFI boot loader for the Linux VM. UEFI loads
LSVMLOAD, assuming it passes certificate verification (**LSVMLOAD** must be
signed by a certificate that Hyper-V trusts). **LSVMLOAD** performs the 
following steps.

- Uses TPM to unseal a **keyfile** (containing the disk partition passphrases)
- Maps the ESP onto an **ESP ramdisk**
- Maps the encrypted boot partition onto an unencrypted **boot ramdisk**
- Patches the initial ramdisk with the **keyfile** (on the boot ramdisk)
- Loads the Linux SHIM from the **boot ramdisk**
- Launches the SHIM, which is redirected to the **ESP ramdisk**

The SHIM finds GRUB2 on the **ESP ramdisk** (copied from the encrypted boot
partition by LSVMLOAD). The SHIM executes GRUB2, which is redirected to the
**boot ramdisk**, where it finds:

- A patched GRUB2 configuration file
- A patched initial ramdisk (patched by LSVMLOAD with the **keyfile**)
- The Linux kernel

GRUB2 executes the kernel and the initial ramdisk. The initial ramdisk mounts
the boot and root partitions using the **keyfile** injected by LSVMLOAD.

LSVMLOAD works using unmodified SHIM and GRUB2 executables, making it
possible to configure a Linux environment for shielding without having
to change any programs along the boot chain.

Documents
---------

- [LSVM Overview](doc/LSVM_Overview.pdf)

- [LSVM How-To](doc/LSVM_How_To.pdf)


Supported Linux distributions
-----------------------------

LSVMTool current supports the following Linux distributions.

- Ubuntu 16.04 LTS with the 4.4 kernel
- Red Hat Enterprise Linux 7.3
- SUSE Linux Enterprise Server 12 Service Pack 2

Installing
----------

This section explains how to install from a binary distribution. Binary 
distributions can be downloaded from the following link.

- [Binary Distributions](binaries)

These distributions include **LSVMPREP** and a signed **LSVMLOAD**.

Use the following commands to install the distibution.

```
# tar zxvf lsvmtools-1.0.0-x86_64.tar.gz
# cd lsvmtools-1.0.0-x86_64
# ./install

Created /opt/lsvmtools-1.0.0
```

This installs LSVMTools in the following location.

```
/opt/lsvmtools-1.0.0
```

Running LSVMPREP
----------------

**Caution: Running LSVMPREP encrypts the boot partition and makes irreversible
configuration changes to a virtual machine. Only run LSVMPREP to prepare an
image for templatization.**

To run LSVMPREP, execute the following commands as root.

```
# cd /opt/lsvmtools-1.0.0
# ./lsvmprep

***************************************************
*     ____    _   _   _ _____ ___ ___  _   _      *
*    / ___|  / \ | | | |_   _|_ _/ _ \| \ | |     *
*   | |     / _ \| | | | | |  | | | | |  \| |     *
*   | |___ / ___ \ |_| | | |  | | |_| | |\  |     *
*    \____/_/   \_\___/  |_| |___\___/|_| \_|     *
*                                                 *
*                                                 *
* LSVMPREP is about to encrypt the boot partition *
* and make irreversible configuration changes to  *
* this machine. If you are certain you want to    *
* proceed, type YES in uppercase and then press   *
* enter; else press ENTER to terminate.           *
*                                                 *
***************************************************

> _
```

If LSVMPREP runs successfully, the image is ready to be templatized. See
[LSVM How-To](doc/LSVM_How_To.pdf) for what to do next.

Building
--------

This section explains how to build LSVMTools from source, but note that 
LSVMPREP requires a signed LSVMLOAD image, which must be downloaded separately 
(see the previous section for details).

To build LSVMTools, type these commands.

```
# ./configure
# make
```

These commands build LSVMPREP and an unsigned LSVMLOAD.

To run the tests, type:

```
# make tests
```
Recovering the LUKS keys
------------------------

In case anything goes wrong, use the following command to recover LUKS keys.

```
# dmsetup table --showkeys
```

License
-------

```
LSVMTools 

MIT License

Copyright (c) Microsoft Corporation. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in 
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE
```

Code of Conduct
---------------

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

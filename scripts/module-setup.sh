#!/bin/bash

check() 
{
    # Check if module exists
    if [ ! -f /lib/modules/$(uname -r)/kernel/drivers/char/tpm/tpm_crb.ko ]; then
        return 1
    fi    

    # Check if /boot and / are encrypted.
    require_binaries cryptsetup || return 1

    mounts=$(mount -l | grep -E " /(boot)? " | cut -d " " -f1)
    printf "%s\n" "$mounts" | while read -r line; do
        part=$(lsblk -P -p -s "$line" | grep "TYPE=\"part\"" | cut -d " " -f1 | cut -d "=" -f2 | sed "s/\"//g"1)
        cryptsetup isLuks "$part"
        if [[ "$?" != 0 ]]; then
            return 1
        fi
    done

    return 0
}

install()
{
    inst "lsvmtool"
}

installkernel()
{
    hostonly='' instmods tpm_crb hyperv-keyboard xts
}

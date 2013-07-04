#!/bin/bash

make ARCH=arm tenderloin_android_defconfig
make ARCH=arm CROSS_COMPILE=/home/admin/shumash/android/skz/prebuilt/linux-x86/toolchain/arm-eabi-4.4.3/bin/arm-eabi- uImage


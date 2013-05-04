#!/bin/bash

# get current path

$TOPDIR=`pwd`
now=$(date +"%m_%d_%Y")

#make ARCH=arm tenderloin_android_defconfig
#make ARCH=arm -j16 CROSS_COMPILE=~/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-eabi- uImage
cp ~/android/hp-kernel-tenderloin/arch/arm/boot/uImage ~/android/moboot/tools/uImage
cd ~/android/moboot/tools
rm -f uImage.CyanogenMod.new

mkimage -A arm -O linux -T ramdisk -C none -a 0x60000000 -e 0x60000000 -n "Image" -d ramdisk.img uRamdisk
mkimage -A arm -O linux -T multi -a 0x40208000  -e  0x40208000 -C none -n "multi image" -d uImage:uRamdisk  uImage.CyanogenMod.new

cp -f uImage.CyanogenMod.new boot.img

zip -f SKZ_OC2157MHz_Kernel.zip
cp SKZ_OC2157MHz_Kernel.zip SKZ_OC2157MHz_Kernel_$now.zip 

#cd $TOPDIR


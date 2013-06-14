#!/bin/bash
# %1=CM build, %2=copy to prebuilt kernel directory (y/n)
# get current path

TOPDIR=`pwd`
now=$(date +"%m_%d_%Y")

make ARCH=arm tenderloin_android_defconfig
make ARCH=arm -j16 CROSS_COMPILE=~/android/skz/prebuilt/linux-x86/toolchain/arm-eabi-4.4.3/bin/arm-eabi- uImage

cp ~/android/hp-kernel-tenderloin/arch/arm/boot/uImage ~/android/moboot/tools/uImage
cd ~/android/moboot/tools
rm -f uImage.CyanogenMod.new

mkimage -A arm -O linux -T ramdisk -C none -a 0x60000000 -e 0x60000000 -n "Image" -d $1_ramdisk.img uRamdisk
mkimage -A arm -O linux -T multi -a 0x40208000  -e  0x40208000 -C none -n "multi image" -d uImage:uRamdisk  uImage.CyanogenMod.new

cp -f uImage.CyanogenMod.new boot.img

zip -d OC320-2157MHz_Kernel.zip boot.img
zip -u OC320-2157MHz_Kernel.zip boot.img
cp OC320-2157MHz_Kernel.zip $1_OC320-2157MHz_Kernel_$now.zip 

if ($2 == y)
then
 cp ~/android/hp-kernel-tenderloin/arch/arm/boot/uImage ~/android/skz/device/hp/tenderloin/prebuilt/boot/kernel
fi

cd $TOPDIR

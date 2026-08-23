#!/bin/bash

#export PATH="$HOME/x-tools/arm-cortex_a8-linux-gnueabihf/bin

ROOT_DIR=$PWD
#file_device_tree="/home/william/Documents/Embedded_Systems/Embedded_Linux/kernel/linux-stable/arch/arm/boot/dts/ti/omap/am335x-boneblack-custom.*"
dts_target="/home/william/Documents/Embedded_Systems/Embedded_Linux/EmbeddedLinuxSystems"
linux_file="/home/william/Documents/Embedded_Systems/Embedded_Linux/kernel/linux-stable"
dts_file="/home/william/Documents/Embedded_Systems/Embedded_Linux/kernel/linux-stable/arch/arm/boot/dts/ti/omap/"


echo "Install imagem"
echo "actual_dir= $ROOT_DIR"
echo "linux_file= $linux_file"

rm "$file_device_tree/am335x-boneblack-custom.*" 
cp "$dts_target/am335x-boneblack-custom-pwm.dts" "$file_device_tree/am335x-boneblack-custom.dts"
cd $linux_file


make ARCH=arm CROSS_COMPILE=arm-cortex_a8-linux-gnueabihf- dtbs

#sudo rm /srv/tftp/am335x-boneblack-custom.dtb 
#sudo cp arch/arm/boot/dts/ti/omap/am335x-boneblack-custom.dtb /srv/tftp/


cd $ROOT_DIR

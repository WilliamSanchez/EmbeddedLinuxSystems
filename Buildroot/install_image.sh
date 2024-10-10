#!/bin/bash

file_root="/home/william/Documents/Embedded_Systems/Embedded_Linux/QEMU/buildroot/output/images/rootfs.tar"
file_nfsroot="/home/william/Documents/Embedded_Systems/Embedded_Linux/kernel/BB/nfsroot/"
file_services="/home/william/Documents/Embedded_Systems/Embedded_Linux/EmbeddedLinuxSystems/Buildroot/rootfs-overlay"

echo "Install imagem"
echo "file_root= $file_root"
echo "file_nfsroot= $file_nfsroot"

sudo rm -rf "$file_nfsroot*"
sudo cp $file_root $file_nfsroot
cd $file_nfsroot
sudo tar -xvf "rootfs.tar" 
sudo rm -rf "rootfs.tar"

cd "/home/william/Documents/Embedded_Systems/Embedded_Linux/EmbeddedLinuxSystems/kernel/lidar_serial-device-driver"
sudo make install

cd "/home/william/Documents/Embedded_Systems/Embedded_Linux/EmbeddedLinuxSystems/kernel/pwm_device_driver"
sudo make install

if [[ $1 = "demons" ]]; then
  echo "Installing demonds"
  sudo cp "$file_services"/S25modprobe "$file_services"/S99custom-service  "$file_nfsroot"/etc/init.d/
fi

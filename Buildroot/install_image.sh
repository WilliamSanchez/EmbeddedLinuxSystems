#!/bin/bash

file_root="/home/william/Documents/Embedded_Systems/Embedded_Linux/QEMU/buildroot/output/images/rootfs.tar"
file_nfsroot="/home/william/Documents/Embedded_Systems/Embedded_Linux/kernel/BB/nfsroot/"

echo "Install imagem"
echo "file_root= $file_root"
echo "file_nfsroot= $file_nfsroot"

sudo rm -rf "$file_nfsroot*"
sudo cp $file_root $file_nfsroot
cd $file_nfsroot
sudo tar -xvf "rootfs.tar" 
sudo rm -rf "rootfs.tar"

# EmbeddedLinuxSystems


1-> Toolchain
  
    1.1 Getting a cross-compiling toolchain
    
    	$ git clone https://github.com/crosstool-ng/crosstool-ng
    	$ cd crosstool-ng/
    	
    1.2 Configure toolchain
    
    	$ ./bootstrap
    	$ ./configure --enable-local
    	$ make
   	$ ./ct-ng help (if you need help)
   	$ ./ct-ng menuconfig
   	$ ./ct-ng list-samples
    
    1.3 Compile toolchain
    
    	$ ./ct-ng build
    	$ cd $HOME/x-tools 	
  
2-> Bootloadr (U-boot)

   2.1 Getting U-Boot
   
  	$ git clone https://gitlab.denx.de/u-boot/u-boot
  	$ cd u-boot
  	$ git checkout v2023.01
  	
   2.2 Compile U-boot
   
	$ cd ~.profile
	  copy and paste 'export PATH="$HOMEx-tools/arm-cortex_a8-linux-gnueabihf/bin:$PATH"'
	$ make CROSS_COMPILE=arm-cortex_a8-linux-gnueabihf- am335x_boneblack_defconfig 
	$ make ARCH=arm CROSS_COMPILE=arm-cortex_a8-linux-gnueabihf-
	$ 'Format-sd card (see format-sdcard.sh script)', create two partitions boot and rootfs
	$ cp MLO u-boot.img /media/<name>/boot
	$ sudo umount /media/<name>/boot
	
    2.3 Inside of target
    
    	* Network configuration
    	  => setenv ipaddr 192.168.1.100
    	  => setenv serverip 192.168.1.1    	  
    	  => saveenv
    
    2.4 Inside of PC
    
    	* add server ip connection
	  $ nmcli con add type ethernet ifname en... ip4 192.168.1.1/24
	  
	* sertting tftp server
	  $ sudo apt install tftpd-hpa
	  $ sudo systemctl status tftpd-hpa
	  $ sudo systemctl restart tftpd-hpa
	  $ cp <file> /srv/tftp
	  
	
3-> Kernel Cross-compile

     3.1 getting the main linux tree 
      
        $ clone https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux
        $ cd linux
        $ git branch -a
        $ git checkout stable/linux-5.15.y	
        
     3.2 Compile the linux kernel
     
     	$ make ARCH=arm multi_v7_defconfig
     	$ make ARCH=arm menuconfig 
     	$ make -j5 ARCH=arm CROSS_COMPILE=arm-cortex_a8-linux-gnueabihf- zImge
     	$ make -j5 ARCH=arm CROSS_COMPILE=arm-cortex_a8-linux-gnueabihf- dtbs
     	
     3.3 Inside of target
        => setenv bootargs console=ttyO0,115200
        => saveenv
        => setenv bootcmd 'tftp 0x80200000 zImage; tftp 0x80F00000 am335x-boneblack_custom.dtb; bootz 0x80200000 - 0x80F00000'
        => saveenv
     	

4-> Busybox

     4.1 Setting NFS Server
      	$ sudo apt install nfs-kernel-server
      	* edit /etc/exports file
      	  /home/<usder>/<path nfsroot>/nfsroot 192.168.1.100(rw,no_root_squash,no_subtree_check)
      	$ sudo exportfs -r
      	
     4.2 inside of target, add booting arguments
        => setenv bootargs ${bootargs} root=/dev/nfs ip=192.168.1.100 nfsroot=192.168.1.1:/home/<user>/<path nfsroot>/nfsroot,nfsvers=3,tcp rw
        => saveenv
        
     4.3 Getting and compiel busybox
     
        $ git clone https://git.busybox.net/busubox
        $ cd busybox
        $ git checkout 1_35_0
        $ make distclean
        $ make defconfig
        $ make menuconfig 
        $ make ARCH=arm CROSS_COMPILE=arm-cortex_a8-linux-gnueabihf- install
        
     4.4. System configuration and startup     
     	* create a /etc/inittab file   (nfsroot file)
     	* create a /ect/init.d/rcS satartup script (nfsroot file)

5-> File Systems (Buildroot)

     5.1 Getting and configure Buildroot
        $ git clone https://git.buildroot.net/buildroot
        $ cd buildroot
        $ make menuconfig
        $ make
        
        $ ls ../buildroot/output/images/rootfs.tar
        
        
        
        

6-> Applications

7-> Yocto
  
   7.1 Getting yocto project
	
	$ git clone https://git.yoctoproject.org/git/poky
	$ cd poky
	$ git checkout -b kirkstone-4.0.5 kirkstone-4.0.5

   7.2 Generate an embedded linux sysrems
	$ source oe-init-build build-nova
	$ bitbake core-image-minimal 

  7.3 
        

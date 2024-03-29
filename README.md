# EmbeddedLinuxSystems


1-> Toolchain
  
    1.1 Getting a cross-compiling toolchain
    
    	* git clone https://github.com/crosstool-ng/crosstool-ng
    	* cd crosstool-ng/
    	
    1.2 Configure toolchain
    	* ./bootstrap
    	* ./configure --enable-local
    	* make
   	* ./ct-ng help (if you need help)
   	* ./ct-ng menuconfig
   	* ./ct-ng list-samples
    
    1.3 Compile toolchain
    	* ./ct-ng build
    	* cd $HOME/x-tools 	
  
2-> Bootloadr (U-boot)
3-> Kernel Cross-compile
4-> Busibox
5-> File Systems (Buildroot)
6-> Applications
7-> Yocto


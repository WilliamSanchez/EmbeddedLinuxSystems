obj-m += lidar_serial.o

KDIR := /home/william/Documents/Embedded_Systems/Embedded_Linux/kernel/linux
INSTALLDIR := /home/william/Documents/Embedded_Systems/Embedded_Linux/kernel/BB/nfsroot
PWD := $(shell pwd)

all:
	make ARCH=arm CROSS_COMPILE=arm-cortex_a8-linux-gnueabihf- -C $(KDIR) M=$(PWD) modules
	
install:
	make ARCH=arm CROSS_COMPILE=arm-cortex_a8-linux-gnueabihf- -C $(KDIR) M=$(PWD) INSTALL_MOD_PATH=$(INSTALLDIR) modules_install

clean:
	make -C $(KDIR) M=$(PWD) clean

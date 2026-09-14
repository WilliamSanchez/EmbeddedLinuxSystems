obj-m += ov7670_device_driver.o
GCC := /home/william/x-tools/arm-cortex_a8-linux-gnueabihf/bin/arm-cortex_a8-linux-gnueabihf-
KDIR := /home/william/Documents/Embedded_Systems/Embedded_Linux/QEMU/buildroot/output/build/linux-6.5
INSTALLDIR := /home/william/Documents/Embedded_Systems/Embedded_Linux/kernel/BB/nfsroot
PWD := $(shell pwd)

all:
	make ARCH=arm CROSS_COMPILE=$(GCC) -C $(KDIR) M=$(PWD) modules

install:
	make ARCH=arm CROSS_COMPILE=$(GCC) -C $(KDIR) M=$(PWD) INSTALL_MOD_PATH=$(INSTALLDIR) modules_install

clean:
	make -C $(KDIR) M=$(PWD) clean

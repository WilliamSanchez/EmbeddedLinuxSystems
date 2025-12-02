obj-m += st7796s_driver.o
st7796s_driver-objs := st7796s_device_driver.o st7796s.o

#KDIR := /home/william/Documents/Embedded_Systems/Embedded_Linux/kernel/linux-stable
KDIR := /home/william/Documents/Embedded_Systems/Embedded_Linux/QEMU/buildroot/output/build/linux-6.5
INSTALLDIR := /home/william/Documents/Embedded_Systems/Embedded_Linux/kernel/BB/nfsroot
PWD := $(shell pwd)

all:
	make ARCH=arm CROSS_COMPILE=arm-cortex_a8-linux-gnueabihf- -C $(KDIR) M=$(PWD) modules

install:
	make ARCH=arm CROSS_COMPILE=arm-cortex_a8-linux-gnueabihf- -C $(KDIR) M=$(PWD) INSTALL_MOD_PATH=$(INSTALLDIR) modules_install

clean:
	make -C $(KDIR) M=$(PWD) clean

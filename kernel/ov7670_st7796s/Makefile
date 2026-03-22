#obj-m += ov7670_device_driver.o
obj-m += mlx9060_driver.o
mlx9060_driver-objs := mlx90640_device_driver.o mlx90640.o
#obj-m += st7796s_driver.o
#st7796s_driver-objs := st7796s_device_driver.o st7796s.o

#KDIR := /home/william/Documents/Embedded_Systems/Embedded_Linux/kernel/linux-stable
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

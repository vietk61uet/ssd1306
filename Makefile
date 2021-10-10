obj-m := i2c_client.o my-i2c-omap.o button.o
ARCH=arm
CROSS_COMPILE=/home/phanviet/gcc-linaro-6.5.0-2018.12-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
KERNEL_DIR=/home/phanviet/bb-kernel/KERNEL/
HOST_KERNEL_DIR=/lib/modules/$(shell uname -r)/build/

all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNEL_DIR) M=$(PWD) modules
clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNEL_DIR) M=$(PWD) clean

host:
	make -C $(HOST_KERNEL_DIR) M=$(PWD) modules
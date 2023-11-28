obj-m += gpio_controller_driver.o
KDIR = /lib/modules/$(shell uname -r)/build
all:
	make -C $(KDIR)  M=$(shell pwd) modules
clean:
	make -C $(KDIR)  M=$(shell pwd) clean
install:
	rmmod gpio_controller_driver
	insmod gpio_controller_driver.ko
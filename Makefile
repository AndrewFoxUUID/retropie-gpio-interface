obj-m += gpio_controller_driver.o
KDIR = /lib/modules/$(shell uname -r)/build
all:
	make -C $(KDIR)  M=$(shell pwd) modules
clean:
	make -C $(KDIR)  M=$(shell pwd) clean
install:
	insmod gpio_controller_driver.ko
reinstall:
	rmmod gpio_controller_driver
	insmod gpio_controller_driver.ko
get_spidev_tester:
	wget https://raw.githubusercontent.com/raspberrypi/linux/rpi-6.1.y/tools/spi/spidev_test.c
	gcc -o spidev_test spidev_test.c
spidev_test0:
	./spidev_test -D /dev/spidev0.0
spidev_test1:
	./spidev_test -D /dev/spidev0.1
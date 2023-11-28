#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/err.h>
#include <linux/uaccess.h>

static int gpio_controller_open(struct inode *inode, struct file *file);
static ssize_t gpio_controller_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static int gpio_controller_release(struct inode *inode, struct file *file);
static int __init gpio_controller_driver_init(void);
static void __exit gpio_controller_driver_exit(void);

dev_t dev = 0;
static struct cdev gpio_controller_cdev;
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = gpio_controller_open,
    .read = gpio_controller_read,
    .release = gpio_controller_release
};
static struct class *dev_class;

static int gpio_controller_open(struct inode *inode, struct file *file) {
    return 0;
}

static ssize_t gpio_controller_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {
    char gpio_state = 0;
    gpio_state = '0' + gpio_get_value(17);
    return simple_read_from_buffer(buf, len, off, &gpio_state, sizeof(char))
}

static int gpio_controller_release(struct inode *inode, struct file *file) {
    return 0;
}

static int __init gpio_controller_driver_init(void) {
    if (alloc_chrdev_region(&dev, 0, 1, "gpio_controller") == 0) {
        cdev_init(&gpio_controller_cdev, &fops);
        if (cdev_add(&gpio_controller_cdev, dev, 1) == 0) {
            if (!IS_ERR(dev_class = class_create(THIS_MODULE, "gpio_controller_class"))) {
                if (!IS_ERR(device_create(dev_class, NULL, dev, NULL, "gpio_controller_device"))) {
                    if (gpio_is_valid(17) == true) {
                        if (gpio_request(17, "GPIO_17") == 0) {
                            gpio_direction_input(17);
                            gpio_export(17, false);
                            return 0;
                        }
                        gpio_free(17);
                    }
                }
                device_destroy(dev_class, dev);
            }
            class_destroy(dev_class);
        }
        cdev_del(&gpio_controller_cdev);
    }
    unregister_chrdev_region(dev, 1);
    return -1;
}

static void __exit gpio_controller_driver_exit(void) {
    gpio_unexport(17);
    gpio_free(17);
    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&gpio_controller_cdev);
    unregister_chrdev_region(dev, 1);
}

module_init(gpio_controller_driver_init);
module_exit(gpio_controller_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrew Fox");
MODULE_DESCRIPTION("A driver for a GPIO based custom RetroPie controller");
MODULE_VERSION("0.1");

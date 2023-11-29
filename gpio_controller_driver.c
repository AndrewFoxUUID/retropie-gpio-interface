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
#include <linux/jiffies.h>
#include <linux/interrupt.h>

extern unsigned long volatile jiffies;
unsigned long old_jiffie = 0;
unsigned int GPIO_irqNumber;

static int gpio_controller_open(struct inode *inode, struct file *file);
static ssize_t gpio_controller_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static int gpio_controller_release(struct inode *inode, struct file *file);
static irqreturn_t gpio_irq_handler(int irq, void *dev_id);
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
    char gpio_state[2] = {'\0', '\n'};
    gpio_state[0] = '0' + gpio_get_value(11);
    return simple_read_from_buffer(buf, len, off, &gpio_state, sizeof(char[2]));
}

static int gpio_controller_release(struct inode *inode, struct file *file) {
    return 0;
}

static irqreturn_t gpio_irq_handler(int irq, void *dev_id) {
    static unsigned long flags = 0;
    unsigned long diff = jiffies - old_jiffie;
    if (diff < 20) {
        pr_info("SHORT IRQ EVENT")
        return IRQ_HANDLED;
    }
    old_jiffie = jiffies;
    local_irq_save(flags);
    pr_info("LONG IRQ EVENT");
    local_irq_restore(flags);
    return IRQ_HANDLED;
}

static int __init gpio_controller_driver_init(void) {
    if (alloc_chrdev_region(&dev, 0, 1, "gpio_controller") == 0) {
        cdev_init(&gpio_controller_cdev, &fops);
        if (cdev_add(&gpio_controller_cdev, dev, 1) == 0) {
            if (!IS_ERR(dev_class = class_create(THIS_MODULE, "gpio_controller_class"))) {
                if (!IS_ERR(device_create(dev_class, NULL, dev, NULL, "gpio_controller_device"))) {
                    if (gpio_is_valid(11) == true) {
                        if (gpio_request(11, "GPIO_11") == 0) {
                            gpio_direction_input(11);
                            GPIO_irqNumber = gpio_to_irq(11);
                            if (request_irq(GPIO_irqNumber, (void*) gpio_irq_handler, IRQF_TRIGGER_RISING, "gpio_controller_device", NULL) == 0) {
                                return 0;
                            }
                        }
                        gpio_free(11);
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
    gpio_unexport(11);
    gpio_free(11);
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

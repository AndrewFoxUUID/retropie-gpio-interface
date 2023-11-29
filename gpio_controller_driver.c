#include <linux/input.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/jiffies.h>

extern unsigned long volatile jiffies;

static irqreturn_t gpio_controller_interrupt(int irq, void *dummy);
static int __init gpio_controller_driver_init(void);
static void __exit gpio_controller_driver_exit(void);

static struct input_dev *gpio_controller_dev;
unsigned int pin11_irq_number;
unsigned long old_jiffie = 0;

static irqreturn_t gpio_controller_interrupt(int irq, void *dummy) {
    static unsigned long flags;
    local_irq_save(flags);
    if (jiffies - old_jiffie > 20) {
        input_report_key(gpio_controller_dev, KEY_A, gpio_get_value(11));
        input_sync(gpio_controller_dev);
    }
    old_jiffie = jiffies;
    local_irq_restore(flags);
    return IRQ_HANDLED;
}

static int __init gpio_controller_driver_init(void) {
    if (gpio_is_valid(11) == true) {
        if (gpio_request(11, "GPIO_11") == 0) {
            gpio_direction_input(11);
            pin11_irq_number = gpio_to_irq(11);
            if (request_irq(pin11_irq_number, gpio_controller_interrupt, IRQF_TRIGGER_HIGH, "gpio_controller_device", NULL) == 0) {
                gpio_controller_dev = input_allocate_device();
                if (gpio_controller_dev) {
                    gpio_controller_dev->name = "gpio_controller_device";
                    gpio_controller_dev->evbit[0] = BIT_MASK(EV_KEY);
                    gpio_controller_dev->keybit[BIT_WORD(KEY_A)] = BIT_MASK(KEY_A);
                    if (input_register_device(gpio_controller_dev) == 0){
                        return 0;
                    }
                    input_free_device(gpio_controller_dev);
                }
                free_irq(pin11_irq_number, gpio_controller_interrupt);
            }
            gpio_free(11);
        }
    }
    return -1;
}

static void __exit gpio_controller_driver_exit(void) {
    input_unregister_device(gpio_controller_dev);
    free_irq(pin11_irq_number, gpio_controller_interrupt);
    gpio_free(11);
}

module_init(gpio_controller_driver_init);
module_exit(gpio_controller_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrew Fox");
MODULE_DESCRIPTION("A driver for a GPIO based custom RetroPie controller");
MODULE_VERSION("0.2");

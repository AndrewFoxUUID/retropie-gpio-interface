#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/jiffies.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrew Fox");
MODULE_DESCRIPTION("A driver for a GPIO based custom RetroPie controller");
MODULE_VERSION("0.2");

#define LEFT_SHOULDER_PIN   4
#define RIGHT_SHOULDER_PIN  17
#define START_PIN           22
#define SELECT_PIN          27
#define A_PIN               12
#define B_PIN               25
#define X_PIN               24
#define Y_PIN               23
#define VRX_PIN             6
#define VRY_PIN             13
#define SW_PIN              5

#define LEFT_SHOULDER_KEY   KEY_GRAVE
#define RIGHT_SHOULDER_KEY  KEY_1
#define START_KEY           KEY_SPACE
#define SELECT_KEY          KEY_ENTER
#define A_KEY               KEY_C
#define B_KEY               KEY_V
#define X_KEY               KEY_Z
#define Y_KEY               KEY_X
#define UP_KEY              KEY_UP
#define DOWN_KEY            KEY_DOWN
#define LEFT_KEY            KEY_LEFT
#define RIGHT_KEY           KEY_RIGHT

#define DEBOUNCE_TIME       15

extern unsigned long volatile jiffies;

static struct input_dev *gpio_controller_dev;
/*unsigned int left_shoulder_irq_number;
unsigned int right_shoulder_irq_number;
unsigned int start_irq_number;
unsigned int select_irq_number;*/
unsigned int a_irq_number;
/*unsigned int b_irq_number;
unsigned int x_irq_number;
unsigned int y_irq_number;
unsigned long old_left_shoulder_jiffie = 0;
unsigned long old_right_shoulder_jiffie = 0;
unsigned long old_start_jiffie = 0;
unsigned long old_select_jiffie = 0;*/
unsigned long old_a_jiffie = 0;
/*unsigned long old_b_jiffie = 0;
unsigned long old_x_jiffie = 0;
unsigned long old_y_jiffie = 0;*/

/*static irqreturn_t left_shoulder_press_interrupt(int irq, void *dummy) {
    static unsigned long flags;
    local_irq_save(flags);
    if (jiffies - old_left_shoulder_jiffie > DEBOUNCE_TIME) {
        input_report_key(gpio_controller_dev, LEFT_SHOULDER_KEY, 1);
        input_sync(gpio_controller_dev);
        old_left_shoulder_jiffie = jiffies;
    }
    local_irq_restore(flags);
    return IRQ_HANDLED;
}

static irqreturn_t left_shoulder_release_interrupt(int irq, void *dummy) {
    static unsigned long flags;
    local_irq_save(flags);
    if (jiffies - old_left_shoulder_jiffie > DEBOUNCE_TIME) {
        input_report_key(gpio_controller_dev, LEFT_SHOULDER_KEY, 0);
        input_sync(gpio_controller_dev);
        old_left_shoulder_jiffie = jiffies;
    }
    local_irq_restore(flags);
    return IRQ_HANDLED;
}

static irqreturn_t right_shoulder_press_interrupt(int irq, void *dummy) {
    static unsigned long flags;
    local_irq_save(flags);
    if (jiffies - old_right_shoulder_jiffie > DEBOUNCE_TIME) {
        input_report_key(gpio_controller_dev, RIGHT_SHOULDER_KEY, 1);
        input_sync(gpio_controller_dev);
        old_right_shoulder_jiffie = jiffies;
    }
    local_irq_restore(flags);
    return IRQ_HANDLED;
}

static irqreturn_t right_shoulder_release_interrupt(int irq, void *dummy) {
    static unsigned long flags;
    local_irq_save(flags);
    if (jiffies - old_right_shoulder_jiffie > DEBOUNCE_TIME) {
        input_report_key(gpio_controller_dev, RIGHT_SHOULDER_KEY, 0);
        input_sync(gpio_controller_dev);
        old_right_shoulder_jiffie = jiffies;
    }
    local_irq_restore(flags);
    return IRQ_HANDLED;
}

static irqreturn_t start_press_interrupt(int irq, void *dummy) {
    static unsigned long flags;
    local_irq_save(flags);
    if (jiffies - old_start_jiffie > DEBOUNCE_TIME) {
        input_report_key(gpio_controller_dev, START_KEY, 1);
        input_sync(gpio_controller_dev);
        old_start_jiffie = jiffies;
    }
    local_irq_restore(flags);
    return IRQ_HANDLED;
}

static irqreturn_t start_release_interrupt(int irq, void *dummy) {
    static unsigned long flags;
    local_irq_save(flags);
    if (jiffies - old_start_jiffie > DEBOUNCE_TIME) {
        input_report_key(gpio_controller_dev, START_KEY, 0);
        input_sync(gpio_controller_dev);
        old_start_jiffie = jiffies;
    }
    local_irq_restore(flags);
    return IRQ_HANDLED;
}

static irqreturn_t select_press_interrupt(int irq, void *dummy) {
    static unsigned long flags;
    local_irq_save(flags);
    if (jiffies - old_select_jiffie > DEBOUNCE_TIME) {
        input_report_key(gpio_controller_dev, SELECT_KEY, 1);
        input_sync(gpio_controller_dev);
        old_select_jiffie = jiffies;
    }
    local_irq_restore(flags);
    return IRQ_HANDLED;
}

static irqreturn_t select_release_interrupt(int irq, void *dummy) {
    static unsigned long flags;
    local_irq_save(flags);
    if (jiffies - old_select_jiffie > DEBOUNCE_TIME) {
        input_report_key(gpio_controller_dev, SELECT_KEY, 0);
        input_sync(gpio_controller_dev);
        old_select_jiffie = jiffies;
    }
    local_irq_restore(flags);
    return IRQ_HANDLED;
}*/

static irqreturn_t a_interrupt(int irq, void *dummy) {
    static unsigned long flags;
    local_irq_save(flags);
    if (jiffies - old_a_jiffie > DEBOUNCE_TIME) {
        pr_info("UNBOUNCED INTERRUPT");
        input_report_key(gpio_controller_dev, A_KEY, gpio_get_value(A_PIN));
        input_sync(gpio_controller_dev);
        old_a_jiffie = jiffies;
    }
    local_irq_restore(flags);
    return IRQ_HANDLED;
}

/*static irqreturn_t b_press_interrupt(int irq, void *dummy) {
    static unsigned long flags;
    local_irq_save(flags);
    if (jiffies - old_b_jiffie > DEBOUNCE_TIME) {
        input_report_key(gpio_controller_dev, B_KEY, 1);
        input_sync(gpio_controller_dev);
        old_b_jiffie = jiffies;
    }
    local_irq_restore(flags);
    return IRQ_HANDLED;
}

static irqreturn_t b_release_interrupt(int irq, void *dummy) {
    static unsigned long flags;
    local_irq_save(flags);
    if (jiffies - old_b_jiffie > DEBOUNCE_TIME) {
        input_report_key(gpio_controller_dev, B_KEY, 0);
        input_sync(gpio_controller_dev);
        old_b_jiffie = jiffies;
    }
    local_irq_restore(flags);
    return IRQ_HANDLED;
}

static irqreturn_t x_press_interrupt(int irq, void *dummy) {
    static unsigned long flags;
    local_irq_save(flags);
    if (jiffies - old_x_jiffie > DEBOUNCE_TIME) {
        input_report_key(gpio_controller_dev, X_KEY, 1);
        input_sync(gpio_controller_dev);
        old_x_jiffie = jiffies;
    }
    local_irq_restore(flags);
    return IRQ_HANDLED;
}

static irqreturn_t x_release_interrupt(int irq, void *dummy) {
    static unsigned long flags;
    local_irq_save(flags);
    if (jiffies - old_x_jiffie > DEBOUNCE_TIME) {
        input_report_key(gpio_controller_dev, X_KEY, 0);
        input_sync(gpio_controller_dev);
        old_x_jiffie = jiffies;
    }
    local_irq_restore(flags);
    return IRQ_HANDLED;
}

static irqreturn_t y_press_interrupt(int irq, void *dummy) {
    static unsigned long flags;
    local_irq_save(flags);
    if (jiffies - old_y_jiffie > DEBOUNCE_TIME) {
        input_report_key(gpio_controller_dev, Y_KEY, 1);
        input_sync(gpio_controller_dev);
        old_y_jiffie = jiffies;
    }
    local_irq_restore(flags);
    return IRQ_HANDLED;
}

static irqreturn_t y_release_interrupt(int irq, void *dummy) {
    static unsigned long flags;
    local_irq_save(flags);
    if (jiffies - old_y_jiffie > DEBOUNCE_TIME) {
        input_report_key(gpio_controller_dev, Y_KEY, 0);
        input_sync(gpio_controller_dev);
        old_y_jiffie = jiffies;
    }
    local_irq_restore(flags);
    return IRQ_HANDLED;
}*/

static int __init gpio_controller_driver_init(void) {
    gpio_controller_dev = input_allocate_device();
    if (gpio_controller_dev) {
        gpio_controller_dev->name = "gpio_controller_device";
        gpio_controller_dev->evbit[0] = BIT_MASK(EV_KEY);
        /*gpio_controller_dev->keybit[BIT_WORD(LEFT_SHOULDER_KEY)] = BIT_MASK(LEFT_SHOULDER_KEY);
        gpio_controller_dev->keybit[BIT_WORD(RIGHT_SHOULDER_KEY)] = BIT_MASK(RIGHT_SHOULDER_KEY);
        gpio_controller_dev->keybit[BIT_WORD(START_KEY)] = BIT_MASK(START_KEY);
        gpio_controller_dev->keybit[BIT_WORD(SELECT_KEY)] = BIT_MASK(SELECT_KEY);*/
        gpio_controller_dev->keybit[BIT_WORD(A_KEY)] = BIT_MASK(A_KEY);
        /*gpio_controller_dev->keybit[BIT_WORD(B_KEY)] = BIT_MASK(B_KEY);
        gpio_controller_dev->keybit[BIT_WORD(X_KEY)] = BIT_MASK(X_KEY);
        gpio_controller_dev->keybit[BIT_WORD(Y_KEY)] = BIT_MASK(Y_KEY);*/

        if (input_register_device(gpio_controller_dev) == 0) {
            char pin_code[8] = "GPIO_XX\0";

            /*if (gpio_is_valid(LEFT_SHOULDER_PIN) == true) {
                pin_code[5] = '0' + (LEFT_SHOULDER_PIN / 10);
                pin_code[6] = '0' + (LEFT_SHOULDER_PIN % 10);
                if (gpio_request(LEFT_SHOULDER_PIN, pin_code) == 0) {
                    gpio_direction_input(LEFT_SHOULDER_PIN);
                    left_shoulder_irq_number = gpio_to_irq(LEFT_SHOULDER_PIN);
                    if (request_irq(left_shoulder_irq_number, left_shoulder_press_interrupt, IRQF_TRIGGER_HIGH, "gpio_controller_device", NULL)) {
                        return -1;//goto unset_left_shoulder_press;
                    }
                    if (request_irq(left_shoulder_irq_number, left_shoulder_release_interrupt, IRQF_TRIGGER_LOW, "gpio_controller_device", NULL)) {
                        return -1;//goto unset_left_shoulder_release;
                    }
                }
            } else {
                return -1;//goto unset_all;
            }

            if (gpio_is_valid(RIGHT_SHOULDER_PIN) == true) {
                pin_code[5] = '0' + (RIGHT_SHOULDER_PIN / 10);
                pin_code[6] = '0' + (RIGHT_SHOULDER_PIN % 10);
                if (gpio_request(RIGHT_SHOULDER_PIN, pin_code) == 0) {
                    gpio_direction_input(RIGHT_SHOULDER_PIN);
                    right_shoulder_irq_number = gpio_to_irq(RIGHT_SHOULDER_PIN);
                    if (request_irq(right_shoulder_irq_number, right_shoulder_press_interrupt, IRQF_TRIGGER_HIGH, "gpio_controller_device", NULL)) {
                        return -1;//goto unset_right_shoulder_press;
                    }
                    if (request_irq(right_shoulder_irq_number, right_shoulder_release_interrupt, IRQF_TRIGGER_LOW, "gpio_controller_device", NULL)) {
                        return -1;//goto unset_right_shoulder_release;
                    }
                }
            } else {
                return -1;//goto unset_left_shoulder_release;
            }

            if (gpio_is_valid(START_PIN) == true) {
                pin_code[5] = '0' + (START_PIN / 10);
                pin_code[6] = '0' + (START_PIN % 10);
                if (gpio_request(START_PIN, pin_code) == 0) {
                    gpio_direction_input(START_PIN);
                    start_irq_number = gpio_to_irq(START_PIN);
                    if (request_irq(start_irq_number, start_press_interrupt, IRQF_TRIGGER_HIGH, "gpio_controller_device", NULL)) {
                        return -1;//goto unset_start_press;
                    }
                    if (request_irq(start_irq_number, start_release_interrupt, IRQF_TRIGGER_LOW, "gpio_controller_device", NULL)) {
                        return -1;//goto unset_start_release;
                    }
                }
            } else {
                return -1;//goto unset_right_shoulder_release;
            }

            if (gpio_is_valid(SELECT_PIN) == true) {
                pin_code[5] = '0' + (SELECT_PIN / 10);
                pin_code[6] = '0' + (SELECT_PIN % 10);
                if (gpio_request(SELECT_PIN, pin_code) == 0) {
                    gpio_direction_input(SELECT_PIN);
                    select_irq_number = gpio_to_irq(SELECT_PIN);
                    if (request_irq(select_irq_number, select_press_interrupt, IRQF_TRIGGER_HIGH, "gpio_controller_device", NULL)) {
                        return -1;//goto unset_select_press;
                    }
                    if (request_irq(select_irq_number, select_release_interrupt, IRQF_TRIGGER_LOW, "gpio_controller_device", NULL)) {
                        return -1;//goto unset_select_release;
                    }
                }
            } else {
                return -1;//goto unset_start_release;
            }*/

            if (gpio_is_valid(A_PIN) == true) {
                pin_code[5] = '0' + (A_PIN / 10);
                pin_code[6] = '0' + (A_PIN % 10);

                pr_info("%s", pin_code);

                if (gpio_request(A_PIN, pin_code) == 0) {
                    gpio_direction_input(A_PIN);
                    a_irq_number = gpio_to_irq(A_PIN);
                    pr_info("got irq number");
                    if (request_irq(a_irq_number, a_interrupt, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "gpio_controller_device", NULL) < 0) {
                        pr_info("request irq failed");
                        return -1;//goto unset_a;
                    }
                }

                pr_info("init success");
            } else {
                return -1;//goto unset_select_release;
            }

            /*if (gpio_is_valid(B_PIN) == true) {
                pin_code[5] = '0' + (B_PIN / 10);
                pin_code[6] = '0' + (B_PIN % 10);
                if (gpio_request(B_PIN, pin_code) == 0) {
                    gpio_direction_input(B_PIN);
                    b_irq_number = gpio_to_irq(B_PIN);
                    if (request_irq(b_irq_number, b_press_interrupt, IRQF_TRIGGER_HIGH, "gpio_controller_device", NULL)) {
                        return -1;//goto unset_b_press;
                    }
                    if (request_irq(b_irq_number, b_release_interrupt, IRQF_TRIGGER_LOW, "gpio_controller_device", NULL)) {
                        return -1;//goto unset_b_release;
                    }
                }
            } else {
                return -1;//goto unset_a_release;
            }

            if (gpio_is_valid(X_PIN) == true) {
                pin_code[5] = '0' + (X_PIN / 10);
                pin_code[6] = '0' + (X_PIN % 10);
                if (gpio_request(X_PIN, pin_code) == 0) {
                    gpio_direction_input(X_PIN);
                    x_irq_number = gpio_to_irq(X_PIN);
                    if (request_irq(x_irq_number, x_press_interrupt, IRQF_TRIGGER_HIGH, "gpio_controller_device", NULL)) {
                        return -1;//goto unset_x_press;
                    }
                    if (request_irq(x_irq_number, x_release_interrupt, IRQF_TRIGGER_LOW, "gpio_controller_device", NULL)) {
                        return -1;//goto unset_x_release;
                    }
                }
            } else {
                return -1;//goto unset_b_release;
            }

            if (gpio_is_valid(Y_PIN) == true) {
                pin_code[5] = '0' + (Y_PIN / 10);
                pin_code[6] = '0' + (Y_PIN % 10);
                if (gpio_request(Y_PIN, pin_code) == 0) {
                    gpio_direction_input(Y_PIN);
                    y_irq_number = gpio_to_irq(Y_PIN);
                    if (request_irq(y_irq_number, y_press_interrupt, IRQF_TRIGGER_HIGH, "gpio_controller_device", NULL)) {
                        return -1;//goto unset_y_press;
                    }
                    if (request_irq(y_irq_number, y_release_interrupt, IRQF_TRIGGER_LOW, "gpio_controller_device", NULL)) {
                        return -1;//goto unset_y_release;
                    }
                }
            } else {
                return -1;//goto unset_x_release;
            }*/

            return 0;

            /*unset_y_release:
                free_irq(y_irq_number, y_release_interrupt);
            unset_y_press:
                free_irq(y_irq_number, y_press_interrupt);
            unset_x_release:
                free_irq(x_irq_number, x_release_interrupt);
            unset_x_press:
                free_irq(x_irq_number, x_press_interrupt);
            unset_b_release:
                free_irq(b_irq_number, b_release_interrupt);
            unset_b_press:
                free_irq(b_irq_number, b_press_interrupt);
            unset_a_release:
                free_irq(a_irq_number, a_release_interrupt);
            unset_a_press:
                free_irq(a_irq_number, a_press_interrupt);
            unset_select_release:
                free_irq(select_irq_number, select_release_interrupt);
            unset_select_press:
                free_irq(select_irq_number, select_press_interrupt);
            unset_start_release:
                free_irq(start_irq_number, start_release_interrupt);
            unset_start_press:
                free_irq(start_irq_number, start_press_interrupt);
            unset_right_shoulder_release:
                free_irq(right_shoulder_irq_number, right_shoulder_release_interrupt);
            unset_right_shoulder_press:
                free_irq(right_shoulder_irq_number, right_shoulder_press_interrupt);
            unset_left_shoulder_release:
                free_irq(left_shoulder_irq_number, left_shoulder_release_interrupt);
            unset_left_shoulder_press:
                free_irq(left_shoulder_irq_number, left_shoulder_press_interrupt);
            unset_all:
                input_unregister_device(gpio_controller_dev);*/
        } else {
            //input_free_device(gpio_controller_dev);
        }
    }
    return -1;
}

static void __exit gpio_controller_driver_exit(void) {
    /*free_irq(left_shoulder_irq_number, left_shoulder_press_interrupt);
    free_irq(left_shoulder_irq_number, left_shoulder_release_interrupt);
    free_irq(right_shoulder_irq_number, right_shoulder_press_interrupt);
    free_irq(right_shoulder_irq_number, right_shoulder_release_interrupt);
    free_irq(start_irq_number, start_press_interrupt);
    free_irq(start_irq_number, start_release_interrupt);
    free_irq(select_irq_number, select_press_interrupt);
    free_irq(select_irq_number, select_release_interrupt);
    free_irq(a_irq_number, a_press_interrupt);
    free_irq(a_irq_number, a_release_interrupt);
    free_irq(b_irq_number, b_press_interrupt);
    free_irq(b_irq_number, b_release_interrupt);
    free_irq(x_irq_number, x_press_interrupt);
    free_irq(x_irq_number, x_release_interrupt);
    free_irq(y_irq_number, y_press_interrupt);
    free_irq(y_irq_number, y_release_interrupt);
    gpio_free(LEFT_SHOULDER_PIN);
    gpio_free(RIGHT_SHOULDER_PIN);
    gpio_free(START_PIN);
    gpio_free(SELECT_PIN);
    gpio_free(A_PIN);
    gpio_free(B_PIN);
    gpio_free(X_PIN);
    gpio_free(Y_PIN);
    */
    input_unregister_device(gpio_controller_dev);
}

module_init(gpio_controller_driver_init);
module_exit(gpio_controller_driver_exit);

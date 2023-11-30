#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/jiffies.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrew Fox");
MODULE_DESCRIPTION("A driver for a GPIO based custom RetroPie controller");
MODULE_VERSION("0.3");

#define LEFT_SHOULDER_PIN   4
#define RIGHT_SHOULDER_PIN  17
#define START_PIN           22
#define SELECT_PIN          27
#define A_PIN               12
#define B_PIN               25
#define X_PIN               24
#define Y_PIN               23
#define JOYSTICK_CS_PIN     18
#define JOYSTICK_DO_PIN     19
#define JOYSTICK_DI_PIN     20
#define JOYSTICK_CLK_PIN    21
#define SPI_BUS_NUM         1
#define SPI_IRQ_NUM         84 // ???

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

#define DEBOUNCE_TIME       2

extern unsigned long volatile jiffies;

static struct input_dev *gpio_controller_dev;
unsigned int left_shoulder_irq_number;
unsigned int right_shoulder_irq_number;
unsigned int start_irq_number;
unsigned int select_irq_number;
unsigned int a_irq_number;
unsigned int b_irq_number;
unsigned int x_irq_number;
unsigned int y_irq_number;
unsigned long old_left_shoulder_jiffie = 0;
unsigned long old_right_shoulder_jiffie = 0;
unsigned long old_start_jiffie = 0;
unsigned long old_select_jiffie = 0;
unsigned long old_a_jiffie = 0;
unsigned long old_b_jiffie = 0;
unsigned long old_x_jiffie = 0;
unsigned long old_y_jiffie = 0;
int left_shoulder_val = 0;
int right_shoulder_val = 0;
int start_val = 0;
int select_val = 0;
int a_val = 0;
int b_val = 0;
int x_val = 0;
int y_val = 0;
struct spi_master *master;
static struct spi_device *joystick_spi_dev;
struct spi_board_info joystick_spi_dev_info = {
    .modalias = "joystick-spi-adc0832-driver",
    .irq = SPI_IRQ_NUMBER,
    .max_speed_hz = 400000,
    .bus_num = SPI_BUS_NUM,
    .chip_select = 0,
    .mode = SPI_MODE_1
};
int left_key_val = 0;
int right_key_val = 0;
int down_key_val = 0;
int up_key_val = 0;
int i;

static irqreturn_t left_shoulder_interrupt(int irq, void *dummy) {
    static unsigned long flags;
    local_irq_save(flags);
    if (jiffies - old_left_shoulder_jiffie > DEBOUNCE_TIME) {
        if (gpio_get_value(LEFT_SHOULDER_PIN)) {
            left_shoulder_val++;
        } else {
            left_shoulder_val = 0;
        }
        input_report_key(gpio_controller_dev, LEFT_SHOULDER_KEY, left_shoulder_val);
        input_sync(gpio_controller_dev);
        old_left_shoulder_jiffie = jiffies;
    }
    local_irq_restore(flags);
    return IRQ_HANDLED;
}

static irqreturn_t right_shoulder_interrupt(int irq, void *dummy) {
    static unsigned long flags;
    local_irq_save(flags);
    if (jiffies - old_right_shoulder_jiffie > DEBOUNCE_TIME) {
        if (gpio_get_value(RIGHT_SHOULDER_PIN)) {
            right_shoulder_val++;
        } else {
            right_shoulder_val = 0;
        }
        input_report_key(gpio_controller_dev, RIGHT_SHOULDER_KEY, right_shoulder_val);
        input_sync(gpio_controller_dev);
        old_right_shoulder_jiffie = jiffies;
    }
    local_irq_restore(flags);
    return IRQ_HANDLED;
}

static irqreturn_t start_interrupt(int irq, void *dummy) {
    static unsigned long flags;
    local_irq_save(flags);
    if (jiffies - old_start_jiffie > DEBOUNCE_TIME) {
        if (gpio_get_value(START_PIN)) {
            start_val++;
        } else {
            start_val = 0;
        }
        input_report_key(gpio_controller_dev, START_KEY, start_val);
        input_sync(gpio_controller_dev);
        old_start_jiffie = jiffies;
    }
    local_irq_restore(flags);
    return IRQ_HANDLED;
}

static irqreturn_t select_interrupt(int irq, void *dummy) {
    static unsigned long flags;
    local_irq_save(flags);
    if (jiffies - old_select_jiffie > DEBOUNCE_TIME) {
        if (gpio_get_value(SELECT_PIN)) {
            select_val++;
        } else {
            select_val = 0;
        }
        input_report_key(gpio_controller_dev, SELECT_KEY, select_val);
        input_sync(gpio_controller_dev);
        old_select_jiffie = jiffies;
    }
    local_irq_restore(flags);
    return IRQ_HANDLED;
}

static irqreturn_t a_interrupt(int irq, void *dummy) {
    static unsigned long flags;
    local_irq_save(flags);
    if (jiffies - old_a_jiffie > DEBOUNCE_TIME) {
        if (gpio_get_value(A_PIN)) {
            a_val++;
        } else {
            a_val = 0;
        }
        input_report_key(gpio_controller_dev, A_KEY, a_val);
        input_sync(gpio_controller_dev);
        old_a_jiffie = jiffies;
    }
    local_irq_restore(flags);
    return IRQ_HANDLED;
}

static irqreturn_t b_interrupt(int irq, void *dummy) {
    static unsigned long flags;
    local_irq_save(flags);
    if (jiffies - old_b_jiffie > DEBOUNCE_TIME) {
        if (gpio_get_value(B_PIN)) {
            b_val++;
        } else {
            b_val = 0;
        }
        input_report_key(gpio_controller_dev, B_KEY, b_val);
        input_sync(gpio_controller_dev);
        old_b_jiffie = jiffies;
    }
    local_irq_restore(flags);
    return IRQ_HANDLED;
}

static irqreturn_t x_interrupt(int irq, void *dummy) {
    static unsigned long flags;
    local_irq_save(flags);
    if (jiffies - old_x_jiffie > DEBOUNCE_TIME) {
        if (gpio_get_value(X_PIN)) {
            x_val++;
        } else {
            x_val = 0;
        }
        input_report_key(gpio_controller_dev, X_KEY, x_val);
        input_sync(gpio_controller_dev);
        old_x_jiffie = jiffies;
    }
    local_irq_restore(flags);
    return IRQ_HANDLED;
}

static irqreturn_t y_interrupt(int irq, void *dummy) {
    static unsigned long flags;
    local_irq_save(flags);
    if (jiffies - old_y_jiffie > DEBOUNCE_TIME) {
        if (gpio_get_value(Y_PIN)) {
            y_val++;
        } else {
            y_val = 0;
        }
        input_report_key(gpio_controller_dev, Y_KEY, y_val);
        input_sync(gpio_controller_dev);
        old_y_jiffie = jiffies;
    }
    local_irq_restore(flags);
    return IRQ_HANDLED;
}

static irqreturn_t joystick_spi_interrupt(int irq, void *dummy) {
    static unsigned long flags;
    unsigned char x1, x2, y1, y2;
    local_irq_save(flags);

    pr_info("joystick interrupt start");

    gpio_set_value(JOYSTICK_CS_PIN, 1); // enable joystick spi device

    gpio_set_value(JOYSTICK_CLK_PIN, 0);
    gpio_set_value(JOYSTICK_DI_PIN, 1);
    msleep(2);
    gpio_set_value(JOYSTICK_CLK_PIN, 1);
    msleep(2);

    gpio_set_value(JOYSTICK_CLK_PIN, 0);
    gpio_set_value(JOYSTICK_DI_PIN, 1);
    msleep(2);
    gpio_set_value(JOYSTICK_CLK_PIN, 1);
    msleep(2);

    gpio_set_value(JOYSTICK_CLK_PIN, 0);
    gpio_set_value(JOYSTICK_DI_PIN, 0); // x
    msleep(2);
    gpio_set_value(JOYSTICK_CLK_PIN, 1);
    gpio_set_value(JOYSTICK_DI_PIN, 1);
    msleep(2);

    gpio_set_value(JOYSTICK_CLK_PIN, 0);
    gpio_set_value(JOYSTICK_DI_PIN, 1);
    msleep(2);

    for (i = 0; i < 8; i++) {
        gpio_set_value(JOYSTICK_CLK_PIN, 1);
        msleep(2);
        gpio_set_value(JOYSTICK_CLK_PIN, 0);
        msleep(2);
        x1 = (x1 << 1) | gpio_get_value(JOYSTICK_DO_PIN);
    }
    for (i = 0; i < 8; i++) {
        x2 = x2 | (gpio_get_value(JOYSTICK_DO_PIN) << i);
        gpio_set_value(JOYSTICK_CLK_PIN, 1);
        msleep(2);
        gpio_set_value(JOYSTICK_CLK_PIN, 0);
        msleep(2);
    }

    gpio_set_value(JOYSTICK_CLK_PIN, 0);
    gpio_set_value(JOYSTICK_DI_PIN, 1);
    msleep(2);
    gpio_set_value(JOYSTICK_CLK_PIN, 1);
    msleep(2);

    gpio_set_value(JOYSTICK_CLK_PIN, 0);
    gpio_set_value(JOYSTICK_DI_PIN, 1);
    msleep(2);
    gpio_set_value(JOYSTICK_CLK_PIN, 1);
    msleep(2);

    gpio_set_value(JOYSTICK_CLK_PIN, 0);
    gpio_set_value(JOYSTICK_DI_PIN, 1); // y
    msleep(2);
    gpio_set_value(JOYSTICK_CLK_PIN, 1);
    gpio_set_value(JOYSTICK_DI_PIN, 1);
    msleep(2);

    gpio_set_value(JOYSTICK_CLK_PIN, 0);
    gpio_set_value(JOYSTICK_DI_PIN, 1);
    msleep(2);

    for (i = 0; i < 8; i++) {
        gpio_set_value(JOYSTICK_CLK_PIN, 1);
        msleep(2);
        gpio_set_value(JOYSTICK_CLK_PIN, 0);
        msleep(2);
        y1 = (y1 << 1) | gpio_get_value(JOYSTICK_DO_PIN);
    }
    for (i = 0; i < 8; i++) {
        y2 = y2 | (gpio_get_value(JOYSTICK_DO_PIN) << i);
        gpio_set_value(JOYSTICK_CLK_PIN, 1);
        msleep(2);
        gpio_set_value(JOYSTICK_CLK_PIN, 0);
        msleep(2);
    }

    gpio_set_value(JOYSTICK_CS_PIN, 0); // disable joystick spi device

    pr_info("disabled joystick spi device again");

    if (x1 == x2 && y1 == y2) {
        if (x1 < 1) {
            left_key_val++;
        } else {
            left_key_val = 0;
        }
        if (x1 > 254) {
            right_key_val++;
        } else {
            right_key_val = 0;
        }
        if (y1 < 1) {
            down_key_val++;
        } else {
            down_key_val = 0;
        }
        if (y1 > 254) {
            up_key_val++;
        } else {
            up_key_val = 0;
        }
        input_report_key(gpio_controller_dev, LEFT_KEY, left_key_val);
        input_report_key(gpio_controller_dev, RIGHT_KEY, right_key_val);
        input_report_key(gpio_controller_dev, DOWN_KEY, down_key_val);
        input_report_key(gpio_controller_dev, UP_KEY, up_key_val);
        input_sync(gpio_controller_dev);
    }

    pr_info("joystick interrupt end");

    local_irq_restore(flags);
    return IRQ_HANDLED;
}

static int __init gpio_controller_driver_init(void) {
    gpio_controller_dev = input_allocate_device();
    if (gpio_controller_dev) {
        gpio_controller_dev->name = "gpio_controller_device";
        set_bit(EV_KEY, gpio_controller_dev->evbit);
        set_bit(EV_REP, gpio_controller_dev->evbit);
        set_bit(LEFT_SHOULDER_KEY, gpio_controller_dev->keybit);
        set_bit(RIGHT_SHOULDER_KEY, gpio_controller_dev->keybit);
        set_bit(START_KEY, gpio_controller_dev->keybit);
        set_bit(SELECT_KEY, gpio_controller_dev->keybit);
        set_bit(A_KEY, gpio_controller_dev->keybit);
        set_bit(B_KEY, gpio_controller_dev->keybit);
        set_bit(X_KEY, gpio_controller_dev->keybit);
        set_bit(Y_KEY, gpio_controller_dev->keybit);

        if (input_register_device(gpio_controller_dev) == 0) {
            char pin_code[8] = "GPIO_XX\0";

            if (gpio_is_valid(LEFT_SHOULDER_PIN) == true) {
                pin_code[5] = '0' + (LEFT_SHOULDER_PIN / 10);
                pin_code[6] = '0' + (LEFT_SHOULDER_PIN % 10);
                if (gpio_request(LEFT_SHOULDER_PIN, pin_code) == 0) {
                    gpio_direction_input(LEFT_SHOULDER_PIN);
                    left_shoulder_irq_number = gpio_to_irq(LEFT_SHOULDER_PIN);
                    if (request_irq(left_shoulder_irq_number, left_shoulder_interrupt, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "gpio_controller_device", NULL) < 0) {
                        goto unset_left_shoulder;
                    }
                } else {
                    goto unset_left_shoulder_pin;
                }
            } else {
                goto unregister_dev;
            }

            if (gpio_is_valid(RIGHT_SHOULDER_PIN) == true) {
                pin_code[5] = '0' + (RIGHT_SHOULDER_PIN / 10);
                pin_code[6] = '0' + (RIGHT_SHOULDER_PIN % 10);
                if (gpio_request(RIGHT_SHOULDER_PIN, pin_code) == 0) {
                    gpio_direction_input(RIGHT_SHOULDER_PIN);
                    right_shoulder_irq_number = gpio_to_irq(RIGHT_SHOULDER_PIN);
                    if (request_irq(right_shoulder_irq_number, right_shoulder_interrupt, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "gpio_controller_device", NULL) < 0) {
                        goto unset_right_shoulder;
                    }
                } else {
                    goto unset_right_shoulder_pin;
                }
            } else {
                goto unset_left_shoulder;
            }

            if (gpio_is_valid(START_PIN) == true) {
                pin_code[5] = '0' + (START_PIN / 10);
                pin_code[6] = '0' + (START_PIN % 10);
                if (gpio_request(START_PIN, pin_code) == 0) {
                    gpio_direction_input(START_PIN);
                    start_irq_number = gpio_to_irq(START_PIN);
                    if (request_irq(start_irq_number, start_interrupt, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "gpio_controller_device", NULL) < 0) {
                        goto unset_start;
                    }
                } else {
                    goto unset_start_pin;
                }
            } else {
                goto unset_right_shoulder;
            }

            if (gpio_is_valid(SELECT_PIN) == true) {
                pin_code[5] = '0' + (SELECT_PIN / 10);
                pin_code[6] = '0' + (SELECT_PIN % 10);
                if (gpio_request(SELECT_PIN, pin_code) == 0) {
                    gpio_direction_input(SELECT_PIN);
                    select_irq_number = gpio_to_irq(SELECT_PIN);
                    if (request_irq(select_irq_number, select_interrupt, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "gpio_controller_device", NULL) < 0) {
                        goto unset_select;
                    }
                } else {
                    goto unset_select_pin;
                }
            } else {
                goto unset_start;
            }

            if (gpio_is_valid(A_PIN) == true) {
                pin_code[5] = '0' + (A_PIN / 10);
                pin_code[6] = '0' + (A_PIN % 10);
                if (gpio_request(A_PIN, pin_code) == 0) {
                    gpio_direction_input(A_PIN);
                    a_irq_number = gpio_to_irq(A_PIN);
                    if (request_irq(a_irq_number, a_interrupt, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "gpio_controller_device", NULL) < 0) {
                        goto unset_a;
                    }
                } else {
                    goto unset_a_pin;
                }
            } else {
                goto unset_select;
            }

            if (gpio_is_valid(B_PIN) == true) {
                pin_code[5] = '0' + (B_PIN / 10);
                pin_code[6] = '0' + (B_PIN % 10);
                if (gpio_request(B_PIN, pin_code) == 0) {
                    gpio_direction_input(B_PIN);
                    b_irq_number = gpio_to_irq(B_PIN);
                    if (request_irq(b_irq_number, b_interrupt, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "gpio_controller_device", NULL) < 0) {
                        goto unset_b;
                    }
                } else {
                    goto unset_b_pin;
                }
            } else {
                goto unset_a;
            }

            if (gpio_is_valid(X_PIN) == true) {
                pin_code[5] = '0' + (X_PIN / 10);
                pin_code[6] = '0' + (X_PIN % 10);
                if (gpio_request(X_PIN, pin_code) == 0) {
                    gpio_direction_input(X_PIN);
                    x_irq_number = gpio_to_irq(X_PIN);
                    if (request_irq(x_irq_number, x_interrupt, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "gpio_controller_device", NULL) < 0) {
                        goto unset_x;
                    }
                } else {
                    goto unset_x_pin;
                }
            } else {
                goto unset_b;
            }

            if (gpio_is_valid(Y_PIN) == true) {
                pin_code[5] = '0' + (Y_PIN / 10);
                pin_code[6] = '0' + (Y_PIN % 10);
                if (gpio_request(Y_PIN, pin_code) == 0) {
                    gpio_direction_input(Y_PIN);
                    y_irq_number = gpio_to_irq(Y_PIN);
                    if (request_irq(y_irq_number, y_interrupt, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "gpio_controller_device", NULL) < 0) {
                        goto unset_y;
                    }
                } else {
                    goto unset_y_pin;
                }
            } else {
                goto unset_x;
            }

            pr_info("starting joystick pin init");

            if (gpio_is_valid(JOYSTICK_CS_PIN) == true) {
                pin_code[5] = '0' + (JOYSTICK_CS_PIN / 10);
                pin_code[6] = '0' + (JOYSTICK_CS_PIN % 10);
                if (gpio_request(JOYSTICK_CS_PIN, pin_code) == 0) {
                    gpio_direction_output(JOYSTICK_CS_PIN, 1);
                } else {
                    goto unset_joystick_cs_pin;
                }
            } else {
                goto unset_y;
            }

            if (gpio_is_valid(JOYSTICK_CLK_PIN) == true) {
                pin_code[5] = '0' + (JOYSTICK_CLK_PIN / 10);
                pin_code[6] = '0' + (JOYSTICK_CLK_PIN % 10);
                if (gpio_request(JOYSTICK_CLK_PIN, pin_code) == 0) {
                    gpio_direction_output(JOYSTICK_CLK_PIN, 0);
                } else {
                    goto unset_joystick_clk_pin;
                }
            } else {
                goto unset_joystick_cs_pin;
            }

            if (gpio_is_valid(JOYSTICK_DO_PIN) == true) {
                pin_code[5] = '0' + (JOYSTICK_DO_PIN / 10);
                pin_code[6] = '0' + (JOYSTICK_DO_PIN % 10);
                if (gpio_request(JOYSTICK_DO_PIN, pin_code) == 0) {
                    gpio_direction_input(JOYSTICK_DO_PIN);
                } else {
                    goto unset_joystick_do_pin;
                }
            } else {
                goto unset_joystick_clk_pin;
            }

            if (gpio_is_valid(JOYSTICK_DI_PIN) == true) {
                pin_code[5] = '0' + (JOYSTICK_DI_PIN / 10);
                pin_code[6] = '0' + (JOYSTICK_DI_PIN % 10);
                if (gpio_request(JOYSTICK_DI_PIN, pin_code) == 0) {
                    gpio_direction_output(JOYSTICK_DI_PIN, 0);
                } else {
                    goto unset_joystick_di_pin;
                }
            } else {
                goto unset_joystick_do_pin;
            }

            pr_info("finished joystick pin init");

            master = spi_busnum_to_master(1);
            if (master == NULL) {
                goto unset_y;
            }
            joystick_spi_dev = spi_new_device(master, &joystick_spi_dev_info);
            if (joystick_spi_dev == NULL) {
                goto unset_y;
            }
            joystick_spi_dev->bits_per_word = 8;
            if (spi_setup(joystick_spi_dev)) {
                goto unset_joystick;
            }

            pr_info("finished joystick device init");

            if (request_irq(SPI_IRQ_NUMBER, joystick_spi_interrupt, IRQF_SHARED, "gpio_controller_device", NULL) < 0) {
                goto unset_joystick_irq;
            }

            pr_info("finished joystick irq init");

            return 0;

            unset_joystick_irq:
                free_irq(SPI_IRQ_NUMBER, joystick_spi_interrupt);
            unset_joystick_di_pin:
                gpio_free(JOYSTICK_DI_PIN);
            unset_joystick_do_pin:
                gpio_free(JOYSTICK_DO_PIN);
            unset_joystick_clk_pin:
                gpio_free(JOYSTICK_CLK_PIN);
            unset_joystick_cs_pin:
                gpio_free(JOYSTICK_CS_PIN);
            unset_joystick:
                spi_unregister_device(joystick_spi_dev);
            unset_y:
                free_irq(y_irq_number, y_interrupt);
            unset_y_pin:
                gpio_free(Y_PIN);
            unset_x:
                free_irq(x_irq_number, x_interrupt);
            unset_x_pin:
                gpio_free(X_PIN);
            unset_b:
                free_irq(b_irq_number, b_interrupt);
            unset_b_pin:
                gpio_free(B_PIN);
            unset_a:
                free_irq(a_irq_number, a_interrupt);
            unset_a_pin:
                gpio_free(A_PIN);
            unset_select:
                free_irq(select_irq_number, select_interrupt);
            unset_select_pin:
                gpio_free(SELECT_PIN);
            unset_start:
                free_irq(start_irq_number, start_interrupt);
            unset_start_pin:
                gpio_free(START_PIN);
            unset_right_shoulder:
                free_irq(right_shoulder_irq_number, right_shoulder_interrupt);
            unset_right_shoulder_pin:
                gpio_free(RIGHT_SHOULDER_PIN);
            unset_left_shoulder:
                free_irq(left_shoulder_irq_number, left_shoulder_interrupt);
            unset_left_shoulder_pin:
                gpio_free(LEFT_SHOULDER_PIN);
            unregister_dev:
                input_unregister_device(gpio_controller_dev);
        } else {
            input_free_device(gpio_controller_dev);
        }
    }
    return -1;
}

static void __exit gpio_controller_driver_exit(void) { // note: this doesn't work?
    free_irq(SPI_IRQ_NUMBER, joystick_spi_interrupt);
    spi_unregister_device(joystick_spi_dev);
    free_irq(left_shoulder_irq_number, left_shoulder_interrupt);
    free_irq(right_shoulder_irq_number, right_shoulder_interrupt);
    free_irq(start_irq_number, start_interrupt);
    free_irq(select_irq_number, select_interrupt);
    free_irq(a_irq_number, a_interrupt);
    free_irq(b_irq_number, b_interrupt);
    free_irq(x_irq_number, x_interrupt);
    free_irq(y_irq_number, y_interrupt);
    gpio_free(LEFT_SHOULDER_PIN);
    gpio_free(RIGHT_SHOULDER_PIN);
    gpio_free(START_PIN);
    gpio_free(SELECT_PIN);
    gpio_free(A_PIN);
    gpio_free(B_PIN);
    gpio_free(X_PIN);
    gpio_free(Y_PIN);
    input_unregister_device(gpio_controller_dev);
}

module_init(gpio_controller_driver_init);
module_exit(gpio_controller_driver_exit);

#include <linux/kernel.h>
#include <linux/input-polldev.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/jiffies.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>
#include "dev_info.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrew Fox");
MODULE_DESCRIPTION("A driver for a GPIO based custom RetroPie controller");
MODULE_VERSION("0.4");

#define LEFT_SHOULDER_PIN   4
#define RIGHT_SHOULDER_PIN  17
#define START_PIN           22
#define SELECT_PIN          27
#define A_PIN               12
#define B_PIN               25
#define X_PIN               24
#define Y_PIN               23
#define JOYSTICK_CS_PIN     18
#define JOYSTICK_DOI_PIN    20
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

static struct input_dev *controller_buttons_dev;
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
    .irq = SPI_IRQ_NUM,
    .max_speed_hz = 500000,//400000,
    .bus_num = SPI_BUS_NUM,
    .chip_select = 0,
    .mode = SPI_MODE_0//3
};
int left_key_val = 0;
int right_key_val = 0;
int down_key_val = 0;
int up_key_val = 0;
int i;
static struct input_polled_dev *controller_joystick_dev;

bool buttons_device_allocated = false;
bool buttons_device_registered = false;
bool spi_device_registered = false;
bool joystick_device_allocated = false;
bool joystick_device_registered = false;

bool left_shoulder_pin_requested = false;
bool right_shoulder_pin_requested = false;
bool start_pin_requested = false;
bool select_pin_requested = false;
bool a_pin_requested = false;
bool b_pin_requested = false;
bool x_pin_requested = false;
bool y_pin_requested = false;
bool joystick_cs_pin_requested = false;
bool joystick_clk_pin_requested = false;
bool joystick_doi_pin_requested = false;

bool left_shoulder_irq_set = false;
bool right_shoulder_irq_set = false;
bool start_irq_set = false;
bool select_irq_set = false;
bool a_irq_set = false;
bool b_irq_set = false;
bool x_irq_set = false;
bool y_irq_set = false;

static irqreturn_t left_shoulder_interrupt(int irq, void *dummy) {
    static unsigned long flags;
    local_irq_save(flags);
    if (jiffies - old_left_shoulder_jiffie > DEBOUNCE_TIME) {
        if (gpio_get_value(LEFT_SHOULDER_PIN)) {
            left_shoulder_val++;
        } else {
            left_shoulder_val = 0;
        }
        input_report_key(controller_buttons_dev, LEFT_SHOULDER_KEY, left_shoulder_val);
        input_sync(controller_buttons_dev);
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
        input_report_key(controller_buttons_dev, RIGHT_SHOULDER_KEY, right_shoulder_val);
        input_sync(controller_buttons_dev);
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
        input_report_key(controller_buttons_dev, START_KEY, start_val);
        input_sync(controller_buttons_dev);
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
        input_report_key(controller_buttons_dev, SELECT_KEY, select_val);
        input_sync(controller_buttons_dev);
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
        input_report_key(controller_buttons_dev, A_KEY, a_val);
        input_sync(controller_buttons_dev);
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
        input_report_key(controller_buttons_dev, B_KEY, b_val);
        input_sync(controller_buttons_dev);
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
        input_report_key(controller_buttons_dev, X_KEY, x_val);
        input_sync(controller_buttons_dev);
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
        input_report_key(controller_buttons_dev, Y_KEY, y_val);
        input_sync(controller_buttons_dev);
        old_y_jiffie = jiffies;
    }
    local_irq_restore(flags);
    return IRQ_HANDLED;
}

static void joystick_spi_poll(struct input_polled_dev *dev) {
    unsigned char x1, x2, y1, y2;
    pr_info("joystick poll start\n");

    gpio_set_value(JOYSTICK_CS_PIN, 0);
    // Start Sequence
    gpio_set_value(JOYSTICK_CLK_PIN, 0);
    gpio_direction_output(JOYSTICK_DOI_PIN, 1);
    msleep(ADC0832DELAY);
    gpio_set_value(JOYSTICK_CLK_PIN, 1);
    msleep(ADC0832DELAY);
    gpio_set_value(JOYSTICK_CLK_PIN, 0);
    gpio_set_value(JOYSTICK_DOI_PIN, ADC0832_MUX_DIFFERENTIAL);
    msleep(ADC0832DELAY);
    gpio_set_value(JOYSTICK_CLK_PIN, 1);
    msleep(ADC0832DELAY);
    // Send Sequence
    gpio_set_value(JOYSTICK_CLK_PIN, 0);
    gpio_set_value(JOYSTICK_DOI_PIN, PS2JOYSTICK_X_AXIS);
    msleep(ADC0832DELAY);
    gpio_set_value(JOYSTICK_CLK_PIN, 1);
    gpio_set_value(JOYSTICK_DOI_PIN, 1);
    msleep(ADC0832DELAY);
    // Receive Sequence
    gpio_set_value(JOYSTICK_CLK_PIN, 0);
    gpio_set_value(JOYSTICK_DOI_PIN, 1);
    msleep(ADC0832DELAY);
    gpio_direction_input(JOYSTICK_DOI_PIN);
    for (i = 0; i < 8; i++) {
        gpio_set_value(JOYSTICK_CLK_PIN, 1);
        msleep(ADC0832DELAY);
        gpio_set_value(JOYSTICK_CLK_PIN, 0);
        msleep(ADC0832DELAY);
        x1 = (x1 << 1) | gpio_get_value(JOYSTICK_DOI_PIN);
    }
    for (i = 0; i < 8; i++) {
        x2 = x2 | (gpio_get_value(JOYSTICK_DOI_PIN) << i);
        gpio_set_value(JOYSTICK_CLK_PIN, 1);
        msleep(ADC0832DELAY);
        gpio_set_value(JOYSTICK_CLK_PIN, 0);
        msleep(ADC0832DELAY);
    }
    // Reset Sequence
    gpio_set_value(JOYSTICK_CS_PIN, 1);
    msleep(ADC0832DELAY);
    gpio_set_value(JOYSTICK_CS_PIN, 0);
    // Start Sequence
    gpio_set_value(JOYSTICK_CLK_PIN, 0);
    gpio_direction_output(JOYSTICK_DOI_PIN, 1);
    msleep(ADC0832DELAY);
    gpio_set_value(JOYSTICK_CLK_PIN, 1);
    msleep(ADC0832DELAY);
    gpio_set_value(JOYSTICK_CLK_PIN, 0);
    gpio_set_value(JOYSTICK_DOI_PIN, ADC0832_MUX_DIFFERENTIAL);
    msleep(ADC0832DELAY);
    gpio_set_value(JOYSTICK_CLK_PIN, 1);
    msleep(ADC0832DELAY);
    // Send Sequence
    gpio_set_value(JOYSTICK_CLK_PIN, 0);
    gpio_set_value(JOYSTICK_DOI_PIN, PS2JOYSTICK_Y_AXIS);
    msleep(ADC0832DELAY);
    gpio_set_value(JOYSTICK_CLK_PIN, 1);
    gpio_set_value(JOYSTICK_DOI_PIN, 1);
    msleep(ADC0832DELAY);
    // Receive Sequence
    gpio_set_value(JOYSTICK_CLK_PIN, 0);
    gpio_set_value(JOYSTICK_DOI_PIN, 1);
    msleep(ADC0832DELAY);
    gpio_direction_input(JOYSTICK_DOI_PIN);
    for (i = 0; i < 8; i++) {
        gpio_set_value(JOYSTICK_CLK_PIN, 1);
        msleep(ADC0832DELAY);
        gpio_set_value(JOYSTICK_CLK_PIN, 0);
        msleep(ADC0832DELAY);
        y1 = (y1 << 1) | gpio_get_value(JOYSTICK_DOI_PIN);
    }
    for (i = 0; i < 8; i++) {
        y2 = y2 | (gpio_get_value(JOYSTICK_DOI_PIN) << i);
        gpio_set_value(JOYSTICK_CLK_PIN, 1);
        msleep(ADC0832DELAY);
        gpio_set_value(JOYSTICK_CLK_PIN, 0);
        msleep(ADC0832DELAY);
    }
    // End Sequence
    gpio_set_value(JOYSTICK_CS_PIN, 1);

    pr_info("disabled joystick spi device again\n");

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
        input_report_key(controller_buttons_dev, LEFT_KEY, left_key_val);
        input_report_key(controller_buttons_dev, RIGHT_KEY, right_key_val);
        input_report_key(controller_buttons_dev, DOWN_KEY, down_key_val);
        input_report_key(controller_buttons_dev, UP_KEY, up_key_val);
        input_sync(controller_buttons_dev);
    }

    pr_info("joystick poll end\n");
}

static void unallocate_all(void) {
    if (y_irq_set) {free_irq(y_irq_number, NULL);}
    if (x_irq_set) {free_irq(x_irq_number, NULL);}
    if (b_irq_set) {free_irq(b_irq_number, NULL);}
    if (a_irq_set) {free_irq(a_irq_number, NULL);}
    if (select_irq_set) {free_irq(select_irq_number, NULL);}
    if (start_irq_set) {free_irq(start_irq_number, NULL);}
    if (right_shoulder_irq_set) {free_irq(right_shoulder_irq_number, NULL);}
    if (left_shoulder_irq_set) {free_irq(left_shoulder_irq_number, NULL);}

    if (joystick_doi_pin_requested) {gpio_free(JOYSTICK_DOI_PIN);}
    if (joystick_clk_pin_requested) {gpio_free(JOYSTICK_CLK_PIN);}
    if (joystick_cs_pin_requested) {gpio_free(JOYSTICK_CS_PIN);}
    if (y_pin_requested) {gpio_free(Y_PIN);}
    if (x_pin_requested) {gpio_free(X_PIN);}
    if (b_pin_requested) {gpio_free(B_PIN);}
    if (a_pin_requested) {gpio_free(A_PIN);}
    if (select_pin_requested) {gpio_free(SELECT_PIN);}
    if (start_pin_requested) {gpio_free(START_PIN);}
    if (right_shoulder_pin_requested) {gpio_free(RIGHT_SHOULDER_PIN);}
    if (left_shoulder_pin_requested) {gpio_free(LEFT_SHOULDER_PIN);}

    if (joystick_device_registered) {
        input_unregister_polled_device(controller_joystick_dev);
    } else if (joystick_device_allocated) {
        input_free_polled_device(controller_joystick_dev);
    }
    if (spi_device_registered) {spi_unregister_device(joystick_spi_dev);}
    if (buttons_device_registered) {
        input_unregister_device(controller_buttons_dev);
    } else if (buttons_device_allocated) {
        input_free_device(controller_buttons_dev);
    }
}

static int __init gpio_controller_driver_init(void) {
    controller_buttons_dev = input_allocate_device();
    if (controller_buttons_dev) {
        buttons_device_allocated = true;

        controller_buttons_dev->name = "controller_buttons_device";
        set_bit(EV_KEY, controller_buttons_dev->evbit);
        set_bit(EV_REP, controller_buttons_dev->evbit);
        set_bit(LEFT_SHOULDER_KEY, controller_buttons_dev->keybit);
        set_bit(RIGHT_SHOULDER_KEY, controller_buttons_dev->keybit);
        set_bit(START_KEY, controller_buttons_dev->keybit);
        set_bit(SELECT_KEY, controller_buttons_dev->keybit);
        set_bit(A_KEY, controller_buttons_dev->keybit);
        set_bit(B_KEY, controller_buttons_dev->keybit);
        set_bit(X_KEY, controller_buttons_dev->keybit);
        set_bit(Y_KEY, controller_buttons_dev->keybit);
        set_bit(LEFT_KEY, controller_buttons_dev->keybit);
        set_bit(RIGHT_KEY, controller_buttons_dev->keybit);
        set_bit(DOWN_KEY, controller_buttons_dev->keybit);
        set_bit(UP_KEY, controller_buttons_dev->keybit);

        if (input_register_device(controller_buttons_dev) == 0) {
            char pin_code[8] = "GPIO_XX\0";
            buttons_device_registered = true;

            if (gpio_is_valid(LEFT_SHOULDER_PIN) == false) {goto init_fail;}
            pin_code[5] = '0' + (LEFT_SHOULDER_PIN / 10);
            pin_code[6] = '0' + (LEFT_SHOULDER_PIN % 10);
            if (gpio_request(LEFT_SHOULDER_PIN, pin_code) < 0) {goto init_fail;}
            left_shoulder_pin_requested = true;
            gpio_direction_input(LEFT_SHOULDER_PIN);
            left_shoulder_irq_number = gpio_to_irq(LEFT_SHOULDER_PIN);
            if (request_irq(left_shoulder_irq_number, left_shoulder_interrupt, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "controller_buttons_device", NULL) < 0) {goto init_fail;}
            left_shoulder_irq_set = true;

            if (gpio_is_valid(RIGHT_SHOULDER_PIN) == false) {goto init_fail;}
            pin_code[5] = '0' + (RIGHT_SHOULDER_PIN / 10);
            pin_code[6] = '0' + (RIGHT_SHOULDER_PIN % 10);
            if (gpio_request(RIGHT_SHOULDER_PIN, pin_code) < 0) {goto init_fail;}
            right_shoulder_pin_requested = true;
            gpio_direction_input(RIGHT_SHOULDER_PIN);
            right_shoulder_irq_number = gpio_to_irq(RIGHT_SHOULDER_PIN);
            if (request_irq(right_shoulder_irq_number, right_shoulder_interrupt, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "controller_buttons_device", NULL) < 0) {goto init_fail;}
            right_shoulder_irq_set = true;

            if (gpio_is_valid(START_PIN) == false) {goto init_fail;}
            pin_code[5] = '0' + (START_PIN / 10);
            pin_code[6] = '0' + (START_PIN % 10);
            if (gpio_request(START_PIN, pin_code) < 0) {goto init_fail;}
            start_pin_requested = true;
            gpio_direction_input(START_PIN);
            start_irq_number = gpio_to_irq(START_PIN);
            if (request_irq(start_irq_number, start_interrupt, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "controller_buttons_device", NULL) < 0) {goto init_fail;}
            start_irq_set = true;

            if (gpio_is_valid(SELECT_PIN) == false) {goto init_fail;}
            pin_code[5] = '0' + (SELECT_PIN / 10);
            pin_code[6] = '0' + (SELECT_PIN % 10);
            if (gpio_request(SELECT_PIN, pin_code) < 0) {goto init_fail;}
            select_pin_requested = true;
            gpio_direction_input(SELECT_PIN);
            select_irq_number = gpio_to_irq(SELECT_PIN);
            if (request_irq(select_irq_number, select_interrupt, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "controller_buttons_device", NULL) < 0) {goto init_fail;}
            select_irq_set = true;

            if (gpio_is_valid(A_PIN) == false) {goto init_fail;}
            pin_code[5] = '0' + (A_PIN / 10);
            pin_code[6] = '0' + (A_PIN % 10);
            if (gpio_request(A_PIN, pin_code) < 0) {goto init_fail;}
            a_pin_requested = true;
            gpio_direction_input(A_PIN);
            a_irq_number = gpio_to_irq(A_PIN);
            if (request_irq(a_irq_number, a_interrupt, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "controller_buttons_device", NULL) < 0) {goto init_fail;}
            a_irq_set = true;

            if (gpio_is_valid(B_PIN) == false) {goto init_fail;}
            pin_code[5] = '0' + (B_PIN / 10);
            pin_code[6] = '0' + (B_PIN % 10);
            if (gpio_request(B_PIN, pin_code) < 0) {goto init_fail;}
            b_pin_requested = true;
            gpio_direction_input(B_PIN);
            b_irq_number = gpio_to_irq(B_PIN);
            if (request_irq(b_irq_number, b_interrupt, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "controller_buttons_device", NULL) < 0) {goto init_fail;}
            b_irq_set = true;

            if (gpio_is_valid(X_PIN) == false) {goto init_fail;}
            pin_code[5] = '0' + (X_PIN / 10);
            pin_code[6] = '0' + (X_PIN % 10);
            if (gpio_request(X_PIN, pin_code) < 0) {goto init_fail;}
            x_pin_requested = true;
            gpio_direction_input(X_PIN);
            x_irq_number = gpio_to_irq(X_PIN);
            if (request_irq(x_irq_number, x_interrupt, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "controller_buttons_device", NULL) < 0) {goto init_fail;}
            x_irq_set = true;

            if (gpio_is_valid(Y_PIN) == false) {goto init_fail;}
            pin_code[5] = '0' + (Y_PIN / 10);
            pin_code[6] = '0' + (Y_PIN % 10);
            if (gpio_request(Y_PIN, pin_code) < 0) {goto init_fail;}
            y_pin_requested = true;
            gpio_direction_input(Y_PIN);
            y_irq_number = gpio_to_irq(Y_PIN);
            if (request_irq(y_irq_number, y_interrupt, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "controller_buttons_device", NULL) < 0) {goto init_fail;}
            y_irq_set = true;

            pr_info("starting joystick pin init\n");

            if (gpio_is_valid(JOYSTICK_CS_PIN) == false) {goto init_fail;}
            pin_code[5] = '0' + (JOYSTICK_CS_PIN / 10);
            pin_code[6] = '0' + (JOYSTICK_CS_PIN % 10);
            if (gpio_request(JOYSTICK_CS_PIN, pin_code) < 0) {goto init_fail;}
            joystick_cs_pin_requested = true;
            gpio_direction_output(JOYSTICK_CS_PIN, 1);

            if (gpio_is_valid(JOYSTICK_CLK_PIN) == false) {goto init_fail;}
            pin_code[5] = '0' + (JOYSTICK_CLK_PIN / 10);
            pin_code[6] = '0' + (JOYSTICK_CLK_PIN % 10);
            if (gpio_request(JOYSTICK_CLK_PIN, pin_code) < 0) {goto init_fail;}
            joystick_clk_pin_requested = true;
            gpio_direction_output(JOYSTICK_CLK_PIN, 0);

            if (gpio_is_valid(JOYSTICK_DOI_PIN) == false) {goto init_fail;}
            pin_code[5] = '0' + (JOYSTICK_DOI_PIN / 10);
            pin_code[6] = '0' + (JOYSTICK_DOI_PIN % 10);
            if (gpio_request(JOYSTICK_DOI_PIN, pin_code) < 0) {goto init_fail;}
            joystick_doi_pin_requested = true;

            pr_info("finished joystick pin init\n");

            master = spi_busnum_to_master(SPI_BUS_NUM);
            if (master == NULL) {goto init_fail;}

            pr_info("got master\n");

            joystick_spi_dev = spi_new_device(master, &joystick_spi_dev_info);
            if (joystick_spi_dev == NULL) {goto init_fail;}
            spi_device_registered = true;

            pr_info("registered spi device\n");

            joystick_spi_dev->bits_per_word = 8;
            if (spi_setup(joystick_spi_dev)) {goto init_fail;}

            pr_info("finished spi setup\n");

            controller_joystick_dev = input_allocate_polled_device();
            if (!controller_joystick_dev) {goto init_fail;}
            joystick_device_allocated = true;

            pr_info("allocated input poll device\n");

            //controller_joystick_dev->poll = joystick_spi_poll;
            controller_joystick_dev->input = controller_buttons_dev;
            if (input_register_device(controller_buttons_dev) < 0) {goto init_fail;}
            joystick_device_registered = true;

            pr_info("finished joystick input device init\n");

            return 0;
        }
    }
init_fail:
    unallocate_all();
    return -1;
}

static void __exit gpio_controller_driver_exit(void) {
    unallocate_all();
}

module_init(gpio_controller_driver_init);
module_exit(gpio_controller_driver_exit);

/* main.c - Hello World ADC demo */

/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>

/* 1000 ms = 1s */
#define SLEEP_TIME_0_MS		1000
#define SLEEP_TIME_1_MS		500
#define SLEEP_TIME_2_MS		250

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0fabian)
#define LED1_NODE DT_ALIAS(led1fabian)
#define LED2_NODE DT_ALIAS(led2fabian)

#if DT_NODE_HAS_STATUS(LED0_NODE, okay)
#define LED0	DT_GPIO_LABEL(LED0_NODE, gpios)
#define PIN0	DT_GPIO_PIN(LED0_NODE, gpios)
#if DT_PHA_HAS_CELL(LED0_NODE, gpios, flags)
#define FLAGS	DT_GPIO_FLAGS(LED0_NODE, gpios)
#endif
#else
/* A build error here means your board isn't set up to blink an LED. */
#error "Unsupported board: led0 devicetree alias is not defined"
#define LED0	""
#define PIN0	0
#endif

#ifndef FLAGS
#define FLAGS	0
#endif

#define LED1 DT_GPIO_LABEL(LED1_NODE,gpios)
#define LED2 DT_GPIO_LABEL(LED2_NODE,gpios)

#define PIN1	DT_GPIO_PIN(LED1_NODE, gpios)
#define PIN2	DT_GPIO_PIN(LED2_NODE, gpios)

void main(void) {
	struct device *dev0;
	struct device *dev1;
	struct device *dev2;
	bool led0_is_on = true;
	bool led1_is_on = true;
	bool led2_is_on = true;
	int ret0;
	int ret1;
	int ret2;

	// Initializing the deviec to control GPIO
	dev0 = device_get_binding(LED0);
	dev1 = device_get_binding(LED1);
	dev2 = device_get_binding(LED2);
	if (dev0 == NULL) {
		printk("LED0 No inicializado");
		return;
	}
	if (dev1 == NULL) {
		printk("LED1 No inicializado");
		return;
	}
	if (dev2 == NULL) {
		printk("LED2 No inicializado");
		return;
	}

	// configuring device to work as output pin
	ret0 = gpio_pin_configure(dev0, PIN0, GPIO_OUTPUT_ACTIVE | FLAGS);
	if (ret0 < 0) {
		return;
	}

	ret1 = gpio_pin_configure(dev1, PIN1, GPIO_OUTPUT_ACTIVE | FLAGS);
	if (ret1 < 0) {
		return;
	}

	ret2 = gpio_pin_configure(dev2, PIN2, GPIO_OUTPUT_ACTIVE | FLAGS);
	if (ret2 < 0) {
		return;
	}

	while (1) {
		gpio_pin_set(dev0, PIN0, (int) led0_is_on);
		led0_is_on = !led0_is_on;
		gpio_pin_set(dev1, PIN1, (int) led1_is_on);
		led1_is_on = !led1_is_on;
		gpio_pin_set(dev2, PIN2, (int) led2_is_on);
		led2_is_on = !led2_is_on;
		k_msleep(SLEEP_TIME_0_MS);

		printk("My name is Fabian, Example Blinky LEDs\n");
	}

}

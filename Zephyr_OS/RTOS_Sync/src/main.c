/* main.c - Hello World demo */

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

/*
 * The hello world demo has two threads that utilize semaphores and sleeping
 * to take turns printing a greeting message at a controlled rate. The demo
 * shows both the static and dynamic approaches for spawning a thread; a real
 * world application would likely use the static approach for both threads.
 */

/* size of stack area used by each thread */
#define STACKSIZE 1024

/* scheduling priority used by each thread */
#define PRIORITY 7

/* delay between greetings (in ms) */
#define SLEEPTIME 500

/* definition of Leds and button */
#define LED0_NODE DT_ALIAS(led0fabian)
#define LED1_NODE DT_ALIAS(led1fabian)
#define LED2_NODE DT_ALIAS(led2fabian)

#define SW0_NODE  DT_ALIAS(sw0fabian)

/*
 * Devicetree helper macro which gets the 'flags' cell from a 'gpios'
 * property, or returns 0 if the property has no 'flags' cell.
 */

#define FLAGS_OR_ZERO(node)						\
	COND_CODE_1(DT_PHA_HAS_CELL(node, gpios, flags),		\
		    (DT_GPIO_FLAGS(node, gpios)),			\
		    (0))

// for LED 0
#if DT_NODE_HAS_STATUS(LED0_NODE, okay) && DT_NODE_HAS_PROP(LED0_NODE, gpios)
#define LED0_GPIO_LABEL	DT_GPIO_LABEL(LED0_NODE, gpios)
#define LED0_GPIO_PIN	DT_GPIO_PIN(LED0_NODE, gpios)
#define LED0_GPIO_FLAGS	(GPIO_OUTPUT | FLAGS_OR_ZERO(LED0_NODE))
#endif

// for LED 1
#if DT_NODE_HAS_STATUS(LED1_NODE, okay) && DT_NODE_HAS_PROP(LED1_NODE, gpios)
#define LED1_GPIO_LABEL	DT_GPIO_LABEL(LED1_NODE, gpios)
#define LED1_GPIO_PIN	DT_GPIO_PIN(LED1_NODE, gpios)
#define LED1_GPIO_FLAGS	(GPIO_OUTPUT | FLAGS_OR_ZERO(LED1_NODE))
#endif

// for LED 2
#if DT_NODE_HAS_STATUS(LED2_NODE, okay) && DT_NODE_HAS_PROP(LED2_NODE, gpios)
#define LED2_GPIO_LABEL	DT_GPIO_LABEL(LED2_NODE, gpios)
#define LED2_GPIO_PIN	DT_GPIO_PIN(LED2_NODE, gpios)
#define LED2_GPIO_FLAGS	(GPIO_OUTPUT | FLAGS_OR_ZERO(LED2_NODE))
#endif

// for SW 0
#if DT_NODE_HAS_STATUS(SW0_NODE, okay)
#define SW0_GPIO_LABEL	DT_GPIO_LABEL(SW0_NODE, gpios)
#define SW0_GPIO_PIN	DT_GPIO_PIN(SW0_NODE, gpios)
#define SW0_GPIO_FLAGS	(GPIO_INPUT | FLAGS_OR_ZERO(SW0_NODE))
#endif

// necessary struct for hardware interruption handle
static struct gpio_callback button_cb_data;

/*
 * @param my_name      thread identification string
 * @param my_sem       thread's own semaphore
 * @param other_sem    other thread's semaphore
 */

// Creation of binary semaphore
K_SEM_DEFINE(sem_Led, 0, 1);

void buttonPressed(struct device *dev, struct gpio_callback *cb, uint32_t pins) {
	printk("Button pressed at %" PRIu32 " cycles\n", k_cycle_get_32());
	k_sem_give(&sem_Led);   // semaphore delivered
	printk("Semaphore delivered\n");
}

void taskLed0(void) {
	// task definitions
	struct device *led;
	int ret;
	int cnt = 0;

	// Led configuration
	led = device_get_binding(LED0_GPIO_LABEL);
	if (led == NULL) {
		printk("Didn't find LED device %s\n", LED0_GPIO_LABEL);
		return;
	}

	ret = gpio_pin_configure(led, LED0_GPIO_PIN, LED0_GPIO_FLAGS);
	if (ret != 0) {
		printk("Error %d: failed to configure LED device %s pin %d\n", ret,
				LED0_GPIO_LABEL, LED0_GPIO_PIN);
		return;
	}

	// main loop task 0
	while (1) {
		printk("Task LED 0 execution\n");
		gpio_pin_set(led, LED0_GPIO_PIN, cnt % 2);
		cnt++;

		// task delay
		k_msleep(500);
	}
}

void taskLed1(void) {
	// definiciones de la tarea
	struct device *led;
	int ret;
	int cnt = 0;

	// configuracion del led
	led = device_get_binding(LED1_GPIO_LABEL);
	if (led == NULL) {
		printk("Didn't find LED device %s\n", LED1_GPIO_LABEL);
		return;
	}

	ret = gpio_pin_configure(led, LED1_GPIO_PIN, LED1_GPIO_FLAGS);
	if (ret != 0) {
		printk("Error %d: failed to configure LED device %s pin %d\n", ret,
				LED1_GPIO_LABEL, LED1_GPIO_PIN);
		return;
	}

	// main loop task 2
	while (1) {
		printk("Task LED 1 execution\n");
		gpio_pin_set(led, LED1_GPIO_PIN, cnt % 2);
		cnt++;

		//k_sem_give(&sem_Led);  // semaforo entregado
		//printk("Semaforo entregado\n");
		// delay de la tarea
		k_msleep(250);
	}
}

void taskLed2(void) {
	// definiciones de la tarea
	struct device *led;
	int ret;
	//int cnt = 0;

	// configuracion del led
	led = device_get_binding(LED2_GPIO_LABEL);
	if (led == NULL) {
		printk("Didn't find LED device %s\n", LED2_GPIO_LABEL);
		return;
	}

	ret = gpio_pin_configure(led, LED2_GPIO_PIN, LED2_GPIO_FLAGS);
	if (ret != 0) {
		printk("Error %d: failed to configure LED device %s pin %d\n", ret,
				LED2_GPIO_LABEL, LED2_GPIO_PIN);
		return;
	}

	// main loop task 3
	while (1) {

		// if (k_sem_take(&sem_Led, K_MSEC(50)) != 0) {
		if (k_sem_take(&sem_Led, K_FOREVER) != 0) {
			printk("Semaphore was not received\n");
		} else {
			/* fetch available data */
			printk("Semaphore received, executed task\n");

			printk("Task LED 2 execution\n");
			gpio_pin_set(led, LED2_GPIO_PIN, 1);
			// cnt++;
		}

		// delay de la tarea
		k_msleep(1000);
		gpio_pin_set(led, LED2_GPIO_PIN, 0);
	}
}

// This task will be used just to create an interruption
void taskSwi0(void) {
	// definition for tasks
	struct device *button;
	int ret;

	button = device_get_binding(SW0_GPIO_LABEL);
	if (button == NULL) {
		printk("Error: didn't find %s device\n", SW0_GPIO_LABEL);
		return;
	}

	ret = gpio_pin_configure(button, SW0_GPIO_PIN, SW0_GPIO_FLAGS);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n", ret, SW0_GPIO_LABEL,
				SW0_GPIO_PIN);
		return;
	}

	// a hardware interruption appear
	ret = gpio_pin_interrupt_configure(button, SW0_GPIO_PIN,
	GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n", ret,
				SW0_GPIO_LABEL, SW0_GPIO_PIN);
		return;
	}

	// interrupt configuration
	gpio_init_callback(&button_cb_data, buttonPressed, BIT(SW0_GPIO_PIN));
	gpio_add_callback(button, &button_cb_data);
	printk("Set up button at %s pin %d\n", SW0_GPIO_LABEL, SW0_GPIO_PIN);

}

void main(void) {

}


// creation of operative system tasks
K_THREAD_DEFINE(threadSwi0, STACKSIZE, taskSwi0, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(threadLed0, STACKSIZE, taskLed0, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(threadLed1, STACKSIZE, taskLed1, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(threadLed2, STACKSIZE, taskLed2, NULL, NULL, NULL, PRIORITY, 0, 0);




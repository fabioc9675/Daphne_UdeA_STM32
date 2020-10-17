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
#include <drivers/adc.h>

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

/* definition of LEDs and Button */
#define LED0_NODE DT_ALIAS(led0fabian)
#define LED1_NODE DT_ALIAS(led1fabian)
#define LED2_NODE DT_ALIAS(led2fabian)

#define SW0_NODE  DT_ALIAS(sw0fabian)

/* definition of ADC module */
#define ADC1_NODE DT_ALIAS(adc1fabian)

/*
 * Devicetree helper macro which gets the 'flags' cell from a 'gpios'
 * property, or returns 0 if the property has no 'flags' cell.
 */

#define FLAGS_OR_ZERO(node)                          \
	COND_CODE_1(DT_PHA_HAS_CELL(node, gpios, flags), \
			(DT_GPIO_FLAGS(node, gpios)),            \
			(0))

// for ADC 1
#if DT_NODE_HAS_STATUS(ADC1_NODE, okay)
#define ADC1_LABEL  DT_LABEL(ADC1_NODE)
// #define ADC1_PIN	DT_INPUT(ADC1_NODE)
// #define ADC1_FLAGS	(GPIO_INPUT | FLAGS_OR_ZERO(ADC1_NODE))
#else
#error "ADC Not defined"
#define ADC1_LABEL "NO"
#define ADC1_PIN    0
#endif

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

// structure definition for hardware interrupt
static struct gpio_callback button_cb_data;

/*
 * @param my_name         thread identification string
 * @param my_sem          thread's own semaphore
 * @param other_sem       other thread's semaphore
 */

// binary semaphore creation
K_SEM_DEFINE(sem_Led, 0, 1);

// buffer size for ADC
#define ADC_BUFFER_SIZE    100

static uint32_t seq_buffer[ADC_BUFFER_SIZE];

void adcCallback(struct device *dev, struct adc_sequence *sq, short unsigned int num) {
	int i = 0;
	//printk("Print sequence\n");

	for (i = 0; i < 10; i++) {
		printk("%d,", seq_buffer[i]);
	}
	printk("\n");
}

void buttonPressed(struct device *dev, struct gpio_callback *cb, uint32_t pins) {
	printk("button pressed at %" PRIu32 " cycles\n", k_cycle_get_32());
	k_sem_give(&sem_Led);   // deliver semaphore
	printk("Semaphore already delivered\n");
}

/*
 * Task definition
 */
void taskLed0(void) {
	// structure definitions
	struct device *led;
	int ret;
	int cnt = 0;

	// Led device configuration
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
		// printk("Execution of task LED 0\n");
		gpio_pin_set(led, LED0_GPIO_PIN, cnt % 2);
		cnt++;

		// task delay
		k_msleep(500);
	}
}

/*
 * Task definition
 */
void taskLed1(void) {
	// structure definitions
	struct device *led;
	int ret;
	int cnt = 0;

	// Led device configuration
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

	// main loop task 1
	while (1) {
		// printk("Execution of task LED 1\n");
		gpio_pin_set(led, LED1_GPIO_PIN, cnt % 2);
		cnt++;

		// task delay
		k_msleep(250);
	}
}

/*
 * Task definition
 */
void taskLed2(void) {
	// structure definitions
	struct device *led;
	int ret;

	// Led device configuration
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

	// main loop task 2
	while (1) {

		// if (k_sem_take(&sem_Led, K_MSEC(50)) != 0) {
		if (k_sem_take(&sem_Led, K_FOREVER) != 0) {
			printk("Semaphore was not received\n");
		} else {
			/* Execute instructions because semaphore */
			printk("Semaphore already received, execution of task LED 2\n");

			// printk("Execution of task LED 2\n");
			gpio_pin_set(led, LED2_GPIO_PIN, 1);
		}

		// task delay
		k_msleep(1000);
		gpio_pin_set(led, LED2_GPIO_PIN, 0);
	}
}

/*
 * Task used to create an interrupt by hardware
 */
void taskSwi0(void) {
	// structure definitions
	struct device *button;
	int ret;

	// button device configuration
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

	// hardware interruption
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

// task create to use ADC peripheral
void taskAdc1(void) {
	// structure definitions
	struct device *adcc;
	struct adc_channel_cfg adccfg;
	struct adc_sequence_options adc0_sq_opt;
	struct adc_sequence adc0_sq;
	int ret;

	//printk("label %s, channel\n", ADC1_LABEL);

	// adc configuration
	adcc = device_get_binding(ADC1_LABEL);
	if (adcc == NULL) {
		printk("Error: didn't find %s device\n", ADC1_LABEL);
		return;
	} else {
		printk("ADC device defined, %s\n", ADC1_LABEL);
	}

	adccfg.gain = ADC_GAIN_1;
	adccfg.reference=ADC_REF_INTERNAL;
	adccfg.acquisition_time=ADC_ACQ_TIME_DEFAULT;//ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS,40);
	adccfg.channel_id=0;
	adccfg.differential=0;

	adc0_sq_opt.interval_us=0;
	adc0_sq_opt.callback=adcCallback;
	adc0_sq_opt.extra_samplings=0;

	adc0_sq.options=&adc0_sq_opt;
	adc0_sq.channels=0xFF;  // problema, siempre funciona en PA0
	adc0_sq.buffer=seq_buffer;
	adc0_sq.buffer_size=100;
	adc0_sq.resolution=12;
	adc0_sq.oversampling=0;

	ret = adc_channel_setup(adcc, &adccfg);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin\n",
				ret, ADC1_LABEL);
		return;
	} else {
		printk("Open ADC successfully\n");
	}

	while(1){
		// Sample initialized
		ret = adc_read(adcc, &adc0_sq);
		if (ret != 0){
			//printk("Not convertion = %d \n", ret);
		} else {
			//printk("convertion sucessfull = %d, %d \n", ret, adc0_sq.channels);
		}
		k_msleep(500);
	}
}

// task creation, operative system schedulling
K_THREAD_DEFINE(threadAdc1, STACKSIZE, taskAdc1, NULL, NULL, NULL, PRIORITY, 0,
        0);
K_THREAD_DEFINE(threadSwi0, STACKSIZE, taskSwi0, NULL, NULL, NULL, PRIORITY, 0,
		0);
K_THREAD_DEFINE(threadLed0, STACKSIZE, taskLed0, NULL, NULL, NULL, PRIORITY, 0,
		0);
K_THREAD_DEFINE(threadLed1, STACKSIZE, taskLed1, NULL, NULL, NULL, PRIORITY, 0,
		0);
K_THREAD_DEFINE(threadLed2, STACKSIZE, taskLed2, NULL, NULL, NULL, PRIORITY, 0,
		0);


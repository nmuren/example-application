/*
 * Copyright (c) 2025 IoT Course - Tracing Exercise
 * SPDX-License-Identifier: Apache-2.0
 *
 * Multi-Threaded Work Queue System
 * Demonstrates: Threading, Work Queues, ISRs, Timer ISRs, Priority Scheduling
 * Tools: SEGGER SystemView for tracing analysis
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <SEGGER_SYSVIEW.h>

LOG_MODULE_REGISTER(trace_demo, LOG_LEVEL_INF);

/* Button configuration */
#define SW0_NODE DT_ALIAS(sw0)
#if !DT_NODE_HAS_STATUS(SW0_NODE, okay)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});
static struct gpio_callback button_cb_data;



/* Timer configuration */
static struct k_timer computation_timer;

/* Work queue definitions - timer work queue is already implemented */
#define TIMER_WORKQ_STACK_SIZE 2048
#define TIMER_WORKQ_PRIORITY 5

K_THREAD_STACK_DEFINE(timer_workq_stack, TIMER_WORKQ_STACK_SIZE);

static struct k_work_q timer_work_queue;

/* Work items */
static struct k_work timer_work;

/* Statistics */
static uint32_t timer_work_count = 0;
static uint32_t gpio_work_count = 0;

static uint32_t fibonacci(uint32_t n)
{
	if (n <= 1) {
		return n;
	}
	return fibonacci(n - 1) + fibonacci(n - 2);
}

static void timer_work_handler(struct k_work *work)
{
	uint32_t start = k_cycle_get_32();

	LOG_INF("Timer work handler started (work #%u)", timer_work_count + 1);

	uint32_t result = fibonacci(300);

	uint32_t end = k_cycle_get_32();
	uint32_t cycles = end - start;

	timer_work_count++;

	LOG_INF("Timer work completed: fib(30)=%u, cycles=%u, count=%u",
	        result, cycles, timer_work_count);
}

static void computation_timer_handler(struct k_timer *timer)
{
	SEGGER_SYSVIEW_RecordEnterISR();
	SEGGER_SYSVIEW_NameResource((uintptr_t)computation_timer_handler, "Timer ISR (5s)");

	/* Submit work to timer work queue */
	k_work_submit_to_queue(&timer_work_queue, &timer_work);

	SEGGER_SYSVIEW_RecordExitISR();
}

static void button_pressed(const struct device *dev,
                          struct gpio_callback *cb,
                          uint32_t pins)
{
	uint32_t start = k_cycle_get_32();

	LOG_INF("Button pressed - processing in ISR");

	uint32_t result = fibonacci(30);

	uint32_t end = k_cycle_get_32();
	uint32_t cycles = end - start;

	gpio_work_count++;

	LOG_INF("Button ISR completed: fib(30)=%u, cycles=%u, count=%u",
	        result, cycles, gpio_work_count);
}

static int setup_button(void)
{
	int ret;

	if (!gpio_is_ready_dt(&button)) {
		LOG_ERR("Button device not ready");
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret != 0) {
		LOG_ERR("Failed to configure button pin");
		return ret;
	}

	ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		LOG_ERR("Failed to configure button interrupt");
		return ret;
	}

	gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb_data);

	LOG_INF("Button configured on pin %d", button.pin);
	return 0;
}



static void setup_timer(void)
{
	/* Initialize timer to trigger every 5 seconds */
	k_timer_init(&computation_timer, computation_timer_handler, NULL);
	k_timer_start(&computation_timer, K_SECONDS(5), K_SECONDS(5));

	LOG_INF("Timer configured (5 second interval)");
}

static void setup_work_queues(void)
{
	/* Initialize timer work queue */
	k_work_queue_init(&timer_work_queue);
	k_work_queue_start(&timer_work_queue,
	                   timer_workq_stack,
	                   K_THREAD_STACK_SIZEOF(timer_workq_stack),
	                   TIMER_WORKQ_PRIORITY,
	                   NULL);
	k_thread_name_set(&timer_work_queue.thread, "timer_workq");

	/* Initialize timer work item */
	k_work_init(&timer_work, timer_work_handler);

	LOG_INF("Timer work queue initialized (priority %d)", TIMER_WORKQ_PRIORITY);
}


int main(void)
{
	int ret;

	LOG_INF("==========================================");
	LOG_INF("  Multi-Threaded Tracing Demo v2.0");
	LOG_INF("==========================================");

	/* Setup hardware */
	ret = setup_button();
	if (ret < 0) {
		LOG_ERR("Failed to setup button: %d", ret);
		return ret;
	}

	/* Setup work queues */
	setup_work_queues();

	/* Setup timer */
	setup_timer();

	LOG_INF("System ready.");

	/* Main thread goes idle */
	while (1) {
		k_sleep(K_FOREVER);
	}

	return 0;
}

/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT blink_matrix_led

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <string.h>

#include <app/drivers/blink.h>

LOG_MODULE_REGISTER(blink_matrix_led, CONFIG_BLINK_LOG_LEVEL);

/* Macro to create pixel patterns for 5x5 LED matrix */
#define PIXEL_BIT(idx, val)  (val ? BIT(idx) : 0)
#define PIXEL_MASK(...)      (FOR_EACH_IDX(PIXEL_BIT, (|), __VA_ARGS__))

struct blink_matrix_led_data {
	struct k_timer timer;
	bool state;
};

struct blink_matrix_led_config {
	const struct device *display;
	unsigned int period_ms;
	/* TODO: Add brightness field here */
};

/* TODO: Define your pattern here (5x5 LEDs)
 * Use PIXEL_MASK macro: PIXEL_MASK(led0, led1, led2, led3, led4)
 * Example patterns: heart, smiley, letter, number, or your own design
 */
static const uint8_t your_pattern[] = {
	/* TODO: Add your 5 rows here */
};

/* Blank pattern (all LEDs off) */
static const uint8_t blank_pattern[] = {
	PIXEL_MASK(0, 0, 0, 0, 0),
	PIXEL_MASK(0, 0, 0, 0, 0),
	PIXEL_MASK(0, 0, 0, 0, 0),
	PIXEL_MASK(0, 0, 0, 0, 0),
	PIXEL_MASK(0, 0, 0, 0, 0),
};

static void display_pattern(const struct device *display, const uint8_t *pattern)
{
	/* TODO: Implement display pattern write
	 * - Create display_buffer_descriptor
	 * - Call display_write() with pattern data
	 * - Check return value and log errors
	 */
}

static void blink_matrix_led_on_timer_expire(struct k_timer *timer)
{
	const struct device *dev = k_timer_user_data_get(timer);
	const struct blink_matrix_led_config *config = dev->config;
	struct blink_matrix_led_data *data = dev->data;

	/* TODO: Implement timer callback
	 * - Toggle data->state
	 * - Display your_pattern when state is true
	 * - Display blank_pattern when state is false
	 */
}

static int blink_matrix_led_set_period_ms(const struct device *dev,
					   unsigned int period_ms)
{
	const struct blink_matrix_led_config *config = dev->config;
	struct blink_matrix_led_data *data = dev->data;

	if (period_ms == 0) {
		k_timer_stop(&data->timer);
		data->state = false;
		display_pattern(config->display, blank_pattern);
		return 0;
	}

	k_timer_start(&data->timer, K_MSEC(period_ms), K_MSEC(period_ms));

	return 0;
}

static DEVICE_API(blink, blink_matrix_led_api) = {
	.set_period_ms = &blink_matrix_led_set_period_ms,
};

static int blink_matrix_led_init(const struct device *dev)
{
	const struct blink_matrix_led_config *config = dev->config;
	struct blink_matrix_led_data *data = dev->data;
	struct display_capabilities caps;
	int ret;

	if (!device_is_ready(config->display)) {
		LOG_ERR("Display device not ready");
		return -ENODEV;
	}

	/* TODO: Verify display capabilities
	 * - Call display_get_capabilities()
	 * - Check for PIXEL_FORMAT_MONO01 support
	 * - Return -ENOTSUP if not supported
	 */

	/* TODO: Configure display
	 * - Set pixel format to PIXEL_FORMAT_MONO01
	 * - Set brightness from config->brightness
	 * - Disable blanking with display_blanking_off()
	 * - Check return values and handle errors
	 */

	/* Initialize timer */
	k_timer_init(&data->timer, blink_matrix_led_on_timer_expire, NULL);
	k_timer_user_data_set(&data->timer, (void *)dev);

	data->state = false;
	display_pattern(config->display, blank_pattern);

	/* Start blinking if a period was specified */
	if (config->period_ms > 0) {
		k_timer_start(&data->timer, K_MSEC(config->period_ms),
			      K_MSEC(config->period_ms));
	}

	LOG_INF("Matrix LED blink driver initialized");

	return 0;
}

#define BLINK_MATRIX_LED_DEFINE(inst)                                          \
	static struct blink_matrix_led_data data##inst;                        \
                                                                               \
	static const struct blink_matrix_led_config config##inst = {           \
	    .display = DEVICE_DT_GET(DT_INST_PHANDLE(inst, display)),         \
	    .period_ms = DT_INST_PROP_OR(inst, blink_period_ms, 0U),          \
	    /* TODO: Read brightness-level from devicetree */                 \
	};                                                                     \
                                                                               \
	DEVICE_DT_INST_DEFINE(inst, blink_matrix_led_init, NULL, &data##inst, \
			      &config##inst, POST_KERNEL,                      \
			      CONFIG_BLINK_INIT_PRIORITY,                      \
			      &blink_matrix_led_api);

DT_INST_FOREACH_STATUS_OKAY(BLINK_MATRIX_LED_DEFINE)

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

#include "matrix_font.h"

LOG_MODULE_REGISTER(blink_matrix_led, CONFIG_BLINK_LOG_LEVEL);

struct blink_matrix_led_data {
	struct k_timer timer;
	unsigned int index;
};

struct blink_matrix_led_config {
	const struct device *display;
	unsigned int period_ms;
	uint8_t brightness;
};

/* "I <heart> IOT", followed by a blank. */
static const uint8_t *const letter_patterns[] = {
	matrix_letter_patterns['I' - 'A'],
	heart_pattern,
	matrix_letter_patterns['I' - 'A'],
	matrix_letter_patterns['O' - 'A'],
	matrix_letter_patterns['T' - 'A'],
	blank_pattern,
};

static void display_pattern(const struct device *display, const uint8_t *pattern)
{
	/*
	 * Five rows are stored as one byte each. Pitch is 8 so the driver
	 * skips the three unused bits before the next row.
	 */
	static const struct display_buffer_descriptor desc = {
		.buf_size = 5,
		.width = 5,
		.height = 5,
		.pitch = 8,
	};
	int ret;

	ret = display_write(display, 0, 0, &desc, pattern);
	if (ret < 0) {
		LOG_ERR("Could not write pattern (%d)", ret);
	}
}

static void blink_matrix_led_on_timer_expire(struct k_timer *timer)
{
	const struct device *dev = k_timer_user_data_get(timer);
	const struct blink_matrix_led_config *config = dev->config;
	struct blink_matrix_led_data *data = dev->data;

	display_pattern(config->display, letter_patterns[data->index]);
	data->index = (data->index + 1) % ARRAY_SIZE(letter_patterns);
}

static int blink_matrix_led_set_period_ms(const struct device *dev,
					   unsigned int period_ms)
{
	const struct blink_matrix_led_config *config = dev->config;
	struct blink_matrix_led_data *data = dev->data;

	if (period_ms == 0) {
		k_timer_stop(&data->timer);
		data->index = 0;
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

	display_get_capabilities(config->display, &caps);
	if (!(caps.supported_pixel_formats & PIXEL_FORMAT_MONO01)) {
		LOG_ERR("Display does not support MONO01");
		return -ENOTSUP;
	}

	ret = display_set_pixel_format(config->display, PIXEL_FORMAT_MONO01);
	if (ret < 0) {
		LOG_ERR("Could not set pixel format (%d)", ret);
		return ret;
	}

	ret = display_set_brightness(config->display, config->brightness);
	if (ret < 0) {
		LOG_ERR("Could not set brightness (%d)", ret);
		return ret;
	}

	ret = display_blanking_off(config->display);
	if (ret < 0) {
		LOG_ERR("Could not turn blanking off (%d)", ret);
		return ret;
	}

	/* Initialize timer */
	k_timer_init(&data->timer, blink_matrix_led_on_timer_expire, NULL);
	k_timer_user_data_set(&data->timer, (void *)dev);

	data->index = 0;
	display_pattern(config->display, blank_pattern);

	/* Start blinking if a period was specified */
	if (config->period_ms > 0) {
		k_timer_start(&data->timer, K_MSEC(config->period_ms),
			      K_MSEC(config->period_ms));
	}

	LOG_INF("Matrix LED blink driver initialized");

	return 0;
}

#define BLINK_MATRIX_LED_DEFINE(inst)                                     	\
	BUILD_ASSERT(DT_INST_PROP(inst, brightness_level) >= 1 &&             	\
			     DT_INST_PROP(inst, brightness_level) <= 255,      			\
		     "brightness-level must be between 1 and 255");            		\
                                                                        	\
	static struct blink_matrix_led_data data##inst;                        	\
                                                                        	\
	static const struct blink_matrix_led_config config##inst = {           	\
	    .display = DEVICE_DT_GET(DT_INST_PHANDLE(inst, display)),          	\
	    .period_ms = DT_INST_PROP_OR(inst, blink_period_ms, 0U),           	\
	    .brightness = DT_INST_PROP(inst, brightness_level),                	\
	};                                                                     	\
                                                                            \
	DEVICE_DT_INST_DEFINE(inst, blink_matrix_led_init, NULL, &data##inst, 	\
			      &config##inst, POST_KERNEL,                      			\
			      CONFIG_BLINK_INIT_PRIORITY,                      			\
			      &blink_matrix_led_api);

DT_INST_FOREACH_STATUS_OKAY(BLINK_MATRIX_LED_DEFINE)

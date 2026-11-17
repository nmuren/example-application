/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#include <app/drivers/blink.h>

#include <app_version.h>

LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

#define BLINK_PERIOD_MS_SLOW 1000U
#define BLINK_PERIOD_MS_FAST 200U
#define SHAKE_THRESHOLD_MM_S2 10000
#define SHAKE_COOLDOWN_MS 1000

int main(void)
{
	int ret;
	unsigned int period_ms = BLINK_PERIOD_MS_SLOW;
	int previous_x = 0;
	bool have_previous = false;
	int64_t next_shake_ms = 0;
	const struct device *sensor, *blink;
	struct sensor_value val;

	printk("Zephyr Example Application %s\n", APP_VERSION_STRING);

	sensor = DEVICE_DT_GET(DT_ALIAS(accel0));
	if (!device_is_ready(sensor)) {
		LOG_ERR("Sensor not ready");
		return 0;
	}

	blink = DEVICE_DT_GET(DT_NODELABEL(blink_led));
	if (!device_is_ready(blink)) {
		LOG_ERR("Blink LED not ready");
		return 0;
	}

	ret = blink_set_period_ms(blink, period_ms);
	if (ret < 0) {
		LOG_ERR("Could not set blink period (%d)", ret);
		return 0;
	}

	printk("Shake along the X axis to toggle between 200 and 1000 ms\n");

	while (1) {
		ret = sensor_sample_fetch(sensor);
		if (ret < 0) {
			LOG_ERR("Could not fetch sample (%d)", ret);
			return 0;
		}

		ret = sensor_channel_get(sensor, SENSOR_CHAN_ACCEL_X, &val);
		if (ret < 0) {
			LOG_ERR("Could not get sample (%d)", ret);
			return 0;
		}

		/* Convert m/s^2 to mm/s^2 so logging needs no floating point. */
		int accel_x = (int)(sensor_value_to_micro(&val) / 1000);
		int change = accel_x - previous_x;
		int64_t now = k_uptime_get();

		/* Ignore the first reading and allow at most one toggle per second. */
		if (have_previous && now >= next_shake_ms &&
		    (change > SHAKE_THRESHOLD_MM_S2 || change < -SHAKE_THRESHOLD_MM_S2)) {
			unsigned int next_period_ms = period_ms == BLINK_PERIOD_MS_SLOW
				? BLINK_PERIOD_MS_FAST : BLINK_PERIOD_MS_SLOW;

			ret = blink_set_period_ms(blink, next_period_ms);
			if (ret < 0) {
				LOG_ERR("Could not set blink period (%d)", ret);
				return 0;
			}
			period_ms = next_period_ms;
			next_shake_ms = now + SHAKE_COOLDOWN_MS;
			LOG_INF("Shake detected");
		}
		previous_x = accel_x;
		have_previous = true;

		LOG_INF("Acceleration X: %d mm/s^2, blink interval: %u ms",
			accel_x, period_ms);

		k_sleep(K_MSEC(100));
	}

	return 0;
}


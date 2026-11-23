/*
 * Shared presentation helpers for Sheet 4 BLE sensor values.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef APP_BLE_SENSOR_FORMAT_H_
#define APP_BLE_SENSOR_FORMAT_H_

#include <stddef.h>
#include <stdint.h>

#include <zephyr/sys/printk.h>

#define BLE_SENSOR_ENVIRONMENTAL_STRING_SIZE 64

static inline void ble_sensor_format_environmental(char *buffer, size_t size,
					    int32_t temperature_milli,
					    int32_t humidity_milli)
{
	int32_t temperature_fraction = temperature_milli % 1000;
	int32_t humidity_fraction = humidity_milli % 1000;

	if (temperature_fraction < 0) {
		temperature_fraction = -temperature_fraction;
	}
	if (humidity_fraction < 0) {
		humidity_fraction = -humidity_fraction;
	}

	snprintk(buffer, size, "temperature=%d.%03d C, humidity=%d.%03d %%",
		 temperature_milli / 1000, temperature_fraction,
		 humidity_milli / 1000, humidity_fraction);
}

#endif /* APP_BLE_SENSOR_FORMAT_H_ */

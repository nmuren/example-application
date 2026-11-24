/*
 * Shared BLE manufacturer-data format for Sheet 4.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef APP_BLE_SENSOR_PACKET_H_
#define APP_BLE_SENSOR_PACKET_H_

#include <stdint.h>

#include <zephyr/toolchain.h>

#define BLE_SENSOR_COMPANY_ID 0x3535

enum ble_sensor_kind {
	BLE_SENSOR_KIND_ACCELEROMETER = 1,
	BLE_SENSOR_KIND_TEMP_HUMIDITY = 2,
};

/* All multi-byte fields are stored little-endian on the air. */
struct ble_sensor_packet {
	uint16_t company_id;
	uint8_t sensor_kind;
	int32_t value_1;
	int32_t value_2;
	int32_t value_3;
} __packed;

#endif /* APP_BLE_SENSOR_PACKET_H_ */

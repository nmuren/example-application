/*
 * BLE sensor broadcaster for Sheet 4, Task 2.1.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/util.h>

#include <app/ble_sensor_format.h>
#include <app/ble_sensor_packet.h>

LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

#define UPDATE_PERIOD   K_SECONDS(1)

/*
 * Manufacturer-specific data starts with the 16-bit company identifier.
 * All multi-byte values are encoded little-endian, as required by Bluetooth.
 *
 * For SENSOR_KIND_ACCELEROMETER, values are x/y/z in mm/s^2.
 * For SENSOR_KIND_TEMP_HUMIDITY, values are milli-degrees Celsius,
 * milli-percent relative humidity, and zero.
 */
static struct ble_sensor_packet packet;

static const struct bt_data advertising_data[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
	BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME,
		sizeof(CONFIG_BT_DEVICE_NAME) - 1),
	BT_DATA(BT_DATA_MANUFACTURER_DATA, &packet, sizeof(packet)),
};

#if DT_NODE_EXISTS(DT_ALIAS(temphum0))

static const struct device *const sensor = DEVICE_DT_GET(DT_ALIAS(temphum0));

static int read_sensor(void)
{
	char formatted[BLE_SENSOR_ENVIRONMENTAL_STRING_SIZE];
	struct sensor_value temperature;
	struct sensor_value humidity;
	int err;

	err = sensor_sample_fetch(sensor);
	if (err < 0) {
		return err;
	}

	err = sensor_channel_get(sensor, SENSOR_CHAN_AMBIENT_TEMP, &temperature);
	if (err < 0) {
		return err;
	}

	err = sensor_channel_get(sensor, SENSOR_CHAN_HUMIDITY, &humidity);
	if (err < 0) {
		return err;
	}

	int32_t temperature_milli = (int32_t)(sensor_value_to_micro(&temperature) / 1000);
	int32_t humidity_milli = (int32_t)(sensor_value_to_micro(&humidity) / 1000);

	packet.sensor_kind = BLE_SENSOR_KIND_TEMP_HUMIDITY;
	packet.value_1 = (int32_t)sys_cpu_to_le32((uint32_t)temperature_milli);
	packet.value_2 = (int32_t)sys_cpu_to_le32((uint32_t)humidity_milli);
	packet.value_3 = 0;

	ble_sensor_format_environmental(formatted, sizeof(formatted),
					temperature_milli, humidity_milli);
	LOG_INF("Advertising %s", formatted);

	return 0;
}

#else

static const struct device *const sensor = DEVICE_DT_GET(DT_ALIAS(accel0));

static int read_sensor(void)
{
	struct sensor_value acceleration[3];
	int32_t acceleration_milli[3];
	int err;

	err = sensor_sample_fetch(sensor);
	if (err < 0) {
		return err;
	}

	err = sensor_channel_get(sensor, SENSOR_CHAN_ACCEL_XYZ, acceleration);
	if (err < 0) {
		return err;
	}

	for (size_t i = 0; i < ARRAY_SIZE(acceleration); i++) {
		acceleration_milli[i] =
			(int32_t)(sensor_value_to_micro(&acceleration[i]) / 1000);
	}

	packet.sensor_kind = BLE_SENSOR_KIND_ACCELEROMETER;
	packet.value_1 = (int32_t)sys_cpu_to_le32((uint32_t)acceleration_milli[0]);
	packet.value_2 = (int32_t)sys_cpu_to_le32((uint32_t)acceleration_milli[1]);
	packet.value_3 = (int32_t)sys_cpu_to_le32((uint32_t)acceleration_milli[2]);

	LOG_INF("Advertising acceleration x=%d, y=%d, z=%d mm/s^2",
		acceleration_milli[0], acceleration_milli[1], acceleration_milli[2]);

	return 0;
}

#endif

int main(void)
{
	int err;

	BUILD_ASSERT(sizeof(packet) == 15, "Unexpected sensor packet padding");
	BUILD_ASSERT(3 + 2 + (sizeof(CONFIG_BT_DEVICE_NAME) - 1) +
		     2 + sizeof(packet) <= 31,
		     "Advertising data exceeds the legacy 31-byte limit");
	packet.company_id = sys_cpu_to_le16(BLE_SENSOR_COMPANY_ID);

	if (!device_is_ready(sensor)) {
		LOG_ERR("Sensor %s is not ready", sensor->name);
		return 0;
	}

	err = bt_enable(NULL);
	if (err < 0) {
		LOG_ERR("Bluetooth initialization failed (%d)", err);
		return 0;
	}

	err = read_sensor();
	if (err < 0) {
		LOG_ERR("Initial sensor read failed (%d)", err);
		return 0;
	}

	err = bt_le_adv_start(BT_LE_ADV_NCONN, advertising_data,
			      ARRAY_SIZE(advertising_data), NULL, 0);
	if (err < 0) {
		LOG_ERR("Advertising failed to start (%d)", err);
		return 0;
	}

	LOG_INF("Non-connectable advertising started as %s", CONFIG_BT_DEVICE_NAME);

	while (1) {
		k_sleep(UPDATE_PERIOD);

		err = read_sensor();
		if (err < 0) {
			LOG_ERR("Sensor read failed (%d)", err);
			continue;
		}

		err = bt_le_adv_update_data(advertising_data,
					    ARRAY_SIZE(advertising_data), NULL, 0);
		if (err < 0) {
			LOG_ERR("Advertising update failed (%d)", err);
		}
	}

	return 0;
}

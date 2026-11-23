/*
 * BLE sensor observer for Sheet 4, Task 2.3.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/util.h>

#include <app/ble_sensor_format.h>
#include <app/ble_sensor_packet.h>

LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

#define DISPLAY_MIN_BRIGHTNESS 16U
#define DISPLAY_MAX_BRIGHTNESS 255U

struct received_sample {
	uint8_t sensor_kind;
	int32_t value_1;
	int32_t value_2;
	int32_t value_3;
	int8_t rssi;
};

struct parse_context {
	int8_t rssi;
	bool found;
};

static const struct device *const display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
static struct received_sample latest_sample;
static bool have_sample;
static int64_t next_report_ms;
static bool display_available;
static struct k_spinlock sample_lock;
static struct k_work actuator_work;

static int64_t absolute_value(int64_t value)
{
	return value < 0 ? -value : value;
}

static uint8_t scale_brightness(int64_t value, int64_t maximum)
{
	value = CLAMP(value, 0, maximum);

	return DISPLAY_MIN_BRIGHTNESS +
		(uint8_t)(value * (DISPLAY_MAX_BRIGHTNESS - DISPLAY_MIN_BRIGHTNESS) /
			  maximum);
}

static uint8_t sample_brightness(const struct received_sample *sample)
{
	if (sample->sensor_kind == BLE_SENSOR_KIND_ACCELEROMETER) {
		int64_t x = sample->value_1;
		int64_t y = sample->value_2;
		int64_t z = sample->value_3;
		int64_t largest = MAX(MAX(absolute_value(x), absolute_value(y)),
				      absolute_value(z));

		/* Full brightness at roughly 2 g. */
		return scale_brightness(largest, 20000);
	}

	/* For the simulated sensor, map 0-100% relative humidity to brightness. */
	return scale_brightness(sample->value_2, 100000);
}

static void actuator_update(struct k_work *work)
{
	char formatted[BLE_SENSOR_ENVIRONMENTAL_STRING_SIZE];
	struct received_sample sample;
	uint8_t brightness;
	k_spinlock_key_t key;
	int err;

	ARG_UNUSED(work);

	key = k_spin_lock(&sample_lock);
	sample = latest_sample;
	k_spin_unlock(&sample_lock, key);

	brightness = sample_brightness(&sample);

	if (sample.sensor_kind == BLE_SENSOR_KIND_ACCELEROMETER) {
		LOG_INF("Received accel x=%d, y=%d, z=%d mm/s^2, RSSI=%d dBm, LED=%u",
			sample.value_1, sample.value_2, sample.value_3, sample.rssi,
			brightness);
	} else {
		ble_sensor_format_environmental(formatted, sizeof(formatted),
						sample.value_1, sample.value_2);
		LOG_INF("Received %s, RSSI=%d dBm, LED=%u", formatted,
			sample.rssi, brightness);
	}

	if (!display_available) {
		return;
	}

	err = display_set_brightness(display, brightness);
	if (err < 0) {
		LOG_ERR("Could not set LED matrix brightness (%d)", err);
	}
}

static bool manufacturer_data_found(struct bt_data *data, void *user_data)
{
	struct parse_context *context = user_data;
	struct ble_sensor_packet wire_packet;
	struct received_sample sample;
	k_spinlock_key_t key;
	bool report;
	int64_t now;

	if (data->type != BT_DATA_MANUFACTURER_DATA ||
	    data->data_len < sizeof(wire_packet)) {
		return true;
	}

	memcpy(&wire_packet, data->data, sizeof(wire_packet));
	if (sys_le16_to_cpu(wire_packet.company_id) != BLE_SENSOR_COMPANY_ID) {
		return true;
	}

	if (wire_packet.sensor_kind != BLE_SENSOR_KIND_ACCELEROMETER &&
	    wire_packet.sensor_kind != BLE_SENSOR_KIND_TEMP_HUMIDITY) {
		return true;
	}

	sample.sensor_kind = wire_packet.sensor_kind;
	sample.value_1 = (int32_t)sys_le32_to_cpu((uint32_t)wire_packet.value_1);
	sample.value_2 = (int32_t)sys_le32_to_cpu((uint32_t)wire_packet.value_2);
	sample.value_3 = (int32_t)sys_le32_to_cpu((uint32_t)wire_packet.value_3);
	sample.rssi = context->rssi;
	now = k_uptime_get();

	key = k_spin_lock(&sample_lock);
	report = !have_sample || now >= next_report_ms ||
		 sample.sensor_kind != latest_sample.sensor_kind ||
		 sample.value_1 != latest_sample.value_1 ||
		 sample.value_2 != latest_sample.value_2 ||
		 sample.value_3 != latest_sample.value_3;
	latest_sample = sample;
	have_sample = true;
	if (report) {
		next_report_ms = now + 1000;
	}
	k_spin_unlock(&sample_lock, key);

	if (report) {
		k_work_submit(&actuator_work);
	}

	context->found = true;
	return false;
}

static void advertisement_received(const bt_addr_le_t *addr, int8_t rssi,
				   uint8_t adv_type, struct net_buf_simple *ad)
{
	struct parse_context context = {
		.rssi = rssi,
	};

	ARG_UNUSED(addr);
	ARG_UNUSED(adv_type);

	bt_data_parse(ad, manufacturer_data_found, &context);
}

static int display_init(void)
{
	static const uint8_t all_pixels_on[] = {
		0x1f, 0x1f, 0x1f, 0x1f, 0x1f,
	};
	static const struct display_buffer_descriptor descriptor = {
		.buf_size = sizeof(all_pixels_on),
		.width = 5,
		.height = 5,
		.pitch = 8,
	};
	int err;

	if (!device_is_ready(display)) {
		LOG_WRN("LED matrix is not ready; received values will only be logged");
		return -ENODEV;
	}

	err = display_set_pixel_format(display, PIXEL_FORMAT_MONO01);
	if (err < 0) {
		return err;
	}

	err = display_write(display, 0, 0, &descriptor, all_pixels_on);
	if (err < 0) {
		return err;
	}

	err = display_blanking_off(display);
	if (err < 0) {
		return err;
	}

	display_available = true;
	return 0;
}

int main(void)
{
	static const struct bt_le_scan_param scan_parameters = {
		.type = BT_LE_SCAN_TYPE_PASSIVE,
		.options = 0,
		.interval = BT_GAP_SCAN_FAST_INTERVAL_MIN,
		.window = BT_GAP_SCAN_FAST_WINDOW,
	};
	int err;

	BUILD_ASSERT(sizeof(struct ble_sensor_packet) == 15,
		     "Unexpected sensor packet padding");

	k_work_init(&actuator_work, actuator_update);

	err = display_init();
	if (err < 0) {
		LOG_WRN("LED matrix initialization failed (%d)", err);
	}

	err = bt_enable(NULL);
	if (err < 0) {
		LOG_ERR("Bluetooth initialization failed (%d)", err);
		return 0;
	}

	err = bt_le_scan_start(&scan_parameters, advertisement_received);
	if (err < 0) {
		LOG_ERR("Scanning failed to start (%d)", err);
		return 0;
	}

	LOG_INF("Passive scanning started (duplicate filtering disabled)");
	return 0;
}

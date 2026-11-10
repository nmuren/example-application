/*
 * Copyright (c) 2025 IoT Course - Debugging Exercise
 * SPDX-License-Identifier: Apache-2.0
 *
 * Sensor monitoring system with data collection and processing
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>

LOG_MODULE_REGISTER(debug_array, LOG_LEVEL_INF);

/* Button configuration - using Button 1 */
#define SW0_NODE	DT_ALIAS(sw0)
#if !DT_NODE_HAS_STATUS(SW0_NODE, okay)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios, {0});
static struct gpio_callback button_cb_data;

/* Sensor data configuration */
#define MAX_SENSORS 5
#define SENSOR_NAME_SIZE 16

/**
 * Structure representing a single sensor reading
 */
struct sensor_data {
	char name[SENSOR_NAME_SIZE];
	int32_t value;
	uint32_t timestamp;
	bool valid;
};

/**
 * Sensor processing function pointer type
 */
typedef void (*sensor_processor_t)(struct sensor_data *sensor, uint32_t index);

/**
 * Global sensor array and metadata
 */
static struct sensor_data sensors[MAX_SENSORS];
static uint32_t sensor_count = MAX_SENSORS;

/**
 * Processor functions for different sensor types
 */
static void process_temperature(struct sensor_data *sensor, uint32_t index)
{
	LOG_INF("Processing temperature sensor[%u]: %d", index, sensor->value);
}

static void process_humidity(struct sensor_data *sensor, uint32_t index)
{
	LOG_INF("Processing humidity sensor[%u]: %d", index, sensor->value);
}

static void process_pressure(struct sensor_data *sensor, uint32_t index)
{
	LOG_INF("Processing pressure sensor[%u]: %d", index, sensor->value);
}

static void process_light(struct sensor_data *sensor, uint32_t index)
{
	LOG_INF("Processing light sensor[%u]: %d", index, sensor->value);
}

static void process_accelerometer(struct sensor_data *sensor, uint32_t index)
{
	LOG_INF("Processing accelerometer sensor[%u]: %d", index, sensor->value);
}

/**
 * Array of sensor processor functions
 * Initialized at runtime
 */
static sensor_processor_t sensor_processors[MAX_SENSORS];

/**
 * Initialize sensor processor function table
 */
static void init_sensor_processors(void)
{
	const sensor_processor_t processors[] = {
		process_temperature,
		process_humidity,
		process_pressure,
		process_light,
		process_accelerometer
	};

	/* Assign processor functions to the global array */
	// for (uint32_t i = 0; i < MAX_SENSORS - 1; i++) {
	for (uint32_t i = 0; i < MAX_SENSORS; i++) {
		sensor_processors[i] = processors[i];
	}

	LOG_INF("Sensor processors initialized");
}

/**
 * Initialize sensor array with default values
 */
static void init_sensors(void)
{
	const char *sensor_names[] = {
		"Temperature",
		"Humidity",
		"Pressure",
		"Light",
		"Accelerometer"
	};

	LOG_INF("Initializing %u sensors...", sensor_count);

	for (uint32_t i = 0; i < MAX_SENSORS; i++) {
		snprintf(sensors[i].name, SENSOR_NAME_SIZE, "%s", sensor_names[i]);
		sensors[i].value = 0;
		sensors[i].timestamp = 0;
		sensors[i].valid = false;
	}

	LOG_INF("Sensor initialization complete");
}

/**
 * Sensor history buffer for trend analysis
 */
static uint32_t sensor_history_data[512];  // 2048 bytes

/**
 * Calculate checksum over sensor history chunks
 */
static uint32_t calculate_history_checksum(uint32_t *data, uint32_t offset, uint32_t depth)
{
	volatile uint32_t local_chunk[32];  // 128 bytes for intermediate calculations

	// Process this chunk - copy and transform data
	for (uint32_t i = 0; i < 32; i++) {
		local_chunk[i] = data[offset + i] * 31 + i;
	}

	// Accumulate results from this chunk
	uint32_t chunk_sum = 0;
	for (uint32_t i = 0; i < 32; i++) {
		chunk_sum += local_chunk[i];
	}

	if (depth > 0) {
		// Process next chunk recursively
		uint32_t next_sum = calculate_history_checksum(data, offset + 32, depth - 1);
		return chunk_sum + next_sum;
	}

	return chunk_sum;
}

/**
 * Simulate reading sensor values
 */
static void read_sensor_values(void)
{
	uint32_t current_time = k_uptime_get_32();

	LOG_INF("Reading sensor values at time: %u ms", current_time);

	for (uint32_t i = 0; i < sensor_count; i++) {
		sensors[i].value = (int32_t)(current_time % 100) + (i * 10);
		sensors[i].timestamp = current_time;
		sensors[i].valid = true;

		LOG_DBG("Sensor[%u]: value = %d", i, sensors[i].value);
	}
}

/**
 * Process and validate sensor data
 */
static void process_sensors(void)
{
	uint32_t valid_count = 0;

	LOG_INF("Processing sensor data...");

	for (uint32_t i = 0; i < sensor_count; i++) {
		if (sensors[i].valid) {
			sensor_processors[i](&sensors[i], i);
			valid_count++;
		}
	}

	LOG_INF("Processed %u sensors successfully", valid_count);
}

/**
 * Main sensor processing routine
 * Called when button is pressed
 */
static void run_sensor_diagnostics(void)
{
	LOG_INF("===========================================");
	LOG_INF("  Running Sensor Diagnostics");
	LOG_INF("===========================================");

	read_sensor_values();
	k_msleep(50);

	uint32_t checksum = calculate_history_checksum(sensor_history_data, 0, 8);
	LOG_INF("History checksum: 0x%08x", checksum);

	process_sensors();

	LOG_INF("Diagnostics complete");
}

static struct k_work diagnostics_work;

/**
 * Diagnostics work handler - runs in thread context
 */
static void diagnostics_work_handler(struct k_work *work)
{
	run_sensor_diagnostics();
}

/**
 * Button interrupt handler
 */
static void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	LOG_INF("Button pressed - submitting diagnostics work");
	k_work_submit(&diagnostics_work);
}

/**
 * Setup button interrupt
 */
static int setup_button(void)
{
	int ret;

	if (!gpio_is_ready_dt(&button)) {
		LOG_ERR("Button device not ready");
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret != 0) {
		LOG_ERR("Failed to configure button pin: %d", ret);
		return ret;
	}

	ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		LOG_ERR("Failed to configure button interrupt: %d", ret);
		return ret;
	}

	gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb_data);

	LOG_INF("Button configured successfully");

	return 0;
}

int main(void)
{
	int ret;

	LOG_INF("===========================================");
	LOG_INF("  Sensor Monitoring System v1.0");
	LOG_INF("===========================================");
	LOG_INF("");
	LOG_INF("This application demonstrates sensor data");
	LOG_INF("collection and processing.");
	LOG_INF("");

	init_sensor_processors();
	init_sensors();

	k_work_init(&diagnostics_work, diagnostics_work_handler);

	ret = setup_button();
	if (ret < 0) {
		LOG_ERR("Failed to setup button: %d", ret);
		return ret;
	}

	LOG_INF("Press Button 1 to run sensor diagnostics");
	LOG_INF("System ready...");

	/* Main loop - just keep alive */
	while (1) {
		k_sleep(K_SECONDS(1));
	}

	return 0;
}

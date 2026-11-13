# Copyright (c) 2024 Nordic Semiconductor ASA
# SPDX-License-Identifier: Apache-2.0

zephyr_library()
zephyr_library_sources_ifdef(CONFIG_BLINK_GPIO_LED gpio_led.c)
zephyr_library_sources_ifdef(CONFIG_BLINK_MATRIX_LED matrix_led.c)

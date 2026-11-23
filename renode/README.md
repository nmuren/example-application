# Sheet 4: BBC micro:bit V2 BLE simulation

Use the Renode 1.17 and Zephyr setup from Sheet 1. Both applications target
`bbc_microbit_v2`. The platform adds radio, PPI and timers to the Sheet 1 model.

## Extract the student bundle

From `~/zephyrproject/example-application`, extract `renode.zip`:

```sh
unzip /path/to/renode.zip
```

The archive creates `renode/`. Keep your applications in sibling directories
`ble_broadcaster/` and `ble_observer/`. Application sources and firmware are not
included in the archive; implement the tasks in the sheet.

## Build for simulation

Run these commands from `~/zephyrproject/example-application`:

```sh
west build -b bbc_microbit_v2 ble_broadcaster -d ble_broadcaster/build-renode --pristine -- \
  -DEXTRA_DTC_OVERLAY_FILE="$PWD/renode/microbit_ble.overlay" \
  -DEXTRA_CONF_FILE="$PWD/renode/ble_simulation.conf"

west build -b bbc_microbit_v2 ble_observer -d ble_observer/build-renode --pristine -- \
  -DEXTRA_CONF_FILE="$PWD/renode/ble_simulation.conf"
```

The additional overlay merges with an application's existing board overlay.
It supplies the SI7021 node and its `temphum0` alias. Access the simulated sensor
with `DEVICE_DT_GET(DT_ALIAS(temphum0))`, using `SENSOR_CHAN_AMBIENT_TEMP` and
`SENSOR_CHAN_HUMIDITY`. The extra configuration enables the sensor driver, disables
unmodeled motion sensor drivers, and disables blocking RTT logging.

Keep physical-board builds separate from `build-renode`. Do not apply this
simulation overlay/configuration when flashing a physical micro:bit. The physical
board does not contain an SI7021; use your Week 3 motion sensor code there.

## Run BLE

```sh
cd renode
renode ble_broadcaster.resc    # broadcaster alone
renode ble_observer.resc       # observer alone
renode multi_node_ble.resc     # both devices on one BLE medium
```

The combined script creates both boards in the same emulator. Running two
single-node scripts in separate Renode processes does not connect their radios.
UART windows show application output. The combined script also exposes the
broadcaster UART on TCP port 60001 and observer UART on port 60002.

If your project names differ, edit the ELF paths at the top of the scripts.
To inspect packets in Wireshark, uncomment `emulation LogBLETraffic` in the
script before launching it. Wireshark capture is optional.

## Sensor values and scope

The model starts at 25 degrees Celsius and 50 percent humidity. To test different
inputs, close Renode, edit `Temperature` and `Humidity` in `microbit_ble.repl`,
and relaunch. Continuous sampling/advertisement updates still run in the firmware;
no Python sensor helper is required or included.

The tested scope is non-connectable BLE advertising and passive scanning between
two micro:bit-targeted ELFs. The model uses Renode's NRF52840 radio peripherals to
approximate the nRF52833. It does not model the physical micro:bit's motion sensors
or LED matrix. Use UART to inspect received values during simulation; verify the
matrix/actuator behavior on physical boards. A phone scanner needs real BLE
hardware; it cannot receive advertisements from Renode's internal medium.

## Other repository tools

The full repository also contains the separate Week 1 UART script and Week 3
`microbit_visualizer/` hardware/sensor tools. They are not part of this Sheet 4
bundle, apart from the base `microbit_v2.repl` platform reused by the BLE model.

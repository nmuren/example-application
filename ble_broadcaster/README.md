# BLE broadcaster (Sheet 4, Task 2.1)

The application advertises a packed 15-byte manufacturer-specific payload once
per second using non-connectable BLE advertising. On a physical BBC micro:bit V2
it reads the accelerometer. With the supplied Renode overlay it automatically
uses the simulated SI7021 temperature/humidity sensor instead.

Runtime messages use Zephyr's logging subsystem and the application's default
`INF` log level.

Payload layout (all multi-byte values are little-endian):

| Offset | Size | Meaning |
| ---: | ---: | --- |
| 0 | 2 | Test company ID (`0xffff`) |
| 2 | 1 | Sensor kind (`1` accelerometer, `2` temperature/humidity) |
| 3 | 4 | X in mm/s², or temperature in milli-°C |
| 7 | 4 | Y in mm/s², or relative humidity in milli-% |
| 11 | 4 | Z in mm/s², or zero |

Physical-board build:

```sh
west build -b bbc_microbit_v2 ble_broadcaster -d ble_broadcaster/build --pristine
west flash -d ble_broadcaster/build
```

Always pass the build directory to `west flash`. Running `west flash` without
`-d ble_broadcaster/build` uses the repository's default `build/` directory,
which may contain firmware from an earlier exercise.

Renode build (run from the `example-application` directory):

```sh
west build -b bbc_microbit_v2 ble_broadcaster \
  -d ble_broadcaster/build-renode --pristine -- \
  -DEXTRA_DTC_OVERLAY_FILE="$PWD/renode/microbit_ble.overlay" \
  -DEXTRA_CONF_FILE="$PWD/renode/ble_simulation.conf"
```

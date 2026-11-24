# BLE observer (Sheet 4, Task 2.3)

The observer passively scans without duplicate filtering, extracts manufacturer
data with company ID `0xffff`, decodes the shared sensor packet, and reports the
latest valid value at most once per second. On physical hardware it displays all
25 matrix pixels and maps the
received acceleration magnitude to brightness. For the Renode temperature and
humidity payload, relative humidity controls brightness.

Physical-board build:

```sh
west build -b bbc_microbit_v2 ble_observer -d ble_observer/build --pristine
west flash -d ble_observer/build
```

Always pass the build directory to `west flash`. Running `west flash` without
`-d ble_observer/build` uses the repository's default `build/` directory,
which may contain firmware from an earlier exercise.

The physical observer requires a second BLE-capable board running the
broadcaster firmware; one micro:bit cannot run both applications at once.

Renode build:

```sh
west build -b bbc_microbit_v2 ble_observer \
  -d ble_observer/build-renode --pristine -- \
  -DEXTRA_CONF_FILE="$PWD/renode/ble_simulation.conf"
```

Run the broadcaster and observer together from `example-application/renode`:

```sh
renode multi_node_ble.resc
```

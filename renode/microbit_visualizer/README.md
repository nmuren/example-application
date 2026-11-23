# Week 3 micro:bit visualizer

Requires **Renode 1.17** and Python 3; no extra Python packages are needed.
[Install Renode](https://renode.readthedocs.io/en/latest/introduction/installing.html)
and make sure `renode --version` reports 1.17.

Build your normal `bbc_microbit_v2` application first. From this directory,
start these commands in two separate terminals:

```sh
python3 run.py --elf /absolute/path/to/build/zephyr/zephyr.elf
```

```sh
python3 bridge.py --frame-port 60033
```

Open <http://127.0.0.1:8000/>. Use B to start blinking, as on the board;
drag the board to change the simulated sensor orientation.

This loads the same ELF as the physical board. LEDs follow the modeled PWM
output; UART is diagnostic text only. Run the bridge locally on a trusted
machine. The simulator does not replace the final physical-board demonstration.

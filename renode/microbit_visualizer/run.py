#!/usr/bin/env python3
"""Run an unchanged bbc_microbit_v2 ELF with stock Renode 1.17.

Renode 1.17's NRF_CLOCK model is required; no Renode fork or firmware rebuild.
Run bridge.py in another terminal for the browser UI.
"""
import argparse
from pathlib import Path
import re
import shlex
import shutil
import signal
import subprocess
import sys

ROOT = Path(__file__).resolve().parent
SCRIPT = ROOT / "renode_model" / "microbit_v2_hardware.resc"


def renode_command(option):
    if option:
        return shlex.split(option)
    executable = shutil.which("renode")
    if executable:
        return [executable]
    raise ValueError("Renode not found: install Renode 1.17 or pass --renode '/path/to/renode'")


def detect_version(command):
    result = subprocess.run(command + ["--version"], capture_output=True,
                            text=True, timeout=20, check=True)
    match = re.search(r"Renode v(\d+)\.(\d+)(?:\.\d+)?", result.stdout + result.stderr)
    if not match:
        raise ValueError("could not parse Renode --version output")
    return int(match.group(1)), int(match.group(2))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--elf", type=Path, required=True,
                        help="existing Zephyr physical-board zephyr.elf (absolute or relative)")
    parser.add_argument("--renode", help="Renode 1.17 executable or launch command")
    parser.add_argument("--monitor-port", type=int, default=1234)
    parser.add_argument("--uart-port", type=int, default=60031)
    parser.add_argument("--frame-port", type=int, default=60033)
    args = parser.parse_args()
    elf = args.elf.resolve()
    if not elf.is_file():
        parser.error("ELF file not found: %s" % elf)
    try:
        command = renode_command(args.renode)
        version = detect_version(command)
    except (ValueError, subprocess.CalledProcessError, subprocess.TimeoutExpired) as error:
        parser.error(str(error))
    if version != (1, 17):
        parser.error("Renode 1.17 is required (found %d.%d)" % version)
    script = SCRIPT
    monitor = "$bin=@%s; $uart_port=%d; $frame_port=%d; include @%s" % (
        elf, args.uart_port, args.frame_port, script)
    print("Renode %d.%d: %s" % (*version, script.name), flush=True)
    print("Next terminal: python3 bridge.py --frame-port %d --uart-port %d --monitor-port %d" %
          (args.frame_port, args.uart_port, args.monitor_port), flush=True)
    # Keep the emulator tied to this launcher even when the terminal closes.
    def stop(_signal, _frame):
        raise KeyboardInterrupt

    previous = signal.signal(signal.SIGTERM, stop)
    process = None
    try:
        process = subprocess.Popen(command + ["--disable-xwt", "-P",
                                              str(args.monitor_port), "-e", monitor])
        return process.wait()
    except KeyboardInterrupt:
        return 130
    finally:
        signal.signal(signal.SIGTERM, previous)
        if process is not None and process.poll() is None:
            process.terminate()
            try:
                process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""Local micro:bit visualizer: PWM-model UDP and UART logs -> browser SSE.

Uses only the Python standard library. Run Renode with -P 1234 and the included
scenario, then open the HTTP address printed by this script. No student exercise
logic or device-side display code is implemented here.
"""

import argparse
from collections import deque
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
from pathlib import Path
import queue
import re
import socket
import threading

FRAME = re.compile(r"^@MB2:([0-9a-fA-F]{8}):(\d{1,3}):([01])$")
ANSI = re.compile(r"\x1b\[[0-9;]*[A-Za-z]")
PIXEL_MASK = (1 << 25) - 1
UI = Path(__file__).resolve().parent / "ui" / "index.html"
MONITOR_LOCK = threading.Lock()


def parse_frame(line):
    """Validate a Renode PWM-model UDP frame; never call this on firmware UART."""
    match = FRAME.fullmatch(line.strip())
    if not match:
        return None
    bits, brightness, blanked = match.groups()
    value = int(bits, 16)
    level = int(brightness)
    if value & ~PIXEL_MASK or level > 255:
        return None
    return {"pixels": value, "brightness": level, "blanked": blanked == "1"}


class State:
    def __init__(self, sensor_supported=False):
        self.lock = threading.Lock()
        self.clients = set()
        self.data = {"pixels": 0, "brightness": 255, "blanked": True,
                     "connected": False, "uart": [],
                     "sensorSupported": sensor_supported,
                     "orientation": {"yaw": 0, "pitch": 0, "roll": 0}}
        self.lines = deque(maxlen=70)

    def snapshot(self):
        with self.lock:
            return dict(self.data)

    def update(self, **changes):
        with self.lock:
            self.data.update(changes)
            self.data["uart"] = list(self.lines)
            message = json.dumps(self.data, separators=(",", ":"))
            for client in tuple(self.clients):
                try:
                    client.put_nowait(message)
                except queue.Full:
                    # Slow/disconnected browser: replace stale state with latest.
                    try:
                        client.get_nowait()
                        client.put_nowait(message)
                    except queue.Empty:
                        pass

    def accept_line(self, line):
        """Keep firmware UART diagnostic-only, including legacy @MB2 messages.

        Only follow_frames() may decode display telemetry. Treating UART text
        as LED state would let broken firmware impersonate working PWM output.
        """
        if line.strip():
            with self.lock:
                self.lines.append(ANSI.sub("", line.rstrip("\r\n"))[-240:])
            self.update()

    def subscribe(self):
        client = queue.Queue(maxsize=2)
        with self.lock:
            self.clients.add(client)
            client.put_nowait(json.dumps(self.data, separators=(",", ":")))
        return client

    def unsubscribe(self, client):
        with self.lock:
            self.clients.discard(client)


def follow_uart(state, host, port, stop):
    while not stop.is_set():
        try:
            with socket.create_connection((host, port), timeout=3) as conn:
                conn.settimeout(2)
                state.update(connected=True)
                pending = b""
                while not stop.is_set():
                    try:
                        chunk = conn.recv(2048)
                    except socket.timeout:
                        continue
                    if not chunk:
                        break
                    pending += chunk
                    # Protect against an unbounded line from buggy firmware.
                    if len(pending) > 16384:
                        pending = pending[-4096:]
                    while b"\n" in pending:
                        raw, pending = pending.split(b"\n", 1)
                        state.accept_line(raw.decode("utf-8", errors="replace"))
        except OSError:
            pass
        state.update(connected=False)
        stop.wait(1)


def follow_frames(state, host, port, stop):
    """Receive complete matrix states from the Renode-side PWM model.

    This path requires no simulator-specific firmware or UART protocol; the
    unchanged physical-board ELF writes real PWM0 registers in Renode.
    """
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as receiver:
        receiver.bind((host, port))
        receiver.settimeout(1)
        while not stop.is_set():
            try:
                payload, _ = receiver.recvfrom(128)
            except socket.timeout:
                continue
            frame = parse_frame(payload.decode("ascii", errors="replace"))
            if frame is not None:
                state.update(**frame)


def send_monitor_command(host, port, command):
    """Submit a validated command to Renode's telnet Monitor and await its prompt.

    The first input line can lose bytes during Renode's telnet greeting;
    prime that line with a harmless machine selection before the real command.
    """
    if not re.fullmatch(rb"[A-Za-z0-9_. -]{1,120}", command):
        raise ValueError("unsafe monitor command")
    with MONITOR_LOCK:
        with socket.create_connection((host, port), timeout=3) as conn:
            conn.settimeout(5)
            conn.recv(128)
            conn.sendall(b'mach set "microbit"\n')
            greeting = b""
            while b'(microbit)' not in greeting or b'microbit"' not in greeting:
                chunk = conn.recv(2048)
                if not chunk or len(greeting) > 8192:
                    raise OSError("Renode Monitor did not select microbit")
                greeting += chunk
            conn.sendall(command + b"\n")
            reply = b""
            while command not in reply or b"(microbit)" not in reply[reply.find(command):]:
                chunk = conn.recv(2048)
                if not chunk or len(reply) > 8192:
                    raise OSError("Renode Monitor did not acknowledge command")
                reply += chunk
            result = reply[reply.find(command) + len(command):]
            if b"No such command" in result or b"There was an error" in result:
                raise OSError("Renode rejected command")


def send_button_to_monitor(host, port, name, pressed):
    command = ("gpio0.button%s %s" % (name,
                "Press" if pressed else "Release")).encode("ascii")
    send_monitor_command(host, port, command)


def send_orientation_to_monitor(host, port, yaw, pitch, roll):
    # UI: yaw around Y, pitch around X, roll around Z; the physical sensor
    # model accepts aerospace yaw(Z), pitch(Y), roll(X) in that order.
    command = ("twim0 SetOrientation %d %d %d" % (roll, yaw, pitch)).encode("ascii")
    send_monitor_command(host, port, command)


def make_handler(state, monitor_host, monitor_port):
    class Handler(BaseHTTPRequestHandler):
        def do_GET(self):
            if self.path in ("/", "/index.html"):
                content = UI.read_bytes()
                self.send_response(200)
                self.send_header("Content-Type", "text/html; charset=utf-8")
                self.send_header("Content-Length", str(len(content)))
                self.end_headers()
                self.wfile.write(content)
            elif self.path == "/events":
                self.send_response(200)
                self.send_header("Content-Type", "text/event-stream; charset=utf-8")
                self.send_header("Cache-Control", "no-cache")
                self.send_header("X-Accel-Buffering", "no")
                self.end_headers()
                client = state.subscribe()
                try:
                    while True:
                        try:
                            message = client.get(timeout=15)
                            self.wfile.write(b"data: " + message.encode() + b"\n\n")
                        except queue.Empty:
                            self.wfile.write(b": keepalive\n\n")
                        self.wfile.flush()
                except (BrokenPipeError, ConnectionResetError, OSError):
                    pass
                finally:
                    state.unsubscribe(client)
            else:
                self.send_error(404)

        def do_POST(self):
            if self.path not in ("/button", "/orientation"):
                self.send_error(404)
                return
            try:
                size = int(self.headers.get("Content-Length", "0"))
                if size < 1 or size > 128:
                    raise ValueError("invalid body size")
                data = json.loads(self.rfile.read(size))
                if not isinstance(data, dict):
                    raise ValueError("expected JSON object")
                if self.path == "/button":
                    if data.get("name") not in ("A", "B") or type(data.get("pressed")) is not bool:
                        raise ValueError("expected A/B and boolean pressed")
                elif any(type(data.get(key)) is not int or abs(data[key]) > 180
                         for key in ("yaw", "pitch", "roll")):
                    raise ValueError("expected yaw/pitch/roll integer degrees in -180..180")
            except (ValueError, json.JSONDecodeError):
                self.send_error(400, "Invalid input")
                return
            try:
                if self.path == "/button":
                    send_button_to_monitor(monitor_host, monitor_port,
                                           data["name"], data["pressed"])
                else:
                    send_orientation_to_monitor(monitor_host, monitor_port,
                                                data["yaw"], data["pitch"], data["roll"])
                    state.update(orientation={key: data[key] for key in
                                              ("yaw", "pitch", "roll")})
            except OSError:
                self.send_error(503, "Renode monitor or sensor model unavailable")
                return
            self.send_response(204)
            self.end_headers()

        def log_message(self, fmt, *args):
            # Keep UI polling/connection noise out of the lecture terminal.
            if self.path != "/events":
                super().log_message(fmt, *args)

    return Handler


def argument_parser():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--host", default="127.0.0.1", help="HTTP bind address")
    parser.add_argument("--port", type=int, default=8000, help="browser port")
    parser.add_argument("--renode-host", default="127.0.0.1")
    parser.add_argument("--uart-port", type=int, default=60031)
    parser.add_argument("--frame-port", type=int, default=60033,
                        help="UDP port for Renode's hardware-PWM model (default: 60033)")
    parser.add_argument("--monitor-port", type=int, default=1234)
    return parser


def main():
    args = argument_parser().parse_args()

    state = State(sensor_supported=True)
    stop = threading.Event()
    thread = threading.Thread(target=follow_uart,
                              args=(state, args.renode_host, args.uart_port, stop),
                              daemon=True)
    thread.start()
    threading.Thread(target=follow_frames,
                     args=(state, args.host, args.frame_port, stop),
                     daemon=True).start()
    server = ThreadingHTTPServer((args.host, args.port),
                                 make_handler(state, args.renode_host, args.monitor_port))
    server.daemon_threads = True
    print("Micro:bit board: http://%s:%d/" % (args.host, args.port), flush=True)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        stop.set()
        server.server_close()


if __name__ == "__main__":
    main()

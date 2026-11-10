#!/bin/bash
#
# Start OpenOCD RTT server for BBC micro:bit v2
# Exposes SystemView RTT Channel 1 on TCP port 5000
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OPENOCD_CFG="${SCRIPT_DIR}/openocd-rtt.cfg"

echo "========================================"
echo "  OpenOCD RTT Server for micro:bit v2"
echo "========================================"
echo ""
echo "Configuration:"
echo "  - Target: nRF52833 (BBC micro:bit v2)"
echo "  - Debug Probe: CMSIS-DAP"
echo "  - Firmware logs: USB serial UART at 115200 baud"
echo "  - RTT Channel 1 (SystemView trace): tcp://localhost:5000"
echo ""
echo "Usage:"
echo "  Terminal 1: Run this script"
echo "  Terminal 2: screen /dev/ttyACM0 115200  (replace with your serial port)"
echo "  SystemView: Connect to localhost:5000"
echo ""
echo "Starting OpenOCD..."
echo ""

# Find OpenOCD binary
OPENOCD=""

# First, try to get it from west config (if user has configured it)
if command -v west &> /dev/null; then
    WEST_OPENOCD=$(west config runners.openocd 2>/dev/null || true)
    if [ -n "$WEST_OPENOCD" ] && [ -x "$WEST_OPENOCD" ]; then
        OPENOCD="$WEST_OPENOCD"
        echo "Using OpenOCD from west config: $OPENOCD"
    fi
fi

# Fall back to PATH
if [ -z "$OPENOCD" ]; then
    if command -v openocd &> /dev/null; then
        OPENOCD="openocd"
        echo "Using OpenOCD from PATH: $(which openocd)"
    fi
fi

# If still not found, error out with helpful message
if [ -z "$OPENOCD" ]; then
    echo "ERROR: openocd not found"
    echo ""
    echo "OpenOCD is required to forward RTT data to SystemView."
    echo ""
    echo "Installation options:"
    echo ""
    echo "1. If you installed Zephyr SDK with OpenOCD:"
    echo "   Find your openocd binary (usually in SDK installation) and configure west:"
    echo "   west config runners.openocd /path/to/your/openocd"
    echo ""
    echo "2. Install OpenOCD separately:"
    echo "   macOS:   brew install openocd"
    echo "   Ubuntu:  sudo apt-get install openocd"
    echo "   Fedora:  sudo dnf install openocd"
    echo ""
    echo "3. Or set OPENOCD environment variable:"
    echo "   export OPENOCD=/path/to/openocd"
    echo "   Then run this script again"
    echo ""
    exit 1
fi

# Check if config file exists
if [ ! -f "$OPENOCD_CFG" ]; then
    echo "ERROR: Config file not found: $OPENOCD_CFG"
    exit 1
fi

# Start OpenOCD with RTT server
exec "$OPENOCD" -f "$OPENOCD_CFG"

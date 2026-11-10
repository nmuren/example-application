# Makefile for tracing_exercise
# Allows quick rebuild from GDB or terminal

BOARD ?= bbc_microbit_v2

.PHONY: all build clean flash debug pristine systemview

all: build

build:
	west build -b $(BOARD)

pristine:
	west build -b $(BOARD) -p

clean:
	rm -rf build

flash: build
	west flash --runner openocd

debug: build
	west debug --runner openocd

systemview:
	@echo "To use SystemView on BBC micro:bit V2:"
	@echo "1. Build/flash firmware and run ./start-rtt-server.sh for trace channel 1"
	@echo "2. Open board USB serial at 115200 baud for firmware text logs"
	@echo "3. In SEGGER SystemView select IP, host localhost, port 5000"
	@echo "4. Start recording and reset the board once"

rebuild: clean build

help:
	@echo "Available targets:"
	@echo "  build      - Build the application (default)"
	@echo "  pristine   - Clean build from scratch"
	@echo "  clean      - Remove build directory"
	@echo "  flash      - Build and flash to board"
	@echo "  debug      - Build and start GDB session"
	@echo "  systemview - Show SystemView usage instructions"
	@echo "  rebuild    - Clean and build"
	@echo ""
	@echo "Variables:"
	@echo "  BOARD      - Target board (default: bbc_microbit_v2)"
	@echo ""
	@echo "Examples:"
	@echo "  make"
	@echo "  make BOARD=bbc_microbit_v2"
	@echo "  make pristine"
	@echo "  make systemview"

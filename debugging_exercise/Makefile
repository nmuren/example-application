# Makefile for debugging_exercise_array
# Allows quick rebuild from GDB using 'make' command

BOARD ?= bbc_microbit_v2

.PHONY: all build clean flash debug pristine

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

rebuild: clean build

help:
	@echo "Available targets:"
	@echo "  build     - Build the application (default)"
	@echo "  pristine  - Clean build from scratch"
	@echo "  clean     - Remove build directory"
	@echo "  flash     - Build and flash to board"
	@echo "  debug     - Build and start GDB session"
	@echo "  rebuild   - Clean and build"
	@echo ""
	@echo "Variables:"
	@echo "  BOARD     - Target board (default: nrf52840dk_nrf52840)"
	@echo ""
	@echo "Examples:"
	@echo "  make"
	@echo "  make BOARD=bbc_microbit_v2"
	@echo "  make pristine"

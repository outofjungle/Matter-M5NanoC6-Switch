# Matter M5NanoC6 Switch - Makefile
# ESP32-C6 Matter device using esp-matter SDK

.PHONY: all build clean fullclean erase flash monitor flash-monitor \
        menuconfig size size-components get-mac help check-env \
        generate-pairing

# Default target
all: build

#------------------------------------------------------------------------------
# Configuration
#------------------------------------------------------------------------------

# ESP-IDF target
TARGET := esp32c6

# Project name (must match CMakeLists.txt)
PROJECT_NAME := M5NanoC6-Switch

# Serial port (auto-detect or override with PORT=)
PORT ?= $(shell ls /dev/cu.usbmodem* 2>/dev/null | head -1)

# Baud rate for flashing and monitoring
BAUD_RATE := 115200

# Log file for monitor output
LOG_DIR := $(CURDIR)/logs
LOG_FILE := $(LOG_DIR)/monitor.log

#------------------------------------------------------------------------------
# Environment Check
#------------------------------------------------------------------------------

check-env:
ifndef ESP_MATTER_PATH
	$(error ESP_MATTER_PATH is not set. Please set it to the esp-matter repository path)
endif
ifndef IDF_PATH
	$(error IDF_PATH is not set. Please set it to the ESP-IDF path)
endif
	@echo "ESP_MATTER_PATH: $(ESP_MATTER_PATH)"
	@echo "IDF_PATH: $(IDF_PATH)"
	@echo "TARGET: $(TARGET)"
	@echo "PORT: $(PORT)"

#------------------------------------------------------------------------------
# Build Targets
#------------------------------------------------------------------------------

build: check-env ## Build the project
	idf.py -D IDF_TARGET=$(TARGET) build

menuconfig: check-env ## Open menuconfig
	idf.py menuconfig

clean: ## Clean build artifacts (keeps sdkconfig)
	idf.py clean

fullclean: ## Full clean including sdkconfig
	idf.py fullclean
	rm -f sdkconfig sdkconfig.old
	rm -rf build

size: build ## Show firmware size
	idf.py size

size-components: build ## Show size by component
	idf.py size-components

#------------------------------------------------------------------------------
# Flash Targets
#------------------------------------------------------------------------------

erase: ## Erase entire flash
	@if [ -z "$(PORT)" ]; then \
		echo "Error: No serial port found. Connect device or set PORT="; \
		exit 1; \
	fi
	esptool.py --port $(PORT) erase_flash

flash: build ## Flash firmware to device
	@if [ -z "$(PORT)" ]; then \
		echo "Error: No serial port found. Connect device or set PORT="; \
		exit 1; \
	fi
	idf.py -p $(PORT) flash

monitor: ## Open serial monitor (exit with Ctrl+])
	@if [ -z "$(PORT)" ]; then \
		echo "Error: No serial port found. Connect device or set PORT="; \
		exit 1; \
	fi
	@mkdir -p $(LOG_DIR)
	idf.py -p $(PORT) monitor -b $(BAUD_RATE) 2>&1 | tee $(LOG_FILE)

flash-monitor: build ## Flash and open serial monitor
	@if [ -z "$(PORT)" ]; then \
		echo "Error: No serial port found. Connect device or set PORT="; \
		exit 1; \
	fi
	@mkdir -p $(LOG_DIR)
	idf.py -p $(PORT) flash monitor -b $(BAUD_RATE) 2>&1 | tee $(LOG_FILE)

#------------------------------------------------------------------------------
# Device Info
#------------------------------------------------------------------------------

get-mac: ## Display connected device MAC address
	@if [ -z "$(PORT)" ]; then \
		echo "Error: No serial port found. Connect device or set PORT="; \
		exit 1; \
	fi
	@esptool.py --port $(PORT) chip_id 2>/dev/null | grep -E 'MAC:|Chip is'

#------------------------------------------------------------------------------
# Pairing Configuration
#------------------------------------------------------------------------------

PAIRING_CONFIG := main/include/CHIPPairingConfig.h
PAIRING_QR_IMAGE := pairing_qr.png

generate-pairing: ## Generate random pairing code, update config, and create QR image
	@python3 -c "import random; \
invalid={0,11111111,22222222,33333333,44444444,55555555,66666666,77777777,88888888,99999999,12345678,87654321}; \
d=random.randint(0,4095); \
p=random.randint(1,99999999); \
exec('while p in invalid: p=random.randint(1,99999999)'); \
print(f'{d} {p}')" | xargs -n2 sh -c 'python3 scripts/generate_pairing_config.py -d $$0 -p $$1 -o $(PAIRING_CONFIG) --qr-image $(PAIRING_QR_IMAGE)'

#------------------------------------------------------------------------------
# Help
#------------------------------------------------------------------------------

help: ## Show this help message
	@echo "M5NanoC6 Matter Switch - Build System"
	@echo ""
	@echo "Usage: make [target] [VARIABLE=value]"
	@echo ""
	@echo "Targets:"
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | \
		awk 'BEGIN {FS = ":.*?## "}; {printf "  %-18s %s\n", $$1, $$2}'
	@echo ""
	@echo "Variables:"
	@echo "  PORT              Serial port (auto-detected: $(PORT))"
	@echo "  BAUD_RATE         Baud rate (default: $(BAUD_RATE))"
	@echo ""
	@echo "Required Environment Variables:"
	@echo "  ESP_MATTER_PATH   Path to esp-matter repository"
	@echo "  IDF_PATH          Path to ESP-IDF (v5.3.x)"
	@echo ""
	@echo "Quick Start:"
	@echo "  make build        # Build firmware"
	@echo "  make erase        # Erase flash (first time)"
	@echo "  make flash        # Flash firmware"
	@echo "  make monitor      # View serial output"
	@echo "  make flash-monitor # Flash and monitor"
	@echo ""
	@echo "Logs saved to: $(LOG_FILE)"

# Matter M5NanoC6 Switch - Makefile
# Docker-based build environment (default) with host tools for flashing/monitoring
#
# Docker builds (default):
#   make build, make clean, make menuconfig, make rebuild
#
# Local builds (requires ESP-IDF):
#   make local-build, make local-clean, make local-menuconfig
#
# Flash & Monitor (host tools):
#   make flash, make erase, make monitor

.PHONY: all build clean fullclean rebuild flash monitor erase \
        menuconfig generate-pairing shell image-build help \
        local-build local-clean local-rebuild local-menuconfig \
        image-pull image-status

# Default target
all: build

#------------------------------------------------------------------------------
# Configuration
#------------------------------------------------------------------------------

TARGET := esp32c6

# Auto-detect serial port (macOS: cu.usbmodem*, Linux: ttyUSB* or ttyACM*)
PORT ?= $(shell ls /dev/cu.usbmodem* /dev/ttyUSB* /dev/ttyACM* 2>/dev/null | head -1)

# Pairing configuration
PAIRING_CONFIG := main/include/CHIPPairingConfig.h
PAIRING_QR_IMAGE := pairing_qr.png

# Logging configuration
LOGS_DIR := logs
LOG_FILE := $(LOGS_DIR)/monitor_$(shell date +%Y%m%d_%H%M%S).log

# Docker compose command
DOCKER_COMPOSE := docker-compose
DOCKER_RUN := $(DOCKER_COMPOSE) run --rm esp-idf

#------------------------------------------------------------------------------
# Docker Image Management
#------------------------------------------------------------------------------

image-build: ## Build Docker image with ESP-Matter SDK (~2-3GB, takes 10-20 min first time)
	$(DOCKER_COMPOSE) build

image-pull: ## Pull base ESP-IDF image
	docker pull espressif/esp-matter:release-v1.5_idf_v5.4.1

image-status: ## Show Docker image info
	@echo "Checking Docker images..."
	@docker images | grep -E "REPOSITORY|esp|matter" || echo "No ESP images found"

#------------------------------------------------------------------------------
# Docker Build (default)
#------------------------------------------------------------------------------
# Note: -C /project is required even though docker-compose sets working_dir=/project
# because the ESP-IDF activation script may change the working directory.
# The -C flag explicitly tells idf.py where to find our project's CMakeLists.txt.

build: ## Build firmware in Docker container
	$(DOCKER_RUN) idf.py -C /project -D IDF_TARGET=$(TARGET) build

clean: ## Clean build artifacts in Docker
	$(DOCKER_RUN) idf.py fullclean

rebuild: fullclean build ## Full clean and rebuild in Docker

menuconfig: ## Open SDK configuration in Docker (interactive)
	$(DOCKER_RUN) idf.py menuconfig

#------------------------------------------------------------------------------
# Local Build (requires local ESP-IDF installation)
#------------------------------------------------------------------------------

local-build: ## Build firmware locally with installed ESP-IDF
	idf.py -D IDF_TARGET=$(TARGET) build

local-clean: ## Clean build artifacts locally
	idf.py fullclean

local-rebuild: fullclean local-build ## Full clean and rebuild locally

local-menuconfig: ## Open SDK configuration locally (interactive)
	idf.py menuconfig

#------------------------------------------------------------------------------
# Flash & Monitor (uses host esptool)
#------------------------------------------------------------------------------

flash: ## Flash firmware to device using host esptool
	@test -n "$(PORT)" || (echo "Error: No device found. Set PORT=<device>" && exit 1)
	@test -f build/flash_args || (echo "Error: Build first with 'make build'" && exit 1)
	cd build && esptool --port $(PORT) write_flash @flash_args

monitor: ## Serial monitor with logging (esp-idf-monitor: Ctrl+], screen: Ctrl+A K)
	@test -n "$(PORT)" || (echo "Error: No device found. Set PORT=<device>" && exit 1)
	@mkdir -p $(LOGS_DIR)
	@echo "Logging to $(LOG_FILE)"
	@if command -v esp-idf-monitor >/dev/null 2>&1; then \
		echo "Using esp-idf-monitor (Ctrl+] to exit)"; \
		esp-idf-monitor --port $(PORT) build/M5NanoC6-Switch.elf 2>&1 | tee $(LOG_FILE); \
	else \
		echo "Using screen for monitoring (Ctrl+A then K to exit)"; \
		stty -f $(PORT) 115200 raw -echo; \
		cat $(PORT) | while IFS= read -r line; do \
			echo "$$(date '+%H:%M:%S.%3N') $$line" | tee -a $(LOG_FILE); \
		done; \
	fi

erase: ## Erase flash (factory reset) using host esptool
	@test -n "$(PORT)" || (echo "Error: No device found. Set PORT=<device>" && exit 1)
	esptool --port $(PORT) erase_flash

fullclean: ## Full clean (removes build, sdkconfig, managed_components)
	@echo "Cleaning build artifacts..."
	rm -rf build managed_components sdkconfig sdkconfig.old dependencies.lock

#------------------------------------------------------------------------------
# Docker Utilities
#------------------------------------------------------------------------------

shell: ## Open interactive shell in Docker container
	$(DOCKER_RUN) bash

#------------------------------------------------------------------------------
# Pairing
#------------------------------------------------------------------------------

generate-pairing: ## Generate random pairing code and QR image (interactive confirmation)
	@echo "Generating random pairing configuration..."
	@python3 -c "import random; \
invalid={0,11111111,22222222,33333333,44444444,55555555,66666666,77777777,88888888,99999999,12345678,87654321}; \
d=random.randint(0,4095); \
p=random.randint(1,99999999); \
exec('while p in invalid: p=random.randint(1,99999999)'); \
print(f'{d} {p}')" > /tmp/pairing_values.txt
	@D=$$(awk '{print $$1}' /tmp/pairing_values.txt); \
	P=$$(awk '{print $$2}' /tmp/pairing_values.txt); \
	CURRENT_ID=$$(grep -o 'FIRMWARE_CONFIG_ID [0-9]*' $(PAIRING_CONFIG) 2>/dev/null | awk '{print $$2}' || echo "0"); \
	NEW_ID=$$((($${CURRENT_ID} + 1) % 16)); \
	echo "============================================================"; \
	echo "Pairing Configuration Changes"; \
	echo "============================================================"; \
	printf "  Discriminator: 0x%03X (%d)\n" $$D $$D; \
	echo "  Passcode:      $$P"; \
	printf "  Config ID:     %d -> %d (binary: " $$CURRENT_ID $$NEW_ID; \
	python3 -c "print(f'{$$NEW_ID:04b}')"; \
	echo ")"; \
	echo ""; \
	echo "Config ID is displayed on LED during factory reset:"; \
	echo "  White = 1, Red = 0, LSB first, repeated 5 times"; \
	echo ""; \
	read -p "Proceed with these changes? [y/N]: " REPLY; \
	if [ "$$REPLY" = "y" ] || [ "$$REPLY" = "Y" ]; then \
		echo "Generating configuration..."; \
		$(DOCKER_RUN) python3 /project/scripts/generate_pairing_config.py \
			-d $$D -p $$P \
			-o /project/$(PAIRING_CONFIG) \
			--qr-image /project/$(PAIRING_QR_IMAGE) \
			--no-confirm; \
	else \
		echo "Aborted."; \
		rm -f /tmp/pairing_values.txt; \
		exit 1; \
	fi; \
	rm -f /tmp/pairing_values.txt

#------------------------------------------------------------------------------
# Help
#------------------------------------------------------------------------------

help: ## Show this help
	@echo "Matter M5NanoC6 Switch - Docker Build Environment"
	@echo ""
	@echo "DOCKER BUILD (default):"
	@echo "  make build           Build firmware in Docker"
	@echo "  make clean           Clean build artifacts"
	@echo "  make rebuild         Full clean + rebuild"
	@echo "  make menuconfig      Open SDK configuration (interactive)"
	@echo ""
	@echo "LOCAL BUILD (requires ESP-IDF installation):"
	@echo "  make local-build     Build firmware locally"
	@echo "  make local-clean     Clean build artifacts locally"
	@echo "  make local-rebuild   Full clean + rebuild locally"
	@echo "  make local-menuconfig Open SDK configuration locally"
	@echo ""
	@echo "FLASH & MONITOR (host tools):"
	@echo "  make flash           Flash firmware to device"
	@echo "  make monitor         Monitor with logging to logs/"
	@echo "  make erase           Erase flash (factory reset)"
	@echo ""
	@echo "DOCKER MANAGEMENT:"
	@echo "  make image-build     Build Docker image (~10-20 min, one-time)"
	@echo "  make image-pull      Pull base Docker image"
	@echo "  make image-status    Show Docker image info"
	@echo "  make shell           Open bash shell in container"
	@echo ""
	@echo "UTILITIES:"
	@echo "  make fullclean       Full clean (build, sdkconfig, deps)"
	@echo "  make generate-pairing Generate random pairing code and QR"
	@echo ""
	@echo "Current PORT: $(PORT)"
	@echo ""
	@echo "Override port: make flash PORT=/dev/ttyUSB0"

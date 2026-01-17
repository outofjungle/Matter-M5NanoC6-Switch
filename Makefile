# Matter M5NanoC6 Switch - Docker-based Makefile
# All builds run in Docker containers - no local ESP-IDF installation needed

.PHONY: all build clean fullclean flash monitor erase \
        menuconfig generate-pairing shell image-build help

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

# Docker compose command
DOCKER_COMPOSE := docker-compose
DOCKER_RUN := $(DOCKER_COMPOSE) run --rm esp-idf

# Set UID/GID for file ownership (prevents root-owned files)
export UID := $(shell id -u)
export GID := $(shell id -g)

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
# Build
#------------------------------------------------------------------------------

build: ## Build firmware in Docker container
	$(DOCKER_RUN) idf.py -C /project -D IDF_TARGET=$(TARGET) build

clean: ## Clean build artifacts
	$(DOCKER_RUN) idf.py fullclean

fullclean: ## Full clean (removes build, sdkconfig, managed_components)
	$(DOCKER_RUN) sh -c "rm -rf build managed_components sdkconfig sdkconfig.old dependencies.lock"

rebuild: fullclean build ## Full clean and rebuild from scratch

menuconfig: ## Open SDK configuration (interactive)
	$(DOCKER_RUN) idf.py menuconfig

#------------------------------------------------------------------------------
# Flash & Monitor (using host esptool)
#------------------------------------------------------------------------------

flash: ## Flash firmware to device using host esptool
	@test -n "$(PORT)" || (echo "Error: No device found. Set PORT=<device>" && exit 1)
	@test -f build/flash_args || (echo "Error: Build first with 'make build'" && exit 1)
	cd build && esptool --port $(PORT) write_flash @flash_args

monitor: ## Serial monitor (screen: Ctrl+A K, esp-idf-monitor: Ctrl+])
	@test -n "$(PORT)" || (echo "Error: No device found. Set PORT=<device>" && exit 1)
	@if command -v esp-idf-monitor >/dev/null 2>&1; then \
		echo "Using esp-idf-monitor (Ctrl+] to exit)"; \
		esp-idf-monitor --port $(PORT) build/M5NanoC6-Switch.elf; \
	else \
		echo "Using screen for monitoring (Ctrl+A then K to exit)"; \
		screen $(PORT) 115200; \
	fi

flash-monitor: flash ## Flash and immediately open monitor
	@echo "Waiting for device to reset..."
	@sleep 2
	@$(MAKE) monitor

erase: ## Erase flash (factory reset) using host esptool
	@test -n "$(PORT)" || (echo "Error: No device found. Set PORT=<device>" && exit 1)
	esptool --port $(PORT) erase_flash

#------------------------------------------------------------------------------
# Development Tools
#------------------------------------------------------------------------------

shell: ## Open interactive shell in container
	$(DOCKER_RUN) bash

size: ## Show binary size analysis
	$(DOCKER_RUN) idf.py size

size-components: ## Show size breakdown by component
	$(DOCKER_RUN) idf.py size-components

size-files: ## Show size breakdown by source files
	$(DOCKER_RUN) idf.py size-files

#------------------------------------------------------------------------------
# Pairing
#------------------------------------------------------------------------------

generate-pairing: ## Generate random pairing code and QR image
	@$(DOCKER_RUN) sh -c "python3 -c \"import random; \
invalid={0,11111111,22222222,33333333,44444444,55555555,66666666,77777777,88888888,99999999,12345678,87654321}; \
d=random.randint(0,4095); \
p=random.randint(1,99999999); \
exec('while p in invalid: p=random.randint(1,99999999)'); \
print(f'{d} {p}')\" | xargs -n2 sh -c 'python3 /project/scripts/generate_pairing_config.py -d \$$0 -p \$$1 -o /project/$(PAIRING_CONFIG) --qr-image /project/$(PAIRING_QR_IMAGE)'"

#------------------------------------------------------------------------------
# Help
#------------------------------------------------------------------------------

help: ## Show this help
	@echo "Matter M5NanoC6 Switch - Docker Build System"
	@echo ""
	@echo "First time setup:"
	@echo "  make image-build     Build Docker image (~10-20 min, one-time)"
	@echo ""
	@echo "Build:"
	@echo "  make build           Build firmware"
	@echo "  make clean           Clean build artifacts"
	@echo "  make fullclean       Full clean (build, sdkconfig, deps)"
	@echo "  make rebuild         Full clean + rebuild"
	@echo "  make menuconfig      Open SDK configuration (interactive)"
	@echo ""
	@echo "Flash & Monitor:"
	@echo "  make flash           Flash firmware to device"
	@echo "  make monitor         Open serial monitor"
	@echo "  make flash-monitor   Flash and monitor"
	@echo "  make erase           Erase flash (factory reset)"
	@echo ""
	@echo "Development:"
	@echo "  make shell           Open bash shell in container"
	@echo "  make size            Show binary size analysis"
	@echo "  make size-components Size breakdown by component"
	@echo "  make size-files      Size breakdown by source files"
	@echo ""
	@echo "Docker:"
	@echo "  make image-build     Build Docker image"
	@echo "  make image-pull      Pull base image"
	@echo "  make image-status    Show Docker image info"
	@echo ""
	@echo "Pairing:"
	@echo "  make generate-pairing Generate random pairing code and QR"
	@echo ""
	@echo "Current PORT: $(PORT)"
	@echo ""
	@echo "Override port: make flash PORT=/dev/ttyUSB0"

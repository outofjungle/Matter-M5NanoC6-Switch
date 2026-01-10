# Matter M5NanoC6 Switch - Makefile

.PHONY: all build clean fullclean flash monitor erase \
        menuconfig generate-pairing help

# Default target
all: build

#------------------------------------------------------------------------------
# Configuration
#------------------------------------------------------------------------------

TARGET := esp32c6
PORT ?= $(shell ls /dev/cu.usbmodem* 2>/dev/null | head -1)

# Pairing configuration
PAIRING_CONFIG := main/include/CHIPPairingConfig.h
PAIRING_QR_IMAGE := pairing_qr.png

#------------------------------------------------------------------------------
# Build
#------------------------------------------------------------------------------

build: ## Build firmware
ifndef ESP_MATTER_PATH
	$(error ESP_MATTER_PATH not set)
endif
ifndef IDF_PATH
	$(error IDF_PATH not set)
endif
	idf.py -D IDF_TARGET=$(TARGET) build

clean: ## Clean build artifacts
	idf.py clean

fullclean: ## Full clean (removes build, sdkconfig, managed_components)
	rm -rf build managed_components
	rm -f sdkconfig sdkconfig.old dependencies.lock

menuconfig: ## Open SDK configuration
	idf.py menuconfig

#------------------------------------------------------------------------------
# Flash & Monitor
#------------------------------------------------------------------------------

flash: build ## Flash firmware
	@test -n "$(PORT)" || (echo "Error: No device found. Set PORT=" && exit 1)
	idf.py -p $(PORT) flash

monitor: ## Serial monitor (Ctrl+] to exit)
	@test -n "$(PORT)" || (echo "Error: No device found. Set PORT=" && exit 1)
	idf.py -p $(PORT) monitor

erase: ## Erase flash (factory reset)
	@test -n "$(PORT)" || (echo "Error: No device found. Set PORT=" && exit 1)
	esptool.py --port $(PORT) erase_flash

#------------------------------------------------------------------------------
# Pairing
#------------------------------------------------------------------------------

generate-pairing: ## Generate random pairing code and QR image
	@python3 -c "import random; \
invalid={0,11111111,22222222,33333333,44444444,55555555,66666666,77777777,88888888,99999999,12345678,87654321}; \
d=random.randint(0,4095); \
p=random.randint(1,99999999); \
exec('while p in invalid: p=random.randint(1,99999999)'); \
print(f'{d} {p}')" | xargs -n2 sh -c 'python3 scripts/generate_pairing_config.py -d $$0 -p $$1 -o $(PAIRING_CONFIG) --qr-image $(PAIRING_QR_IMAGE)'

#------------------------------------------------------------------------------
# Help
#------------------------------------------------------------------------------

help: ## Show this help
	@echo "Usage: make [target] [PORT=/dev/...]"
	@echo ""
	@echo "Build:"
	@echo "  build            Build firmware"
	@echo "  clean            Clean build artifacts"
	@echo "  fullclean        Full clean (build, sdkconfig, deps)"
	@echo "  menuconfig       Open SDK configuration"
	@echo ""
	@echo "Flash:"
	@echo "  flash            Flash firmware to device"
	@echo "  monitor          Open serial monitor"
	@echo "  erase            Erase flash (factory reset)"
	@echo ""
	@echo "Pairing:"
	@echo "  generate-pairing Generate random pairing code and QR"
	@echo ""
	@echo "Current PORT: $(PORT)"

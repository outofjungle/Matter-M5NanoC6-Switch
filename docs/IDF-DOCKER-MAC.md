# ESP-IDF Docker on macOS

This document covers using the ESP-IDF Docker image for building firmware on macOS.

## Overview

This project uses:
- **Docker containers** for building firmware (ensures consistent, reproducible builds)
- **Host-native tools** for flashing and monitoring (direct USB access)

## Why This Approach?

**Docker for Mac does NOT support USB device passthrough.**

This is a fundamental limitation of how Docker works on macOS (it runs in a virtual machine). Unlike Linux, you cannot directly access USB devices from Docker containers.

**GitHub Issue**: [docker/for-mac#900](https://github.com/docker/for-mac/issues/900)

This means you cannot use `idf.py flash` or `idf.py monitor` directly from within Docker containers on macOS.

## The Solution

### 1. Build in Docker (Default)

All firmware compilation happens in Docker containers using the official ESP-Matter image:

```bash
make build          # Build firmware in Docker
make menuconfig     # Interactive config in Docker
make clean          # Clean build in Docker
```

**Benefits:**
- Consistent build environment across all developers
- No need to install ESP-IDF or ESP-Matter on your Mac
- Reproducible builds
- Easy cleanup (just remove Docker image)

### 2. Flash/Monitor with Host Tools

Flashing and serial monitoring use native macOS tools (esptool installed via Homebrew):

```bash
make flash          # Flash with host esptool
make monitor        # Monitor with logging
make erase          # Erase flash
```

**Benefits:**
- Direct USB access (no virtualization layer)
- Fast and reliable serial communication
- Native macOS serial drivers

## Setup

### Prerequisites

1. **Docker Desktop for Mac** - [Install Docker Desktop](https://docs.docker.com/desktop/install/mac-install/)
2. **esptool** - For flashing firmware
3. **esp-idf-monitor** (optional) - For better serial monitoring

### Install Host Tools

**Required:** Install esptool for flashing:

```bash
brew install esptool
```

**Optional:** Install esp-idf-monitor for enhanced serial monitoring (colored output, stack trace decoding):

```bash
pip3 install --user esp-idf-monitor
```

The monitor target will automatically use esp-idf-monitor if available, otherwise falls back to `screen` (built into macOS).

### Build Docker Image

```bash
make image-build
```

This downloads and builds the Docker image (~2-3GB, takes 10-20 minutes first time).

## Workflow

### Typical Development Cycle

1. **Build firmware in Docker:**
   ```bash
   make build
   ```

2. **Flash to device with host tools:**
   ```bash
   make flash
   ```

3. **Monitor serial output with logging:**
   ```bash
   make monitor
   ```

   Logs are automatically saved to `logs/monitor_YYYYMMDD_HHMMSS.log`

### Configuration Changes

When you need to change SDK settings:

```bash
make menuconfig     # Opens interactive config in Docker
make build          # Rebuild with new config
make flash          # Flash updated firmware
```

## How It Works

### Build Process (Docker)

When you run `make build`, the Makefile:
1. Runs `docker-compose run --rm esp-idf`
2. Mounts your project directory to `/project` in the container
3. Executes `idf.py -C /project -D IDF_TARGET=esp32c6 build` inside the container
4. Outputs build artifacts to `build/` directory on your Mac

**Why `-C /project` is needed:**
- Even though `docker-compose.yml` sets `working_dir: /project`, the ESP-IDF activation script (`. $IDF_PATH/export.sh`) may change the working directory
- The `-C /project` flag explicitly tells `idf.py` where to find our project's `CMakeLists.txt`
- This ensures the build always targets the correct project directory

The build directory is shared between Docker and your Mac, so the host tools can access the compiled firmware.

### Flash Process (Host)

When you run `make flash`, the Makefile:
1. Detects your ESP32-C6 device (usually `/dev/cu.usbmodem*`)
2. Runs `esptool` natively on your Mac
3. Flashes the firmware from `build/` directory to the device

No Docker container is involved in flashing - it's direct USB communication.

### Monitor Process (Host)

Serial monitoring also runs natively:
- If `esp-idf-monitor` is installed: Uses that (better stack traces, colored output)
- Otherwise: Falls back to `screen` (built into macOS)

Exit with:
- **esp-idf-monitor**: `Ctrl+]`
- **screen**: `Ctrl+A` then `K`

## Serial Port Detection

The Makefile automatically detects the ESP32-C6 device:

```bash
# Auto-detects /dev/cu.usbmodem* or /dev/ttyUSB* or /dev/ttyACM*
make flash
```

Override if needed:

```bash
make flash PORT=/dev/cu.usbmodem14201
```

Check available ports:

```bash
ls /dev/cu.usbmodem*    # ESP32-C6 on macOS
```

## Advanced Docker Usage

### Interactive Shell

Open a bash shell inside the container:

```bash
make shell
```

Inside the container, you can run any ESP-IDF commands:

```bash
# Size analysis
idf.py size                # Overall size
idf.py size-components     # Size by component
idf.py size-files          # Size by source file

# Configuration
idf.py menuconfig          # Interactive config
idf.py reconfigure         # Reconfigure from sdkconfig.defaults

# Advanced debugging
idf.py app                 # Build only app (not bootloader)
idf.py partition-table     # Show partition table
```

## Troubleshooting

### Docker Build Fails

```bash
# Clean Docker cache and rebuild
docker-compose down
docker system prune -f
make image-build
```

### Device Not Found

Check USB connection and drivers:

```bash
ls -la /dev/cu.*
```

If device isn't showing up:
- Check USB cable (some are power-only)
- Install CH340/CP210x drivers if needed
- Try a different USB port

### Build Artifacts Owned by Root

The ESP-IDF Docker image handles file permissions automatically. If you encounter root-owned files:

```bash
sudo chown -R $USER:$USER build/ managed_components/
```

### Slow Builds

First build is slow due to:
- Downloading Docker base image (~2.5GB)
- Installing ESP-Matter dependencies
- Initial component compilation

Subsequent builds are much faster due to:
- Docker layer caching
- Incremental compilation
- Persistent build volume

## References

- [ESP-IDF Docker Image Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/tools/idf-docker-image.html)
- [esptool Documentation](https://docs.espressif.com/projects/esptool/en/latest/)
- [Docker for Mac USB Limitation Issue](https://github.com/docker/for-mac/issues/900)

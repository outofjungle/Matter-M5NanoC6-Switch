# ESP-IDF Docker Image on macOS

Reference: [ESP-IDF Docker Image Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/tools/idf-docker-image.html)

## Overview

This document covers using the ESP-IDF Docker image on macOS, including setup, limitations, and workarounds.

## Prerequisites

- Docker Desktop for Mac installed ([docker.com/install](https://docs.docker.com/install/))

## Basic Usage

Build a project using the ESP-IDF Docker container:

```bash
docker run --rm \
  -v $PWD:/project \
  -w /project \
  -u $UID \
  -e HOME=/tmp \
  espressif/idf \
  idf.py build
```

### Command Breakdown

- `--rm` - Remove container after execution
- `-v $PWD:/project` - Mount current directory to `/project` in container
- `-w /project` - Set working directory to `/project`
- `-u $UID` - Run as current user to maintain file permissions
- `-e HOME=/tmp` - Set temporary home directory
- `espressif/idf` - Official ESP-IDF Docker image

## macOS-Specific Limitations

### USB Device Access (Critical Limitation)

**Docker for Mac does NOT support USB device passthrough to containers.**

This means you cannot:
- Flash firmware directly to ESP32/ESP32-C6 devices via USB
- Monitor serial output through Docker container
- Use `idf.py flash` or `idf.py monitor` commands with physical devices

**GitHub Issue**: [docker/for-mac#900](https://github.com/docker/for-mac/issues/900)

### Solution: Hybrid Docker + Host Tools Approach

This project uses a hybrid approach:

1. **Docker for Building** - All compilation happens in Docker for consistency
2. **Host esptool for Flashing** - Use native esptool.py on macOS for device communication

This avoids USB passthrough limitations while maintaining a clean, reproducible build environment.

## Git Repository Considerations

If your mounted project directory is a git repository with different ownership, you may encounter git errors.

**Solution**: Set the safe directory environment variable:

```bash
docker run --rm \
  -v $PWD:/project \
  -w /project \
  -u $UID \
  -e HOME=/tmp \
  -e IDF_GIT_SAFE_DIR='/project' \
  espressif/idf \
  idf.py build
```

## Recommended Workflow for Mac

This project uses a hybrid approach that avoids USB passthrough limitations:

1. **Build in Docker** - Use Docker container for compilation to ensure consistent environment
2. **Flash/Monitor on Host** - Use native esptool on macOS for device communication

Install esptool on macOS:
```bash
brew install esptool
```

Optional - install esp-idf-monitor for better serial monitoring (stack trace decoding, colored output):
```bash
pip install esp-idf-monitor
```

Then use the Makefile targets:
```bash
make build          # Build in Docker
make flash          # Flash using host esptool
make monitor        # Monitor (uses esp-idf-monitor if available, else screen)
make flash-monitor  # Flash and monitor
```

## Common Commands

### Build Only
```bash
docker run --rm -v $PWD:/project -w /project -u $UID -e HOME=/tmp espressif/idf idf.py build
```

### Clean Build
```bash
docker run --rm -v $PWD:/project -w /project -u $UID -e HOME=/tmp espressif/idf idf.py fullclean
```

### Configure (menuconfig)
```bash
docker run --rm -it -v $PWD:/project -w /project -u $UID -e HOME=/tmp espressif/idf idf.py menuconfig
```

Note: `-it` flag required for interactive terminal

## Troubleshooting

### Permission Errors
- Ensure `-u $UID` is set to match your host user ID
- Check that mounted directory has appropriate permissions

### Git Errors
- Add `-e IDF_GIT_SAFE_DIR='/project'` environment variable

### USB Device Not Found
- This is expected behavior on macOS
- Use host-native esptool for device communication

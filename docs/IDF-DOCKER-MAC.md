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

### Workaround: Remote Serial Ports (RFC2217)

For flashing and monitoring, use remote serial port access via RFC2217 protocol:

1. Set up a serial port server on the host machine
2. Connect to it from within the Docker container over the network
3. This allows `idf.py flash` and `idf.py monitor` to work through the networked connection

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

Given the USB limitations, consider this hybrid approach:

1. **Build in Docker** - Use Docker container for compilation to ensure consistent environment
2. **Flash on Host** - Install ESP-IDF tools natively on Mac for flashing/monitoring
3. **Alternative**: Use RFC2217 remote serial if you prefer pure Docker workflow

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
- Use host-native tools or RFC2217 workaround for device communication

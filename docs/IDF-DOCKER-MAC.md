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

**This project includes a complete RFC2217 setup. See the "RFC2217 Setup" section below for details.**

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

---

## RFC2217 Setup

This project includes a complete RFC2217 setup for remote serial port access from Docker.

### Overview

RFC2217 is a protocol that allows serial port access over TCP/IP. The setup includes:
- **ser2net** - Serial port server running on macOS host
- **Helper scripts** - Easy-to-use scripts for common operations
- **Docker integration** - Automatic connection to host serial port

### Installation

ser2net is installed via Homebrew:

```bash
brew install ser2net
```

### Configuration Files

- `ser2net.yaml` - ser2net configuration (auto-updates device path)
- `scripts/ser2net-start.sh` - Start serial port server
- `scripts/ser2net-stop.sh` - Stop serial port server
- `scripts/docker-flash.sh` - Flash from Docker via RFC2217
- `scripts/docker-monitor.sh` - Monitor from Docker via RFC2217
- `scripts/docker-flash-monitor.sh` - Flash and monitor in one command

### Quick Start

#### 1. Connect ESP32-C6 Device

Connect your ESP32-C6 to your Mac via USB.

#### 2. Start ser2net Server

In one terminal window:

```bash
./scripts/ser2net-start.sh
```

This script will:
- Detect your ESP32-C6 device automatically
- Update `ser2net.yaml` with the correct device path
- Start ser2net on port 4000
- Display the connection string for Docker

Keep this terminal open while developing.

#### 3. Use Helper Scripts

In another terminal window, use the helper scripts:

```bash
# Flash only
./scripts/docker-flash.sh

# Monitor only
./scripts/docker-monitor.sh

# Flash and monitor
./scripts/docker-flash-monitor.sh
```

### Manual Usage

If you prefer to use `idf.py` directly:

```bash
# Flash
docker compose run --rm esp-idf \
  idf.py -p rfc2217://host.docker.internal:4000 flash

# Monitor
docker compose run --rm esp-idf \
  idf.py -p rfc2217://host.docker.internal:4000 monitor

# Flash and monitor
docker compose run --rm esp-idf \
  idf.py -p rfc2217://host.docker.internal:4000 flash monitor
```

### How It Works

1. **Host Side**: ser2net listens on TCP port 4000 and connects to the physical serial device
2. **Docker Side**: ESP-IDF tools connect to `host.docker.internal:4000` using RFC2217 protocol
3. **Communication**: Serial data is tunneled over TCP between Docker and the physical device

### Port Configuration

The default port is **4000**. To change it:

1. Edit `ser2net.yaml`:
   ```yaml
   accepter: telnet(rfc2217),tcp,0.0.0.0,4000  # Change 4000 to desired port
   ```

2. Update helper scripts to use the new port

### Troubleshooting

#### Device Not Found

```bash
# List available serial devices
ls -la /dev/cu.*

# Common ESP32-C6 device patterns:
# - /dev/cu.usbmodem*
# - /dev/cu.usbserial*
```

Update `ser2net.yaml` if the device path is different:
```yaml
esp32_device: /dev/cu.usbmodem101  # Update this line
```

#### Connection Refused from Docker

1. Check ser2net is running: `pgrep -f ser2net`
2. Test from host: `nc -zv localhost 4000`
3. Restart ser2net: `./scripts/ser2net-stop.sh && ./scripts/ser2net-start.sh`

#### Slow Flashing

Increase buffer sizes in `ser2net.yaml`:
```yaml
options:
  dev-to-net-bufsize: 8192
  net-to-dev-bufsize: 8192
```

#### Port Already in Use

Another ser2net instance may be running:
```bash
# Find and kill existing instance
pkill -f ser2net

# Or use the stop script
./scripts/ser2net-stop.sh
```

### Advanced Configuration

#### Custom Baud Rates

Edit `ser2net.yaml`:
```yaml
defines:
  flash_baud: 921600n81    # Faster flashing
  monitor_baud: 115200n81  # Standard monitoring
```

#### Multiple Devices

Add additional connections in `ser2net.yaml`:
```yaml
connections:
  esp32c6:
    accepter: telnet(rfc2217),tcp,0.0.0.0,4000
    connector: serialdev,/dev/cu.usbmodem101,115200n81,local

  esp32c3:
    accepter: telnet(rfc2217),tcp,0.0.0.0,4001
    connector: serialdev,/dev/cu.usbmodem102,115200n81,local
```

#### Background Service

To run ser2net as a background service:

```bash
# Start and enable on boot
brew services start ser2net

# Stop service
brew services stop ser2net
```

Note: When using `brew services`, edit `/opt/homebrew/etc/ser2net/ser2net.yaml` instead of the project's `ser2net.yaml`.

### Performance Tips

1. **Use higher baud rates** for flashing (921600 instead of 115200)
2. **Increase buffer sizes** in ser2net.yaml
3. **Use flash-monitor** script to avoid reconnecting between flash and monitor

### References

- [RFC2217 Specification](https://tools.ietf.org/html/rfc2217)
- [ser2net Documentation](https://linux.die.net/man/8/ser2net)
- [ESP-IDF Remote Serial Ports](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/tools/idf-docker-image.html#using-remote-serial-port)

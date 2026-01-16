# Helper Scripts

This directory contains helper scripts for ESP32-C6 development with Docker on macOS.

## RFC2217 Serial Port Forwarding

### ser2net Scripts

Start and stop the RFC2217 serial port server on macOS:

- `ser2net-start.sh` - Start ser2net server (auto-detects ESP32-C6)
- `ser2net-stop.sh` - Stop ser2net server

### Docker Development Scripts

Use these scripts for common ESP-IDF operations from Docker:

- `docker-flash.sh` - Flash firmware to ESP32-C6
- `docker-monitor.sh` - Monitor serial output
- `docker-flash-monitor.sh` - Flash and monitor in one command

## Quick Start

### 1. Connect Device

Connect your ESP32-C6 to your Mac via USB.

### 2. Start Serial Port Server

In one terminal:

```bash
./scripts/ser2net-start.sh
```

Keep this running while you develop.

### 3. Flash and Monitor

In another terminal:

```bash
# Build, flash, and monitor
docker compose run --rm esp-idf idf.py build
./scripts/docker-flash-monitor.sh
```

## Detailed Documentation

See [docs/IDF-DOCKER-MAC.md](../docs/IDF-DOCKER-MAC.md) for complete RFC2217 setup documentation.

## Troubleshooting

### Device Not Found

```bash
ls -la /dev/cu.*
```

Update `ser2net.yaml` with your device path if needed.

### Connection Issues

```bash
# Check ser2net is running
pgrep -f ser2net

# Test connection
nc -zv localhost 4000

# Restart ser2net
./scripts/ser2net-stop.sh
./scripts/ser2net-start.sh
```

## Why RFC2217?

Docker for Mac does not support USB device pass-through. RFC2217 works around this limitation by:

1. Running a serial port server (ser2net) on the macOS host
2. Connecting to it from Docker containers over TCP/IP
3. Tunneling serial data between Docker and the physical device

This allows `idf.py flash` and `idf.py monitor` to work from within Docker containers.

# Matter M5NanoC6 Switch

ESP32-C6 Matter-enabled switch using esp-matter SDK with Thread networking.

## Features

- **Toggle Control**: Button press toggles ON/OFF state
- **Matter Integration**: State syncs with Matter fabric
- **LED Indicator**: WS2812 LED shows state (bright blue=ON, dim blue=OFF)
- **Factory Reset**: Hold button 20 seconds to reset
- **Configurable Pairing**: Generate unique QR codes per device

**Matter Device Type**: On/Off Plug-in Unit (0x010A) over Thread

## Matter Device Model

Matter devices follow a hierarchical data model: **Node → Endpoints → Clusters → Attributes/Commands**

```
M5NanoC6 Switch (Node)
│
├── Endpoint 0: Root Node (reserved)
│   ├── Basic Information Cluster
│   │   ├── VendorName: "M5Stack"
│   │   ├── ProductName: "M5NanoC6 Switch"
│   │   ├── VendorID: 0xFFF2
│   │   └── ProductID: 0x0010
│   ├── Network Commissioning Cluster (Thread)
│   ├── General Commissioning Cluster
│   └── Access Control Cluster
│
└── Endpoint 1: On/Off Plug-in Unit (0x010A)
    ├── Identify Cluster
    │   └── Identify command → LED blinks white
    ├── Groups Cluster
    ├── Scenes Cluster
    └── On/Off Cluster (server)
        ├── Attributes:
        │   └── OnOff (bool) → LED state
        └── Commands:
            ├── On
            ├── Off
            └── Toggle
```

> **⚠️ DEVELOPMENT ONLY - NOT FOR PRODUCTION USE**
>
> This device uses **test-only Matter credentials** that are publicly known:
> - **Vendor ID 0xFFF2**: Reserved for testing, explicitly forbidden in production
> - **Example DAC Provider**: Uses insecure public test certificates
> - **Test Pairing Codes**: Default credentials are public knowledge
>
> **Security Implications:**
> - Any device with test credentials can be impersonated
> - Attackers can extract secrets from flash memory (no encryption enabled)
> - Debug interfaces remain active in default configuration
>
> **For production deployment**, you MUST:
> 1. Obtain an official Vendor ID from CSA (Connectivity Standards Alliance)
> 2. Use production Device Attestation Certificates (DAC)
> 3. Enable secure boot and flash encryption
> 4. Disable debug interfaces (Matter Shell, JTAG)
>
> See [docs/PRODUCTION-SECURITY-CHECKLIST.md](docs/PRODUCTION-SECURITY-CHECKLIST.md) for complete production hardening requirements.

### How It Works

1. **Node Creation** (`app_main.cpp:217`): Creates the Matter node with device info
2. **Endpoint Creation** (`app_main.cpp:223`): Adds On/Off Plug-in Unit endpoint with required clusters
3. **Attribute Callback** (`app_main.cpp:138`): When OnOff attribute changes, updates LED
4. **Button Press** (`app_main.cpp:152`): Reads OnOff attribute, toggles it, triggers callback

The esp-matter SDK handles cluster creation automatically based on the device type. See [Matter Clusters, Attributes, Commands](https://developer.espressif.com/blog/matter-clusters-attributes-commands/) for more details.

## Hardware

[M5Stack Nano C6 Dev Kit](https://shop.m5stack.com/products/m5stack-nanoc6-dev-kit)

| Peripheral | GPIO | Function |
|------------|------|----------|
| Button     | 9    | Active low |
| LED Data   | 20   | WS2812 data |
| LED Power  | 19   | WS2812 enable |

## Prerequisites

- **Docker Desktop** or Docker Engine + Docker Compose v2+
- macOS, Linux, or Windows with WSL2
- USB serial port access (for flashing)
- Python 3 (for pairing QR code generation)

No ESP-IDF or esp-matter installation required - everything runs in Docker containers!

## Setup

### 1. Install Docker

**macOS/Windows**: Install [Docker Desktop](https://www.docker.com/products/docker-desktop)

**Linux**:
```bash
# Install Docker
curl -fsSL https://get.docker.com | sh
sudo usermod -aG docker $USER
# Log out and back in for group changes to take effect

# Install Docker Compose (if not included)
sudo apt-get install docker-compose-plugin
```

### 2. Build Docker Image (First Time Only)

Build the Docker image with ESP-IDF and ESP-Matter SDK (~2-3GB, takes 10-20 minutes):

```bash
make image-build
```

This creates a self-contained Docker image with:
- ESP-IDF v5.3.4
- ESP-Matter SDK (latest)
- All build tools and dependencies

**Note**: This is a one-time step. The image is cached locally.

### 3. Serial Port Access (Linux only)

Add your user to the dialout group:
```bash
sudo usermod -aG dialout $USER
# Log out and back in
```

## Build and Flash

All commands run in Docker containers:

```bash
make build      # Build firmware
make flash      # Flash to device
make monitor    # Serial monitor (Ctrl+] to exit)
```

### All Make Targets

```bash
# Build
make build            # Build firmware in container
make clean            # Clean build artifacts
make fullclean        # Full clean (build, sdkconfig, deps)
make menuconfig       # SDK configuration (interactive)
make rebuild          # Full clean + rebuild

# Flash & Monitor
make flash            # Flash firmware to device
make monitor          # Serial monitor (Ctrl+] to exit)
make flash-monitor    # Flash and immediately monitor
make erase            # Erase flash (factory reset)

# Development
make shell            # Open bash shell in container
make size             # Show binary size analysis
make size-components  # Size breakdown by component
make size-files       # Size breakdown by files

# Docker
make image-build      # Build Docker image
make image-pull       # Pull base ESP-IDF image
make image-status     # Show Docker image info

# Pairing
make generate-pairing # Generate random pairing code and QR
```

### Override Serial Port

```bash
make flash PORT=/dev/ttyUSB0
```

## Commissioning

Generate a unique pairing code:

```bash
make generate-pairing
```

This creates:
- `main/include/CHIPPairingConfig.h` - Pairing configuration
- `pairing_qr.png` - QR code image for commissioning

Rebuild after generating:
```bash
make build && make flash
```

The device is discoverable via BLE during commissioning.

## Project Structure

```
Matter-M5NanoC6-Switch/
├── CMakeLists.txt
├── Makefile
├── sdkconfig.defaults
├── partitions.csv
├── scripts/
│   └── generate_pairing_config.py
└── main/
    ├── CMakeLists.txt
    ├── app_main.cpp          # Entry point, Matter setup
    ├── app_driver.cpp        # LED and button drivers
    ├── app_priv.h            # GPIO definitions
    ├── app_reset.cpp         # Factory reset handler
    ├── app_reset.h
    └── include/
        ├── CHIPProjectConfig.h   # Device naming
        └── CHIPPairingConfig.h   # Pairing config (generated)
```

## Troubleshooting

### Docker Image Build Fails

If the image build fails during ESP-Matter clone:
```bash
# Clean up and retry
docker-compose down
docker system prune -f
make image-build
```

### Permission Denied (Docker)

If you get "permission denied" errors:
```bash
# Linux: Add user to docker group
sudo usermod -aG docker $USER
# Log out and back in

# macOS/Windows: Ensure Docker Desktop is running
```

### Build Artifacts Owned by Root

If files in `build/` are owned by root:
```bash
# Fix ownership
sudo chown -R $USER:$USER build/ managed_components/

# Prevention: Ensure UID/GID are exported
export UID=$(id -u)
export GID=$(id -g)
make build
```

### Device Not Detected

Check serial port:
```bash
ls /dev/cu.usbmodem*   # macOS
ls /dev/ttyUSB*        # Linux
```

If not found:
- Check USB cable connection
- Check device drivers (CP210x or similar)
- Linux: Ensure user is in dialout group

### Flash Fails with "No Such Device"

The device path may be different. Override with:
```bash
make flash PORT=/dev/cu.usbmodem14201  # macOS
make flash PORT=/dev/ttyUSB0           # Linux
```

### Container Can't Access Serial Port

**Linux**: Ensure user is in dialout group
```bash
sudo usermod -aG dialout $USER
# Log out and back in
```

**macOS**: Device pass-through should work automatically

**Alternative**: Edit `docker-compose.yml` and uncomment `privileged: true`

### Slow Builds

First build in Docker is slow (~10-20 min) due to:
- Downloading ESP-IDF base image (~2.5GB)
- Cloning ESP-Matter SDK (~1GB)
- Installing Python dependencies

**Subsequent builds are fast** due to Docker layer caching and persistent build volumes.

To check cache status:
```bash
docker system df
```

### Clean Everything

```bash
make fullclean      # Clean project build artifacts
docker system prune -f --volumes  # Clean Docker cache
```

### Factory Reset

Hold button for 20 seconds. LED blinks red slowly during countdown. After 20 seconds, LED turns solid red (user can release button). Device resets automatically.

**Security Note:** Factory reset removes:
- All Matter fabric credentials and access control lists
- Thread network credentials
- Commissioned state and pairing information

However, factory reset does NOT erase:
- Factory partition data (vendor ID, product ID, DAC certificates)
- Application firmware
- Persistent configuration in flash

Before transferring device ownership or disposing of the device, ensure sensitive data is cleared appropriately. With flash encryption disabled (default), device secrets remain readable from flash memory even after factory reset.

# Matter M5NanoC6 Switch

ESP32-C6 Matter-enabled switch using esp-matter SDK with WiFi or Thread networking.

## User Guide

- **[USER-GUIDE.md](USER-GUIDE.md)** - Complete guide for device owners (commissioning, usage, troubleshooting)

## Security Considerations

> **⚠️ DEVELOPMENT ONLY - NOT FOR PRODUCTION USE**

### For End Users

This device uses **test-only credentials** and is **NOT secure for production use**:
- Uses publicly known test Vendor ID (0xFFF1)
- No flash encryption (credentials readable from device)
- Debug interfaces enabled
- Test pairing codes are public knowledge

**Suitable for:**
- Personal development and testing
- Learning Matter/Thread/WiFi protocols
- Prototyping and experimentation

**NOT suitable for:**
- Production deployment
- Controlling critical infrastructure
- Processing sensitive data
- Use in untrusted environments

### Before Commissioning

Before adding this device to your network:
- **Network Exposure**: Once commissioned, the device joins your WiFi or Thread network and can communicate with other devices
- **Test Credentials**: Anyone with physical access can extract secrets from the device
- **No Encryption**: Flash memory is not encrypted - device secrets are readable with basic tools
- **Personal Use Only**: Only commission this device on networks you fully control
- **Bluetooth Range**: During commissioning, the device broadcasts via BLE - ensure you're in a trusted environment

**Recommended:**
- Commission in a private location (not public spaces)
- Use a dedicated test/development network
- Do not use this device to control sensitive systems

### Factory Reset Security Notes

Factory reset **removes** these items:
- Matter fabric credentials and access control
- Thread or WiFi network credentials
- Commissioning state

Factory reset **does NOT remove**:
- Device attestation certificates (DAC)
- Factory configuration
- Application firmware
- **Secrets stored in flash memory** (not encrypted)

**Important:** Because this device does not use flash encryption, anyone with physical access and basic tools can read secrets from the device memory even after factory reset. If transferring ownership or disposing of the device, consider the flash contents as potentially readable.

### Thread Border Router Security (Thread builds)

If using Thread:
- Ensure your Border Router is from a trusted manufacturer
- Keep Border Router firmware up to date
- Use a secure WiFi network (WPA3 or WPA2 with strong password)
- Thread Border Router acts as gateway between Thread network and your home network
- Compromised Border Router can expose your entire Thread mesh network
- Do not use untrusted or unofficial Border Router firmware

## Features

- **Toggle Control**: Button press toggles ON/OFF state
- **Matter Integration**: State syncs with Matter fabric
- **LED Indicator**: WS2812 LED shows state (bright blue=ON, dim blue=OFF)
- **Factory Reset**: Hold button 20 seconds to reset, LED shows protocol-specific pattern
  - **Thread**: White (1) / Red (0) binary pattern
  - **WiFi**: Purple (1) / Blue (0) binary pattern
- **Configurable Pairing**: Generate unique QR codes per device
- **Dual Network Support**: Build for WiFi or Thread with `make build-wifi` or `make build-thread`

**Matter Device Type**: On/Off Plug-in Unit (0x010A) over WiFi or Thread

## Matter Device Model

Matter devices follow a hierarchical data model: **Node → Endpoints → Clusters → Attributes/Commands**

```
M5NanoC6 Switch (Node)
│
├── Endpoint 0: Root Node (reserved)
│   ├── Basic Information Cluster
│   │   ├── VendorName: "0x76656E Labs"
│   │   ├── ProductName: "M5NanoC6 Switch"
│   │   ├── VendorID: 0xFFF1
│   │   └── ProductID: 0x8000
│   ├── Network Commissioning Cluster (Thread)
│   ├── General Commissioning Cluster
│   └── Access Control Cluster
│
└── Endpoint 1: On/Off Plug-in Unit (0x010A)
    ├── Identify Cluster
    │   └── Identify command → LED displays binary pattern (2 repetitions)
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
> - **Vendor ID 0xFFF1**: Reserved for testing, explicitly forbidden in production
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
- **esptool** installed on host (for flashing)
- USB serial port access

No ESP-IDF or esp-matter installation required - firmware builds run in Docker!

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

### 2. Install esptool (Required for Flashing)

Install esptool on your host machine:

**macOS:**
```bash
brew install esptool
```

**Linux:**
```bash
pip3 install --user esptool
# Or: sudo apt-get install esptool
```

**Optional:** Install esp-idf-monitor for enhanced serial monitoring:
```bash
pip3 install --user esp-idf-monitor
```

### 3. Build Docker Image (First Time Only)

Build the Docker image with ESP-IDF and ESP-Matter SDK (~2-3GB, takes 10-20 minutes):

```bash
make image-build
```

This creates a self-contained Docker image with:
- ESP-IDF v5.4.1
- ESP-Matter SDK v1.5
- All build tools and dependencies

**Note**: This is a one-time step. The image is cached locally.

### 4. Serial Port Access (Linux only)

Add your user to the dialout group:
```bash
sudo usermod -aG dialout $USER
# Log out and back in
```

## Build and Flash

Build firmware in Docker, flash/monitor with host tools:

```bash
make build      # Build firmware in Docker (Thread, default)
make flash      # Flash to device with host esptool
make monitor    # Serial monitor with logging (Ctrl+] to exit)
```

### WiFi vs Thread Builds

Choose your network protocol at build time:

```bash
# Thread (default)
make build              # or: make build-thread

# WiFi
make build-wifi

# IMPORTANT: Run fullclean when switching protocols
make fullclean && make build-wifi
```

### All Make Targets

```bash
# Build (Docker - default)
make build            # Build firmware in Docker (Thread, default)
make build-thread     # Build Thread firmware explicitly
make build-wifi       # Build WiFi firmware
make clean            # Clean build artifacts
make rebuild          # Full clean + rebuild
make menuconfig       # SDK configuration (interactive)

# Build (Local - requires ESP-IDF installation)
make local-build      # Build firmware locally (Thread, default)
make local-build-thread # Build Thread firmware locally
make local-build-wifi # Build WiFi firmware locally
make local-clean      # Clean build artifacts locally
make local-rebuild    # Full clean + rebuild locally
make local-menuconfig # SDK configuration locally

# Flash & Monitor (host tools)
make flash            # Flash firmware to device
make monitor          # Monitor with logging to logs/
make erase            # Erase flash (factory reset)

# Docker Management
make image-build      # Build Docker image (~10-20 min, one-time)
make image-pull       # Pull base ESP-IDF image
make image-status     # Show Docker image info
make shell            # Open bash shell in container

# Utilities
make fullclean        # Full clean (build, sdkconfig, deps)
make generate-pairing # Generate random pairing code and QR
```

### Override Serial Port

```bash
make flash PORT=/dev/ttyUSB0
```

### Advanced: Running ESP-IDF Commands Directly

For advanced debugging and analysis, open a shell in the Docker container:

```bash
make shell
```

Then run any ESP-IDF commands. Note that you're already in `/project` directory:

```bash
# Inside the container shell (already in /project):
idf.py size                # Show binary size analysis
idf.py size-components     # Size breakdown by component
idf.py size-files          # Size breakdown by source files
idf.py -D SDKCONFIG_DEFAULTS=sdkconfig.defaults reconfigure  # Reconfigure

# Or use explicit path:
idf.py -C /project size
```

## Commissioning

### 1. Generate Pairing Code

```bash
make generate-pairing
```

This creates:
- `main/include/CHIPPairingConfig.h` - Pairing configuration
- `pairing_qr.png` - QR code image for commissioning

Rebuild after generating:
```bash
make build && make flash  # or: make build-wifi && make flash
```

### 2. Commission Your Device

For end-user commissioning instructions, see **[USER-GUIDE.md](USER-GUIDE.md)**.

For developers, the sections below provide detailed monitoring and debugging information.

### Monitoring During Commissioning

Monitor the device during commissioning to see connection progress:

```bash
make monitor
```

Press `Ctrl+]` to exit the monitor.

#### Expected Log Sequence (Thread)

```
I (1755) chip[DL]: CHIPoBLE advertising started
I (1785) app_main: Commissioning window opened

# After commissioning starts...
I (XXXX) chip[DL]: OpenThread started

# After Thread credentials received...
I (XXXX) OPENTHREAD: [N] Mle-----------: Role disabled -> detached
I (XXXX) OPENTHREAD: [N] Mle-----------: Attach attempt 1
I (XXXX) OPENTHREAD: [N] Mle-----------: Role detached -> child

# After joining Thread network...
I (XXXX) OPENTHREAD: [N] Mle-----------: Partition ID 0xXXXXXXXX
I (XXXX) chip[DL]: Thread network joined

# Commission complete
I (XXXX) chip[SVR]: Commissioning completed successfully
```

**Thread Role Progression:**
- **disabled** → **detached** → **child** → (potentially **router** if needed)

A "child" role is normal and sufficient for most end devices.

#### Expected Log Sequence (WiFi)

```
I (1685) app_main: WiFi not provisioned - AP mode active for commissioning
I (1755) chip[DL]: CHIPoBLE advertising started
I (1785) app_main: Commissioning window opened

# After commissioning starts...
I (XXXX) chip[DL]: WIFI_EVENT_STA_START
I (XXXX) chip[DL]: Done driving station state...

# After WiFi credentials received...
I (XXXX) wifi:new:<6,0>, old:<1,1>
I (XXXX) wifi:station: connected to YourSSID
I (XXXX) wifi:station: got ip

# Commission complete
I (XXXX) chip[SVR]: Commissioning completed successfully
```

### Advanced: chip-tool Commissioning

For developers who want to use chip-tool instead of consumer apps:

#### Installation

```bash
# Clone connectedhomeip
git clone https://github.com/project-chip/connectedhomeip.git
cd connectedhomeip

# Build chip-tool
./scripts/examples/gn_build_example.sh examples/chip-tool out/chip-tool
```

#### Thread Commissioning with chip-tool

```bash
# Get Thread operational dataset from your Thread Border Router
# Apple Home: Home app → Home Settings → Thread Network → Export
# ESP TBR: dataset active -x

# Format: chip-tool pairing ble-thread <node-id> hex:<dataset> <setup-pin> <discriminator>
./out/chip-tool/chip-tool pairing ble-thread 1 \
  hex:0e080000000000010000000300001235060004001fffe00208fedcba9876543210 \
  5143243 1568

# Parameters:
#   1              - Node ID (assign any unique ID)
#   hex:<dataset>  - Thread operational dataset in hex
#   5143243        - Setup PIN from CHIPPairingConfig.h
#   1568           - Discriminator from CHIPPairingConfig.h (0x620)
```

#### WiFi Commissioning with chip-tool

```bash
# Format: chip-tool pairing ble-wifi <node-id> <ssid> <password> <setup-pin> <discriminator>
./out/chip-tool/chip-tool pairing ble-wifi 1 "YourSSID" "YourPassword" 5143243 1568

# Parameters:
#   1           - Node ID (assign any unique ID)
#   YourSSID    - Your WiFi network name
#   YourPassword - Your WiFi password
#   5143243     - Setup PIN from CHIPPairingConfig.h
#   1568        - Discriminator from CHIPPairingConfig.h (0x620)
```

#### Controlling with chip-tool

```bash
# Read OnOff attribute
./out/chip-tool/chip-tool onoff read on-off 1 1

# Toggle the switch
./out/chip-tool/chip-tool onoff toggle 1 1

# Turn on
./out/chip-tool/chip-tool onoff on 1 1

# Turn off
./out/chip-tool/chip-tool onoff off 1 1
```

#### Thread Network Diagnostics

```bash
# Read Thread diagnostic info
./out/chip-tool/chip-tool threadnetworkdiagnostics read routing-role 1 1
./out/chip-tool/chip-tool threadnetworkdiagnostics read network-name 1 1
```

### Finding Pairing Info

**View in serial monitor:**
```bash
make monitor
```

Look for:
```
I (1695) app_main: === Commissioning Info ===
I (1695) app_main: Discriminator: 1568 (0x620)
I (1705) app_main: Passcode: 5143243
```

**View in source code:**
```bash
cat main/include/CHIPPairingConfig.h
```

**View QR code:**
```bash
open pairing_qr.png  # macOS
xdg-open pairing_qr.png  # Linux
```

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
```

The ESP-IDF Docker image should handle file permissions automatically.

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

### esptool Not Found

Install esptool on your host machine:

**macOS:**
```bash
brew install esptool
```

**Linux:**
```bash
pip3 install --user esptool
# Or: sudo apt-get install esptool
```

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

Hold button for ~23 seconds total:
1. **Initial delay (1 second)** - Release to cancel
2. **Binary pattern display (~19 seconds)** - Shows firmware config ID for QR code recovery (see [USER-GUIDE.md](USER-GUIDE.md#recovering-your-qr-code))
3. **Result indicator (3 seconds)**:
   - **Solid RED** = Reset will proceed (keep holding)
   - **Solid GREEN** = Reset cancelled (button released)

After solid RED, device resets automatically. LED returns to previous state after GREEN.

**Security Note:** Factory reset removes:
- All Matter fabric credentials and access control lists
- Thread network credentials
- Commissioned state and pairing information

However, factory reset does NOT erase:
- Factory partition data (vendor ID, product ID, DAC certificates)
- Application firmware
- Persistent configuration in flash

Before transferring device ownership or disposing of the device, ensure sensitive data is cleared appropriately. With flash encryption disabled (default), device secrets remain readable from flash memory even after factory reset.

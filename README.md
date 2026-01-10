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
│   │   ├── VendorID: 0xFFF1
│   │   └── ProductID: 0x8000
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

- macOS or Linux
- ESP-IDF v5.3.x
- esp-matter SDK

## Setup

### 1. Install ESP-IDF

```bash
cd ~/Workspace/ESP
git clone -b release/v5.3 --recursive https://github.com/espressif/esp-idf.git
cd esp-idf && ./install.sh esp32c6
```

### 2. Install esp-matter

```bash
cd ~/Workspace/ESP
git clone --recursive https://github.com/espressif/esp-matter.git
cd esp-matter && ./install.sh
```

### 3. Environment Variables

Add to shell profile:

```bash
export IDF_PATH=~/Workspace/ESP/esp-idf
export ESP_MATTER_PATH=~/Workspace/ESP/esp-matter
export _PW_ACTUAL_ENVIRONMENT_ROOT=$ESP_MATTER_PATH/connectedhomeip/connectedhomeip/.environment
export PATH=$_PW_ACTUAL_ENVIRONMENT_ROOT/cipd/packages/pigweed:$PATH
export PATH=$_PW_ACTUAL_ENVIRONMENT_ROOT/cipd/packages/pigweed/bin:$PATH
```

Activate ESP-IDF (once per terminal):
```bash
source $IDF_PATH/export.sh
```

## Build and Flash

```bash
make build      # Build firmware
make flash      # Flash to device
make monitor    # Serial monitor (Ctrl+] to exit)
```

### All Make Targets

```bash
make build            # Build firmware
make clean            # Clean build artifacts
make fullclean        # Full clean (build, sdkconfig, deps)
make menuconfig       # SDK configuration
make flash            # Flash firmware
make monitor          # Serial monitor
make erase            # Erase flash (factory reset)
make generate-pairing # Generate random pairing code and QR
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

### Build fails with "gn not found"
Ensure pigweed tools are in PATH:
```bash
echo $PATH | grep pigweed
```

### Device not detected
```bash
ls /dev/cu.usbmodem*   # macOS
ls /dev/ttyUSB*        # Linux
```

### Factory Reset
Hold button for 20 seconds. LED blinks red with increasing speed during countdown.

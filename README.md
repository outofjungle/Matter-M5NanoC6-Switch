# Matter M5NanoC6 Switch

ESP32-C6 Matter-enabled push button toggle switch using esp-matter SDK.

## Description

A simple push button toggle switch with the following capabilities:

* **Toggle Control**: Press button to toggle state between ON and OFF
* **Matter Integration**: State changes are reported to the Matter network
* **Indicator LED**: WS2812 RGB LED shows switch state (green=ON, red=OFF)
* **Factory Reset**: Long press button to trigger factory reset
* **Matter Data Model**:
  * **Device Type**: `On/Off Plug-in Unit` (0x010A)
  * **Network**: Thread (802.15.4)

## Hardware

* **Devkit**: [M5Stack Nano C6 Dev Kit](https://shop.m5stack.com/products/m5stack-nanoc6-dev-kit)

### Pin Assignment

| Peripheral    | GPIO Pin | Function                        |
|---------------|----------|---------------------------------|
| Button        | GPIO 9   | Built-in button (active low)    |
| LED Data      | GPIO 20  | WS2812 RGB LED data             |
| LED Power     | GPIO 19  | WS2812 power enable             |

## Prerequisites

- macOS or Linux
- ESP-IDF v5.3.x
- esp-matter SDK

## Setup

### 1. Install ESP-IDF

```bash
mkdir -p ~/Workspace/ESP
cd ~/Workspace/ESP

git clone -b release/v5.3 --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh esp32c6
```

### 2. Install esp-matter

```bash
cd ~/Workspace/ESP
git clone --recursive https://github.com/espressif/esp-matter.git
cd esp-matter
./install.sh
```

### 3. Environment Variables

Add to your shell profile (`~/.config/fish/config.fish` for fish, or `~/.zshrc`/`~/.bashrc`):

```bash
# ESP-IDF and ESP-Matter
export IDF_PATH=~/Workspace/ESP/esp-idf
export ESP_MATTER_PATH=~/Workspace/ESP/esp-matter
export _PW_ACTUAL_ENVIRONMENT_ROOT=$ESP_MATTER_PATH/connectedhomeip/connectedhomeip/.environment

# Pigweed tools (for Matter builds)
export PATH=$_PW_ACTUAL_ENVIRONMENT_ROOT/cipd/packages/pigweed:$PATH
export PATH=$_PW_ACTUAL_ENVIRONMENT_ROOT/cipd/packages/pigweed/bin:$PATH
```

Activate ESP-IDF tools (once per terminal):
```bash
source $IDF_PATH/export.sh    # bash/zsh
# OR
source $IDF_PATH/export.fish  # fish
```

## Build and Flash

```bash
cd ~/Workspace/Matter-M5NanoC6-Switch

# Build
idf.py build

# Flash and monitor
idf.py -p /dev/cu.usbmodem* flash monitor
```

### Available Make Targets

```bash
make build          # Build firmware
make flash          # Flash to device
make monitor        # Serial monitor
make flash-monitor  # Flash and monitor
make clean          # Clean build
make erase          # Erase flash
make menuconfig     # SDK configuration
```

## Commissioning

**Manual Pairing Code:** `34970112332`

Or use:
- **Passcode:** `20202021`
- **Discriminator:** `3840`

The device is discoverable via BLE during commissioning.

## Project Structure

```
Matter-M5NanoC6-Switch/
├── CMakeLists.txt          # Project build configuration
├── Makefile                # Build automation
├── sdkconfig.defaults      # SDK configuration
├── partitions.csv          # Flash partition table
└── main/
    ├── CMakeLists.txt      # Component configuration
    ├── app_main.cpp        # Application entry point
    ├── app_driver.cpp      # LED and button drivers
    ├── app_priv.h          # GPIO definitions
    ├── app_reset.cpp       # Factory reset handler
    └── app_reset.h         # Factory reset interface
```

## Code Overview

### Initialization (app_main.cpp)

1. Initialize NVS flash
2. Create Matter node with `on_off_plug_in_unit` endpoint
3. Register attribute update callback
4. Initialize LED driver (WS2812)
5. Initialize button with callbacks:
   - Single click: Toggle on/off state
   - Long press: Factory reset

### Hardware Drivers (app_driver.cpp)

* `app_driver_led_init()` - Initialize WS2812 LED with RMT driver
* `app_driver_led_set_power()` - Set LED color (green=ON, red=OFF)
* `app_driver_button_init()` - Initialize button with iot_button
* `app_driver_attribute_update()` - Handle Matter attribute changes

### GPIO Configuration (app_priv.h)

```cpp
#define M5NANOC6_BUTTON_GPIO      9
#define M5NANOC6_LED_DATA_GPIO    20
#define M5NANOC6_LED_POWER_GPIO   19
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
Hold button for 5+ seconds until reset triggers.

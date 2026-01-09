# ESP-Matter Development Reference Guide for ESP32-C6 Switch Devices

This comprehensive guide covers building Matter switch devices on ESP32-C6 using Espressif's SDK for Matter.

---

## Table of Contents

1. [Device Naming and Product Configuration](#1-device-naming-and-product-configuration)
2. [Commissioning Setup](#2-commissioning-setup)
3. [Factory Data and Manufacturing Partitions](#3-factory-data-and-manufacturing-partitions)
4. [Configuration Options (Kconfig/sdkconfig)](#4-configuration-options-kconfigsdkconfig)
5. [Thread/OpenThread Configuration](#5-threadopenthread-configuration)
6. [ESP32-C6 Specific Considerations](#6-esp32-c6-specific-considerations)

---

## 1. Device Naming and Product Configuration

### Method 1: Custom CHIPProjectConfig.h (Build-time) - RECOMMENDED

Create a custom configuration header file and reference it in sdkconfig:

```cpp
// main/include/CHIPProjectConfig.h
#pragma once

#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME "YourVendor"
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME "Smart Switch"
#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID 0xFFF1  // Test VID
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID 0x8001
#define CHIP_DEVICE_CONFIG_DEFAULT_DEVICE_HARDWARE_VERSION 1
```

Add to `sdkconfig.defaults`:
```
CONFIG_CHIP_PROJECT_CONFIG="main/include/CHIPProjectConfig.h"
```

### Method 2: Manufacturing Tool (Production)

Use `esp-matter-mfg-tool` to set vendor/product names in factory partition:

```bash
esp-matter-mfg-tool \
    -v 0x131B \
    --vendor-name "MyCompany" \
    -p 0x1000 \
    --product-name "Smart Switch" \
    --hw-ver 1 \
    --hw-ver-str "v1.0"
```

---

## 2. Commissioning Setup

### Default Commissioning Values

| Parameter | Default Value | Description |
|-----------|---------------|-------------|
| Setup Passcode | 20202021 | Proof of possession during pairing |
| Discriminator | 3840 (0xF00) | Distinguishes devices during discovery |
| Vendor ID | 0xFFF1 | Test vendor ID |
| Product ID | 0x8000 | Default product ID |
| QR Code | MT:Y.K9042C00KA0648G00 | Default onboarding QR |
| Manual Code | 34970112332 | Manual pairing code |

### QR Code Generation

```bash
# Using generate_setup_payload.py
python3 generate_setup_payload.py \
    --discriminator 3840 \
    --passcode 20202021 \
    --vendor-id 65521 \
    --product-id 32768 \
    --commissioning-flow 0 \
    --discovery-cap-bitmask 2
```

---

## 3. Factory Data and Manufacturing Partitions

### Partition Table Entry

```csv
# Name,   Type, SubType, Offset,   Size
fctry,    data, nvs,     0x3E0000, 0x6000
```

### Factory Data Provider Configuration

```
# Enable factory data providers
CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER=y
CONFIG_ENABLE_ESP32_DEVICE_INSTANCE_INFO_PROVIDER=y

# Partition labels
CONFIG_CHIP_FACTORY_NAMESPACE_PARTITION_LABEL="fctry"
```

### Using esp-matter-mfg-tool

```bash
pip install esp-matter-mfg-tool

esp-matter-mfg-tool \
    -v 0xFFF2 \
    -p 0x8001 \
    --vendor-name "MyCompany" \
    --product-name "Smart Switch" \
    --hw-ver 1 \
    --hw-ver-str "v1.0" \
    --passcode 19861989 \
    --discriminator 601 \
    --serial-num "SWITCH001" \
    -cd /path/to/Certification-Declaration.der
```

**Flashing Factory Partition:**
```bash
esptool.py -p /dev/ttyUSB0 write_flash 0x3E0000 <uuid>-partition.bin
```

---

## 4. Configuration Options (Kconfig/sdkconfig)

### Essential sdkconfig.defaults for ESP32-C6 Thread Device

```
# Target
CONFIG_IDF_TARGET="esp32c6"

# Device Identification
CONFIG_DEVICE_VENDOR_ID=0xFFF1
CONFIG_DEVICE_PRODUCT_ID=0x8000

# Custom project config for device naming
CONFIG_CHIP_PROJECT_CONFIG="main/include/CHIPProjectConfig.h"

# Thread Configuration
CONFIG_OPENTHREAD_ENABLED=y
CONFIG_ENABLE_WIFI_STATION=n
CONFIG_USE_MINIMAL_MDNS=n

# OpenThread Features
CONFIG_OPENTHREAD_SRP_CLIENT=y
CONFIG_OPENTHREAD_DNS_CLIENT=y

# Factory Data (for production)
CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER=y
CONFIG_ENABLE_ESP32_DEVICE_INSTANCE_INFO_PROVIDER=y
CONFIG_CHIP_FACTORY_NAMESPACE_PARTITION_LABEL="fctry"

# Memory Optimizations
CONFIG_NEWLIB_NANO_FORMAT=y
CONFIG_ESP_MATTER_MAX_DYNAMIC_ENDPOINT_COUNT=2

# OTA
CONFIG_ENABLE_OTA_REQUESTOR=y
```

---

## 5. Thread/OpenThread Configuration

### ESP32-C6 Thread Setup

```
# Core Thread Configuration
CONFIG_OPENTHREAD_ENABLED=y
CONFIG_ENABLE_WIFI_STATION=n
CONFIG_USE_MINIMAL_MDNS=n

# SRP Client (required for Matter)
CONFIG_OPENTHREAD_SRP_CLIENT=y
CONFIG_OPENTHREAD_DNS_CLIENT=y

# Thread Device Type
CONFIG_OPENTHREAD_FTD=y  # Full Thread Device
# or CONFIG_OPENTHREAD_MTD=y for low power
```

---

## 6. ESP32-C6 Specific Considerations

### Radio Selection

ESP32-C6 supports both WiFi and 802.15.4 (Thread), but not simultaneously.

### Memory Constraints

```
# Recommended optimizations for ESP32-C6
CONFIG_NEWLIB_NANO_FORMAT=y
CONFIG_ESP_MATTER_MAX_DYNAMIC_ENDPOINT_COUNT=2
CONFIG_ENABLE_CHIP_SHELL=n  # Disable in production
CONFIG_BT_NIMBLE_MAX_CONNECTIONS=1
CONFIG_LOG_DEFAULT_LEVEL_WARN=y
```

### Target Setup

```bash
idf.py set-target esp32c6
idf.py erase_flash
idf.py build flash monitor
```

---

## Sources

- [ESP-Matter Developing Guide](https://docs.espressif.com/projects/esp-matter/en/latest/esp32/developing.html)
- [ESP-Matter Production Guide](https://docs.espressif.com/projects/esp-matter/en/latest/esp32/production.html)
- [ESP-Matter FAQ](https://docs.espressif.com/projects/esp-matter/en/latest/esp32/faq.html)
- [ESP-Matter GitHub](https://github.com/espressif/esp-matter)

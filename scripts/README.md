# Helper Scripts

This directory contains helper scripts for Matter device development.

## generate_pairing_config.py

Generates Matter commissioning configuration including QR codes and SPAKE2+ verifiers.

### Usage

**Via Makefile (recommended):**

```bash
# Generate random pairing code and QR image
make generate-pairing
```

This creates:
- `main/include/CHIPPairingConfig.h` - C header with pairing configuration
- `pairing_qr.png` - QR code image for commissioning

**Direct usage:**

```bash
# Generate with specific values
python3 scripts/generate_pairing_config.py \
    -d 3840 \
    -p 20202021 \
    -o main/include/CHIPPairingConfig.h \
    --qr-image pairing_qr.png
```

### Parameters

| Parameter | Description | Example |
|-----------|-------------|---------|
| `-d, --discriminator` | Discriminator (0-4095) | 3840 |
| `-p, --passcode` | Passcode (1-99999999) | 20202021 |
| `--vendor-id` | Vendor ID (hex) | 0xFFF1 |
| `--product-id` | Product ID (hex) | 0x8000 |
| `--salt` | SPAKE2+ salt (base64) | U1BBS0UyUCBLZXkgU2FsdA== |
| `--iterations` | SPAKE2+ iterations | 1000 |
| `-o, --output` | Output header file path | - |
| `--qr-image` | QR code PNG output | pairing_qr.png |
| `--discovery` | Discovery capabilities (1=SoftAP, 2=BLE, 4=OnNetwork) | 2 |

### Examples

**Generate with custom pairing code:**

```bash
python3 scripts/generate_pairing_config.py \
    -d 2400 \
    -p 12345678 \
    -o main/include/CHIPPairingConfig.h \
    --qr-image pairing_qr.png
```

**Generate with custom vendor/product IDs:**

```bash
python3 scripts/generate_pairing_config.py \
    -d 3840 \
    -p 20202021 \
    --vendor-id 0x1234 \
    --product-id 0x5678 \
    -o main/include/CHIPPairingConfig.h
```

**Print config to stdout (no file output):**

```bash
python3 scripts/generate_pairing_config.py -d 3840 -p 20202021
```

### What It Generates

The script generates:

1. **SPAKE2+ Verifier** - Cryptographic verifier for secure commissioning
2. **QR Code** - Matter onboarding QR code (MT: format)
3. **Manual Pairing Code** - 11-digit code for manual entry
4. **C Header** - Complete configuration for CHIPPairingConfig.h

### Security Notes

**⚠️ IMPORTANT for Production:**

- **Random Passcodes**: Generate unique passcodes per device (not 20202021!)
- **Avoid Invalid Codes**: Script rejects invalid passcodes per Matter spec
- **Unique QR Codes**: Each device should have a unique QR code
- **Secure Storage**: Protect pairing codes during manufacturing

Invalid passcodes (automatically rejected):
- 00000000, 11111111, 22222222, 33333333, 44444444
- 55555555, 66666666, 77777777, 88888888, 99999999
- 12345678, 87654321

### Dependencies

**In Docker (already installed):**
- `ecdsa` - For SPAKE2+ calculations
- `qrcode` - For QR code generation
- `pillow` - For image handling

The script runs in Docker via `make generate-pairing`, so no host installation needed.

### Discovery Capabilities

The `--discovery` parameter sets the commissioning discovery method:

| Value | Method | Description |
|-------|--------|-------------|
| 1 | SoftAP | WiFi Access Point |
| 2 | BLE | Bluetooth Low Energy (default) |
| 4 | OnNetwork | Already on network |

For Thread devices commissioning via BLE (like ESP32-C6), use `2` (default).

### Output Format

**Console Output:**
```
======================================================================
Matter Pairing Configuration
======================================================================

Vendor ID:      0xFFF1
Product ID:     0x8000
Discriminator:  3840 (0xF00)
Passcode:       20202021

QR Code:        MT:Y.K9042C00KA0648G00
Manual Code:    34970112332
```

**CHIPPairingConfig.h:**
```c
#pragma once

/* Commissioning Parameters */
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR 0xF00
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE 20202021

/* SPAKE2+ Parameters */
#define CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT 1000
#define CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT "U1BBS0UyUCBLZXkgU2FsdA=="
#define CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER "..."
```

### Workflow

1. Generate pairing config: `make generate-pairing`
2. Rebuild firmware: `make build`
3. Flash to device: `make flash`
4. Commission using the generated QR code or manual code

### References

- [Matter Specification - Section 5.1.7](https://csa-iot.org/developer-resource/specifications-download-request/) - Setup Code Format
- [SPAKE2+ Algorithm](https://datatracker.ietf.org/doc/html/draft-irtf-cfrg-spake2-26) - Cryptographic details

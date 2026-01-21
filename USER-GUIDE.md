# M5NanoC6 Switch - User Guide

## What is this device?

The M5NanoC6 Switch is a Matter-enabled smart switch that works with Apple Home, Google Home, Amazon Alexa, and other Matter-compatible platforms. It connects to your home network via **Thread** (a low-power mesh networking protocol) and can be controlled from your smartphone or voice assistant.

**Features:**
- Toggle ON/OFF with physical button
- LED indicator shows current state (bright blue = ON, dim blue = OFF)
- Thread network support (requires a Thread Border Router)
- Works with all Matter-compatible ecosystems

> **⚠️ SECURITY WARNING - DEVELOPMENT DEVICE**
>
> This device uses **test-only credentials** and is **NOT secure for production use**:
> - Uses publicly known test Vendor ID (0xFFF1)
> - No flash encryption (credentials readable from device)
> - Debug interfaces enabled
> - Test pairing codes are public knowledge
>
> **Suitable for:**
> - Personal development and testing
> - Learning Matter/Thread protocols
> - Prototyping and experimentation
>
> **NOT suitable for:**
> - Production deployment
> - Controlling critical infrastructure
> - Processing sensitive data
> - Use in untrusted environments
>
> See [PRODUCTION-SECURITY-CHECKLIST.md](docs/PRODUCTION-SECURITY-CHECKLIST.md) for hardening requirements.

---

## Getting Started

### What You'll Need

1. **M5NanoC6 Switch** (the device you received)
2. **Thread Border Router** - One of the following:
   - Apple HomePod mini (2nd gen or later)
   - Apple TV 4K (2nd gen or later)
   - Google Nest Hub (2nd gen)
   - Google Nest WiFi Pro
   - Amazon Echo (4th gen or later)
   - Or any other Thread Border Router

   **Thread Border Router Security Requirements:**
   - Ensure your Border Router is from a trusted manufacturer
   - Keep Border Router firmware up to date
   - Use a secure WiFi network (WPA3 or WPA2 with strong password)
   - Thread Border Router acts as gateway between Thread network and your home network
   - Compromised Border Router can expose your entire Thread mesh network
   - Do not use untrusted or unofficial Border Router firmware

3. **Smartphone** with Matter-compatible app:
   - iPhone with iOS 16.5+ (Home app)
   - Android with Google Home app
   - Amazon Alexa app

### First Time Setup

When you first power on the device:
1. The LED will light up (dim blue)
2. The device enters **commissioning mode** automatically
3. It remains discoverable via Bluetooth LE for pairing

---

## Commissioning (Adding to Your Home)

> **⚠️ BEFORE COMMISSIONING - SECURITY CONSIDERATIONS**
>
> Before adding this device to your network:
> - **Network Exposure**: Once commissioned, the device joins your Thread network and can communicate with other Thread devices
> - **Test Credentials**: This device uses publicly known test credentials - anyone with physical access can extract secrets from the device
> - **No Encryption**: Flash memory is not encrypted - device secrets are readable with basic tools
> - **Personal Use Only**: Only commission this device on networks you fully control
> - **Bluetooth Range**: During commissioning, the device broadcasts via BLE - ensure you're in a trusted environment
>
> **Recommended:**
> - Commission in a private location (not public spaces)
> - Use a dedicated test/development Thread network
> - Do not use this device to control sensitive systems
> - For production use, see [PRODUCTION-SECURITY-CHECKLIST.md](docs/PRODUCTION-SECURITY-CHECKLIST.md)

### Using the QR Code (Recommended)

1. Open your Matter-compatible app:
   - **Apple Home**: Tap `+` → "Add Accessory"
   - **Google Home**: Tap `+` → "Set up device" → "New device"
   - **Amazon Alexa**: Tap "Devices" → `+` → "Add Device"

2. Scan this QR code:

<p align="center">
  <img src="pairing_qr.png" alt="Commissioning QR Code" width="300">
</p>

3. Follow the on-screen instructions in your app
4. The device will connect to your Thread network via your Border Router
5. Once added, you can control the switch from your app

**Note:** If QR scanning doesn't work, you can manually enter the pairing code shown below the QR code.

---

## Using Your Switch

### Physical Button

**Press once**: Toggle between ON and OFF
- The LED will change brightness to reflect the new state

**Hold for 20 seconds**: Factory reset (see below)

### LED Indicator

| LED State | Meaning |
|-----------|---------|
| **Bright Blue** | Switch is ON |
| **Dim Blue** | Switch is OFF |
| **Blinking White** | Identify command (triggered from app) |
| **White/Red Pattern** | Factory reset binary code display (see [Recovering Your QR Code](#recovering-your-qr-code)) |
| **Solid Green** | Factory reset cancelled - returning to previous state |
| **Solid Red** | Factory reset confirmed - resetting device |

### App Control

Once commissioned, you can:
- Turn the switch ON/OFF from your smart home app
- Include it in scenes and automations
- Control it with voice commands
- Monitor its status remotely

---

## Factory Reset

If you need to remove the device from your home or start over:

### Performing a Factory Reset

1. **Press and hold the button**
2. **Wait 3 seconds** (initial delay - you can release to cancel during this time)
3. **Binary pattern displays** (~19 seconds)
   - The LED will blink white (1) and red (0) showing the firmware configuration
   - This pattern helps you recover your QR code later (see [Recovering Your QR Code](#recovering-your-qr-code))
   - **Important:** Pattern is non-cancellable - it will always complete
4. **After pattern completes**, check the LED:
   - **Solid GREEN (3 seconds)** = Button was released, reset cancelled, LED returns to previous state
   - **Solid RED (3 seconds)** = Button still held, factory reset will proceed
5. If RED, device resets and restarts automatically
6. Device will be ready for commissioning again

**Total time: ~25 seconds** (3s delay + 19s pattern + 3s indicator)

### Cancelling a Factory Reset

You can cancel the factory reset at two points:

- **During initial 3-second delay**: Release the button to cancel immediately
- **After binary pattern completes**: Release the button before the pattern ends
  - LED will show **solid GREEN** for 3 seconds
  - LED returns to previous state (bright blue if ON, dim blue if OFF)
  - No reset performed

### When to Factory Reset

- Moving the device to a different home
- Changing Matter platforms (e.g., from Google to Apple)
- Troubleshooting connection issues
- Before giving the device to someone else

**Security Note on Factory Reset:**

Factory reset **removes** these items:
- Matter fabric credentials and access control
- Thread network credentials
- Commissioning state

Factory reset **does NOT remove**:
- Device attestation certificates (DAC)
- Factory configuration
- Application firmware
- **Secrets stored in flash memory** (not encrypted)

**Important:** Because this device does not use flash encryption, anyone with physical access and basic tools can read secrets from the device memory even after factory reset. If transferring ownership or disposing of the device, consider the flash contents as potentially readable.

---

## Recovering Your QR Code

If you've lost your commissioning QR code, you can recover it by reading the binary pattern displayed during the factory reset sequence.

### How to Read the Binary Pattern

1. **Start the factory reset sequence** (hold button for 3 seconds)
2. **Watch the LED pattern** for the next ~19 seconds:
   - The pattern repeats **5 times**
   - Each pattern shows **4 bits** in **LSB-first order** (bit 0, 1, 2, 3)
   - **WHITE LED** = Binary 1
   - **RED LED** = Binary 0
   - Brief pause between bits, longer pause between repetitions

3. **Decode the pattern**:

   Example: If you see WHITE, RED, RED, RED (repeated 5 times)
   - Bit 0 (rightmost): WHITE = 1
   - Bit 1: RED = 0
   - Bit 2: RED = 0
   - Bit 3 (leftmost): RED = 0
   - Binary: 0b0001 (LSB first: bit 3, bit 2, bit 1, bit 0)
   - Decimal value: 1

4. **Find your QR code**:
   - Go to the project's GitHub releases page
   - Find the release tagged `release-0b0001` (using your decoded value)
   - Download the `pairing_qr.png` image from that release
   - Use this QR code to commission your device

### Binary Pattern Examples

| Pattern (LSB first) | Binary | Decimal | Release Tag |
|---------------------|--------|---------|-------------|
| WHITE, RED, RED, RED | 0b0001 | 1 | `release-0b0001` |
| RED, WHITE, RED, RED | 0b0010 | 2 | `release-0b0010` |
| WHITE, WHITE, RED, RED | 0b0011 | 3 | `release-0b0011` |
| RED, RED, WHITE, RED | 0b0100 | 4 | `release-0b0100` |
| WHITE, RED, WHITE, RED | 0b0101 | 5 | `release-0b0101` |

**Tip:** The pattern repeats 5 times - you have multiple chances to observe it. You can release the button after decoding to cancel the reset (LED will show GREEN).

---

## Troubleshooting

### Device won't connect during commissioning

**Solution:**
1. Make sure your Thread Border Router is set up and working
2. Keep your phone close to the device during setup
3. Ensure Bluetooth is enabled on your phone
4. Try factory resetting the device and commissioning again

### Button press doesn't toggle the switch

**Solution:**
1. The switch state is synced across your Matter fabric
2. If controlled from the app, press the button again to toggle
3. Try factory resetting if the button is completely unresponsive

### LED is dim/not visible

**Solution:**
- Dim blue is the normal OFF state
- Make sure the device has power
- Try toggling to ON state for bright blue LED

### Device is unresponsive

**Solution:**
1. Power cycle the device (disconnect and reconnect power)
2. Check if your Thread Border Router is online
3. Factory reset and re-commission if issues persist

---

## Technical Specifications

| Specification | Details |
|--------------|---------|
| **Device Type** | On/Off Plug-in Unit (Matter 0x010A) |
| **Connectivity** | Thread 1.3 |
| **Commissioning** | Bluetooth LE |
| **Processor** | ESP32-C6 (RISC-V) |
| **Button** | GPIO 9 (active low) |
| **LED** | WS2812 RGB (GPIO 20) |
| **Power** | USB-C 5V |

---

## Support

For issues or questions:
- Check the troubleshooting section above
- Consult your Matter platform's documentation
- Refer to the project repository for technical details

---

**Device Information:**
- Vendor: M5Stack
- Product: M5NanoC6 Switch
- Vendor ID: 0xFFF1
- Product ID: 0x8000

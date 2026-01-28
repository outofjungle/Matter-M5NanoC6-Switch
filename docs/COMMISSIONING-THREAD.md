# Thread Commissioning Guide

> **ℹ️ Notice:** For end users, please see **[USER-GUIDE.md](../USER-GUIDE.md)** which includes commissioning instructions for both WiFi and Thread builds in a user-friendly format. This document contains developer-specific information and command-line usage.

Step-by-step instructions for commissioning the M5NanoC6 Switch over Thread.

## Prerequisites

- **Thread Build**: Device must be flashed with Thread firmware (`make build-thread` or `make build`)
- **Thread Border Router (TBR)**: Required for Thread network connectivity. Options:
  - Apple HomePod mini / HomePod (2nd gen)
  - Apple TV 4K (3rd gen or later)
  - Google Nest Hub (2nd gen)
  - ESP Thread Border Router
  - OpenThread Border Router (OTBR)
- **Matter Controller**: One of:
  - Apple Home app (iOS 16.1+ / iPadOS 16.1+)
  - Google Home app
  - chip-tool (command-line)
- **QR Code**: Generated pairing QR code ([`pairing_qr.png`](../pairing_qr.png))

## What is Thread?

Thread is a low-power mesh networking protocol designed for smart home devices. Unlike WiFi:
- **Mesh topology**: Devices relay messages (self-healing network)
- **Low power**: Designed for battery-operated devices
- **Secure by default**: Encrypted mesh with no single point of failure
- **Requires Thread Border Router**: Bridges Thread network to WiFi/Ethernet

## How Thread Commissioning Works

1. **Device boots** without Thread credentials
2. **BLE advertising starts** (device discoverable)
3. **You use a Matter controller** to commission via BLE
4. **Controller provides Thread credentials** to the device
5. **Device joins Thread mesh** network
6. **Matter fabric established** over Thread
7. **TBR routes traffic** between Thread and WiFi/Internet

## Commissioning Methods

### Option 1: Apple Home (iOS/iPadOS)

**Requirements:**
- iOS 16.1+ or iPadOS 16.1+
- iPhone/iPad on your home WiFi network
- Thread Border Router (HomePod mini, HomePod, or Apple TV 4K)
- Bluetooth enabled

**Steps:**

1. **Ensure Thread Border Router is set up:**
   - HomePod mini / HomePod / Apple TV 4K should be added to Apple Home
   - Check Home app → Home Settings → Thread Network (should show active TBR)

2. **Open Home app** on your iPhone/iPad

3. **Tap "+"** in the top-right corner

4. **Select "Add Accessory"**

5. **Choose commissioning method:**
   - **QR Code** (recommended):
     - Point camera at [`pairing_qr.png`](../pairing_qr.png)
     - Tap the notification when the code is recognized

   - **Manual Code Entry**:
     - Tap "More options..."
     - Look for "M5NanoC6 Switch" in the list
     - Or tap "My Accessory Isn't Shown Here"
     - Enter the 8-digit setup code from your pairing config

6. **Tap "Add to Home"** when prompted

7. **Wait for Thread join:**
   - Apple Home automatically provides Thread credentials
   - Device joins the Thread mesh network
   - Matter fabric is established

8. **Complete setup:**
   - Name your device (e.g., "Bedroom Light")
   - Assign to a room
   - Tap "Done"

**LED Behavior:**
- **Dim blue**: Device ready for commissioning
- **Bright blue flashing**: Commissioning in progress
- **Dim/Bright blue**: Normal operation (OFF/ON)

**Troubleshooting:**
- If "Accessory Not Found", ensure Bluetooth is enabled
- If Thread join fails, verify Thread Border Router is online
- Check Home app → Home Settings → Thread Network for status
- Check serial monitor for connection logs: `make monitor`

---

### Option 2: Google Home

**Requirements:**
- Google Home app (Android/iOS)
- Phone on your home WiFi network
- Thread Border Router (Google Nest Hub 2nd gen or compatible)
- Bluetooth enabled

**Steps:**

1. **Ensure Thread Border Router is set up:**
   - Google Nest Hub (2nd gen) should be set up in Google Home
   - The Hub will automatically act as a Thread Border Router

2. **Open Google Home app**

3. **Tap "+" (Add)** in the top-left

4. **Select "Set up device"** → **"New device"**

5. **Select your home** (or create one)

6. **Choose commissioning method:**
   - **Scan QR code**: Point camera at [`pairing_qr.png`](../pairing_qr.png)
   - **Manual entry**: Enter pairing code when prompted

7. **Grant permissions** when requested:
   - Location (required for Thread setup)
   - Bluetooth (required for commissioning)

8. **Wait for Thread join**:
   - Google Home provides Thread credentials automatically
   - Device joins mesh network

9. **Complete setup:**
   - Name your device
   - Assign to a room
   - Tap "Done"

---

### Option 3: chip-tool (Command Line)

**Requirements:**
- chip-tool installed on Linux/Mac
- Thread Border Router on the same network
- Bluetooth adapter on the computer
- Thread network credentials (from your TBR)

**Installation:**

```bash
# Clone connectedhomeip
git clone https://github.com/project-chip/connectedhomeip.git
cd connectedhomeip

# Build chip-tool
./scripts/examples/gn_build_example.sh examples/chip-tool out/chip-tool
```

**Get Thread Network Credentials:**

You need the Thread Operational Dataset from your Thread Border Router:

**From Apple Home:**
- Home app → Home Settings → Thread Network → Export Thread Credentials
- Or use `ioreg` on macOS to extract dataset

**From ESP Thread Border Router:**
```bash
# On the TBR device console
> dataset active -x
# Copy the hex dataset
```

**Commission Device:**

```bash
# Format: chip-tool pairing ble-thread <node-id> hex:<dataset> <setup-pin> <discriminator>

# Example with your pairing config:
./out/chip-tool/chip-tool pairing ble-thread 1 \
  hex:0e080000000000010000000300001235060004001fffe00208fedcba9876543210 \
  5143243 1568

# Parameters:
#   1              - Node ID (assign any unique ID)
#   hex:<dataset>  - Thread operational dataset in hex
#   5143243        - Setup PIN from CHIPPairingConfig.h
#   1568           - Discriminator from CHIPPairingConfig.h (0x620)
```

**Verify Connection:**

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

## Checking Serial Monitor

Monitor the device during commissioning to see what's happening:

```bash
make monitor
```

**Expected log sequence:**

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

## Post-Commissioning

**After successful commissioning:**

- Device will **automatically rejoin** Thread network on every reboot
- **No re-commissioning needed** unless factory reset
- Control the switch from your Matter controller app
- Button press still toggles the switch locally
- Thread mesh **self-heals** if devices move or fail

**Thread Network Benefits:**

- **Mesh resilience**: Multiple paths to reach device
- **Range extension**: Devices relay messages
- **Low latency**: Direct mesh communication
- **Battery friendly**: Low power consumption (this device is mains-powered)

**LED Pattern on Factory Reset:**

When you factory reset the device (hold button 20+ seconds), the LED displays the firmware config ID in **binary**:

- **White** = Binary 1
- **Red** = Binary 0
- Pattern repeats 5 times, MSB first

This helps you recover the correct QR code if you have multiple devices.

## Common Issues

### "Accessory Not Found"

**Causes:**
- Bluetooth is disabled
- Device not in commissioning mode
- Commissioning window expired (opens for 5 minutes after boot)

**Solutions:**
- Enable Bluetooth on your phone
- Reboot the device to reopen commissioning window
- Check `make monitor` for BLE advertising messages

### Thread Join Fails

**Causes:**
- No Thread Border Router on network
- Thread Border Router is offline
- Thread network credentials missing/incorrect
- Device too far from Thread routers

**Solutions:**
- Verify Thread Border Router is online (check controller app)
- Reboot Thread Border Router
- Move device closer to TBR or other Thread devices during commissioning
- Check `make monitor` for Thread join errors

### Device Stuck in "Detached" State

Check serial monitor:
```bash
make monitor
```

Look for:
```
OPENTHREAD: [N] Mle-----------: Role disabled -> detached
OPENTHREAD: [W] Mle-----------: Failed to attach
```

**Solutions:**
- Ensure Thread Border Router is on the same network
- Verify Thread credentials are correct
- Reboot device and retry commissioning

### Cannot Control Device After Joining

**Causes:**
- Thread Border Router lost connectivity to WiFi/Internet
- IPv6 routing issues
- Firewall blocking Matter traffic

**Solutions:**
- Check Thread Border Router connectivity
- Ensure controller and TBR are on same network
- Verify IPv6 is enabled on your router
- Check controller app for device status

### Re-commissioning After Factory Reset

1. Hold button for 23+ seconds
2. LED shows white/red binary pattern
3. Device resets and enters commissioning mode
4. Follow commissioning steps again

## Checking Thread Network Status

**Apple Home:**
- Home app → Home Settings → Thread Network
- Shows active Thread Border Routers and device count

**Google Home:**
- Google Home app → Settings → Thread
- Shows network status and connected devices

**chip-tool:**
```bash
# Read Thread diagnostic info
./out/chip-tool/chip-tool threadnetworkdiagnostics read routing-role 1 1
./out/chip-tool/chip-tool threadnetworkdiagnostics read network-name 1 1
```

**Serial Monitor:**
```bash
make monitor

# Look for Thread status logs
I (XXXX) OPENTHREAD: [N] Mle-----------: Role child
I (XXXX) OPENTHREAD: [N] Mle-----------: RLOC16 0xXX
```

## Finding Your Pairing Info

**View in Serial Monitor:**
```bash
make monitor
```

Look for:
```
I (1695) app_main: === Commissioning Info ===
I (1695) app_main: Discriminator: 1568 (0x620)
I (1705) app_main: Passcode: 5143243
```

**View in Source Code:**

```bash
cat main/include/CHIPPairingConfig.h
```

**View QR Code:**

![Pairing QR Code](../pairing_qr.png)

Or from terminal:
```bash
open pairing_qr.png  # macOS
xdg-open pairing_qr.png  # Linux
```

## Thread Network Topology

After commissioning, your Thread network looks like:

```
Internet
    │
    ├─ WiFi Router
    │      │
    │      ├─ Thread Border Router (HomePod/Nest Hub)
    │      │      │
    │      │      ├─ Thread Router Device 1
    │      │      │      └─ M5NanoC6 Switch (Child)
    │      │      │
    │      │      └─ Thread Router Device 2
    │      │             └─ Other Thread Devices (Children)
    │      │
    │      └─ Matter Controller (iPhone/Android)
```

- **TBR** bridges Thread mesh to WiFi
- **Routers** extend mesh and relay messages
- **Children** (like your switch) communicate through routers
- **Controller** sends commands via TBR to Thread devices

## Next Steps

- **Control from app**: Use your Matter controller to turn the switch on/off
- **Local control**: Press the button to toggle
- **Monitor status**: Run `make monitor` to see Thread state changes
- **Add more Thread devices**: They'll join the same mesh network
- **Update firmware**: Build and flash updates anytime
- **Factory reset**: Hold button 20+ seconds to reset and re-commission

## Security Notes

⚠️ **This device uses test credentials for development only.**

For production deployment:
- Use official Vendor ID from CSA
- Enable secure boot and flash encryption
- Use production Device Attestation Certificates
- Disable debug interfaces

See [PRODUCTION-SECURITY-CHECKLIST.md](PRODUCTION-SECURITY-CHECKLIST.md) for details.

## Learn More About Thread

- [Thread Group](https://www.threadgroup.org/)
- [OpenThread Documentation](https://openthread.io/)
- [Matter over Thread Architecture](https://www.threadgroup.org/What-is-Thread/Matter)
- [ESP Thread Border Router](https://github.com/espressif/esp-thread-br)

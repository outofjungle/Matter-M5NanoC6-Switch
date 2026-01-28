# WiFi Commissioning Guide

Step-by-step instructions for commissioning the M5NanoC6 Switch over WiFi.

## Prerequisites

- **WiFi Build**: Device must be flashed with WiFi firmware (`make build-wifi`)
- **Matter Controller**: One of:
  - Apple Home app (iOS 16.1+ / iPadOS 16.1+)
  - Google Home app
  - chip-tool (command-line)
- **Home WiFi Network**: 2.4GHz WiFi network credentials
- **QR Code**: Generated pairing QR code (`pairing_qr.png`)

## Important: WiFi vs Manual Connection

**Do NOT manually connect to the device's WiFi access point.** Matter commissioning uses Bluetooth LE (BLE) for secure setup, and the WiFi AP is managed automatically by the commissioning process.

## How WiFi Commissioning Works

1. **Device boots** without WiFi credentials
2. **AP mode activates** (`ESP_XXXXXX` network appears)
3. **BLE advertising starts** (device discoverable)
4. **You use a Matter controller** to commission via BLE
5. **Controller provides WiFi credentials** to the device
6. **Device joins your home WiFi** network
7. **Matter fabric established** over WiFi

The AP you see in your WiFi list is used during this process, but you don't manually connect to it.

## Commissioning Methods

### Option 1: Apple Home (iOS/iPadOS)

**Requirements:**
- iOS 16.1+ or iPadOS 16.1+
- iPhone/iPad on your home WiFi network
- Bluetooth enabled

**Steps:**

1. **Open Home app** on your iPhone/iPad

2. **Tap "+"** in the top-right corner

3. **Select "Add Accessory"**

4. **Choose commissioning method:**
   - **QR Code** (recommended):
     - Point camera at `pairing_qr.png`
     - Tap the notification when the code is recognized

   - **Manual Code Entry**:
     - Tap "More options..."
     - Look for "M5NanoC6 Switch" in the list
     - Or tap "My Accessory Isn't Shown Here"
     - Enter the 8-digit setup code from your pairing config

5. **Tap "Add to Home"** when prompted

6. **Select WiFi Network:**
   - If prompted, select your home WiFi network
   - Enter the WiFi password
   - The controller sends credentials to the device via BLE

7. **Wait for connection:**
   - Device will disconnect from its AP
   - Device joins your home WiFi
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
- If WiFi connection fails, verify 2.4GHz network and password
- Check serial monitor for connection logs: `make monitor`

---

### Option 2: Google Home

**Requirements:**
- Google Home app (Android/iOS)
- Phone on your home WiFi network
- Bluetooth enabled

**Steps:**

1. **Open Google Home app**

2. **Tap "+" (Add)** in the top-left

3. **Select "Set up device"** → **"New device"**

4. **Select your home** (or create one)

5. **Choose commissioning method:**
   - **Scan QR code**: Point camera at `pairing_qr.png`
   - **Manual entry**: Enter pairing code when prompted

6. **Grant permissions** when requested:
   - Location (required for WiFi setup)
   - Bluetooth (required for commissioning)

7. **Select WiFi network:**
   - Choose your home WiFi network
   - Enter password

8. **Wait for connection**

9. **Complete setup:**
   - Name your device
   - Assign to a room
   - Tap "Done"

---

### Option 3: chip-tool (Command Line)

**Requirements:**
- chip-tool installed on Linux/Mac
- Device in commissioning mode
- Bluetooth adapter on the computer

**Installation:**

```bash
# Clone connectedhomeip
git clone https://github.com/project-chip/connectedhomeip.git
cd connectedhomeip

# Build chip-tool
./scripts/examples/gn_build_example.sh examples/chip-tool out/chip-tool
```

**Commission Device:**

```bash
# Format: chip-tool pairing ble-wifi <node-id> <ssid> <password> <setup-pin> <discriminator>

# Example with your pairing config (check CHIPPairingConfig.h for actual values):
./out/chip-tool/chip-tool pairing ble-wifi 1 "YourSSID" "YourPassword" 5143243 1568

# Parameters:
#   1           - Node ID (assign any unique ID)
#   YourSSID    - Your WiFi network name
#   YourPassword - Your WiFi password
#   5143243     - Setup PIN from CHIPPairingConfig.h
#   1568        - Discriminator from CHIPPairingConfig.h (0x620)
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

## Post-Commissioning

**After successful commissioning:**

- Device will **automatically reconnect** to WiFi on every reboot
- **No re-commissioning needed** unless factory reset
- Control the switch from your Matter controller app
- Button press still toggles the switch locally

**LED Pattern on Factory Reset:**

When you factory reset the device (hold button 20+ seconds), the LED displays the firmware config ID in **binary**:

- **Blue** = Binary 1
- **Purple** = Binary 0
- Pattern repeats 5 times, MSB first

This helps you recover the correct QR code if you have multiple devices.

## Common Issues

### "Unable to join network ESP_XXXXXX"

This is **expected and normal**. Don't manually connect to the AP from WiFi settings. Use a Matter controller app instead.

### "Accessory Not Found"

**Causes:**
- Bluetooth is disabled
- Device not in commissioning mode
- Commissioning window expired (opens for 5 minutes after boot)

**Solutions:**
- Enable Bluetooth on your phone
- Reboot the device to reopen commissioning window
- Check `make monitor` for BLE advertising messages

### WiFi Connection Fails

**Causes:**
- 5GHz WiFi network (ESP32-C6 only supports 2.4GHz)
- Wrong WiFi password
- WiFi network out of range
- Special characters in SSID/password

**Solutions:**
- Use 2.4GHz WiFi network only
- Verify password is correct
- Move device closer to WiFi router during commissioning
- Avoid special characters in SSID if possible

### Device Keeps Resetting

Check serial monitor for errors:
```bash
make monitor
```

Look for WiFi connection errors or brownout detection.

### Re-commissioning After Factory Reset

1. Hold button for 23+ seconds
2. LED shows blue/purple binary pattern
3. Device resets and enters commissioning mode
4. Follow commissioning steps again with same or different WiFi network

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

```bash
open pairing_qr.png  # macOS
xdg-open pairing_qr.png  # Linux
```

## Next Steps

- **Control from app**: Use your Matter controller to turn the switch on/off
- **Local control**: Press the button to toggle
- **Monitor status**: Run `make monitor` to see state changes
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

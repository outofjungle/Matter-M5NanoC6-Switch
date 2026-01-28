# M5NanoC6 Switch - User Guide

## What is this device?

The M5NanoC6 Switch is a Matter-enabled smart switch that works with Apple Home, Google Home, Amazon Alexa, and other Matter-compatible platforms. It connects to your home network via **WiFi** or **Thread** networking and can be controlled from your smartphone or voice assistant.

**Features:**
- Toggle ON/OFF with physical button
- LED indicator shows current state (bright blue = ON, dim blue = OFF)
- WiFi (2.4GHz) or Thread network support
- Works with all Matter-compatible ecosystems

> **ℹ️ Development Device Notice**
>
> This device is designed for personal use, development, and testing. It uses test credentials and is not intended for production deployment or controlling critical systems.

---

## Getting Started

### Which Build Do You Have?

Your M5NanoC6 Switch can be built with either **WiFi** or **Thread** networking. To find out which build you have:

**Check the LED pattern during factory reset:**
- Hold the button for 1 second, then observe the LED pattern during the next ~19 seconds
- **White/Red pattern** = Thread build
- **Blue/Purple pattern** = WiFi build
- You can release the button after seeing the pattern to cancel the reset

### What You'll Need

**For Thread builds:**
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
   - iPhone with iOS 16.1+ (Home app)
   - Android with Google Home app
   - Amazon Alexa app

**For WiFi builds:**
1. **M5NanoC6 Switch** (the device you received)
2. **2.4GHz WiFi Network** (5GHz not supported)
   - Network name (SSID) and password
   - WPA2 or WPA3 security recommended
3. **Smartphone** with Matter-compatible app:
   - iPhone with iOS 16.1+ (Home app)
   - Android with Google Home app
   - Amazon Alexa app

### First Time Setup

When you first power on the device:
1. The LED will light up (dim blue)
2. The device enters **commissioning mode** automatically
3. It remains discoverable via Bluetooth LE for pairing

---

## Commissioning (Adding to Your Home)

### How Matter Commissioning Works

**Thread builds:**
1. Device boots and starts BLE advertising
2. You use a Matter controller (Apple Home, Google Home, etc.) via Bluetooth
3. Controller provides Thread network credentials to the device
4. Device joins your Thread mesh network
5. Thread Border Router routes traffic between Thread and WiFi/Internet

**WiFi builds:**
1. Device boots and starts BLE advertising
2. You use a Matter controller via Bluetooth
3. Controller provides WiFi credentials to the device
4. Device joins your WiFi network directly
5. Matter communication happens over WiFi

**Important:** Do NOT manually connect to any `ESP_XXXXXX` WiFi network. Matter commissioning uses Bluetooth LE for secure setup.

### Quick Start with QR Code

1. Open your Matter-compatible app:
   - **Apple Home**: Tap `+` → "Add Accessory"
   - **Google Home**: Tap `+` → "Set up device" → "New device"
   - **Amazon Alexa**: Tap "Devices" → `+` → "Add Device"

2. Scan this QR code with your phone's camera:

<p align="center">
  <img src="pairing_qr.png" alt="Commissioning QR Code" width="300">
</p>

3. Follow the on-screen instructions in your app
4. **WiFi build:** Enter your WiFi network password when prompted
5. **Thread build:** The controller automatically provides Thread credentials
6. Wait for the device to connect (LED will flash during commissioning)
7. Once added, you can control the switch from your app

**Note:** If QR scanning doesn't work, look for "M5NanoC6 Switch" in your app's device list or use manual code entry.

### Detailed Commissioning Steps

Choose the guide for your platform:

#### Apple Home (iOS/iPadOS 16.1+)

**Requirements:**
- iPhone or iPad on your WiFi network
- Bluetooth enabled on your phone
- **Thread builds only:** HomePod mini, HomePod (2nd gen), or Apple TV 4K set up in Apple Home

**Steps:**

1. **For Thread builds:** Verify your Thread Border Router is set up
   - Open Home app → Home Settings → Thread Network
   - You should see an active Thread Border Router listed

2. **Open Home app** on your iPhone/iPad

3. **Tap "+"** in the top-right corner

4. **Select "Add Accessory"**

5. **Choose commissioning method:**
   - **QR Code** (recommended):
     - Point camera at the QR code above or at [`pairing_qr.png`](../pairing_qr.png)
     - Tap the notification when recognized
   - **Manual Entry**:
     - Tap "More options..."
     - Look for "M5NanoC6 Switch" in the list
     - Or tap "My Accessory Isn't Shown Here"
     - Enter the 8-digit setup code from your pairing configuration

6. **Tap "Add to Home"** when prompted

7. **For WiFi builds:** Enter network credentials
   - Select your 2.4GHz WiFi network when prompted
   - Enter your WiFi password
   - The controller sends credentials via Bluetooth to the device

8. **Wait for connection**
   - LED will flash bright blue during commissioning
   - Device joins your network (WiFi or Thread)
   - This may take 30-60 seconds

9. **Complete setup:**
   - Name your device (e.g., "Bedroom Light")
   - Assign to a room
   - Tap "Done"

**LED Behavior:**
- **Dim blue**: Ready for commissioning
- **Bright blue flashing**: Commissioning in progress
- **Dim/Bright blue**: Normal operation (OFF/ON)

**Troubleshooting:**
- **"Accessory Not Found"**: Ensure Bluetooth is enabled on your phone
- **WiFi connection fails**: Verify you're using a 2.4GHz network (not 5GHz) and correct password
- **Thread join fails**: Check that your Thread Border Router is online in Home Settings
- **Device not responding**: Power cycle the device and try again

#### Google Home

**Requirements:**
- Google Home app (Android or iOS)
- Phone on your WiFi network
- Bluetooth enabled
- **Thread builds only:** Google Nest Hub (2nd gen) or compatible Thread Border Router

**Steps:**

1. **For Thread builds:** Ensure your Thread Border Router is set up
   - Google Nest Hub (2nd gen) automatically acts as a Thread Border Router

2. **Open Google Home app**

3. **Tap "+" (Add)** in the top-left corner

4. **Select "Set up device"** → **"New device"**

5. **Select your home** (or create one if this is your first device)

6. **Choose commissioning method:**
   - **Scan QR code**: Point camera at the QR code above or at [`pairing_qr.png`](../pairing_qr.png)
   - **Manual entry**: Enter the pairing code when prompted

7. **Grant permissions** when requested:
   - Location (required for WiFi/Thread setup)
   - Bluetooth (required for commissioning)

8. **For WiFi builds:** Select WiFi network
   - Choose your 2.4GHz WiFi network from the list
   - Enter your WiFi password

9. **Wait for connection**
   - Device will join your network
   - This may take 30-60 seconds

10. **Complete setup:**
    - Name your device
    - Assign to a room
    - Tap "Done"

**Troubleshooting:**
- **Device not found**: Enable Bluetooth and Location on your phone
- **WiFi connection fails**: Use 2.4GHz network only, verify password
- **Thread join fails**: Ensure Thread Border Router is online
- **Setup timeout**: Restart device and try commissioning again

#### Amazon Alexa

**Steps:**

1. **Open Alexa app**

2. **Tap "Devices"** at the bottom

3. **Tap "+"** in the top-right corner

4. **Select "Add Device"**

5. **Choose "Matter"** from the device type list

6. **Scan QR code** or enter setup code manually

7. **For WiFi builds:** Select your WiFi network and enter password

8. **Follow on-screen instructions** to complete setup

9. **Name your device** and assign to a group/room

---

## Using Your Switch

### Physical Button

**Press once**: Toggle between ON and OFF
- The LED will change brightness to reflect the new state

**Hold for ~23 seconds**: Factory reset (see below)

### LED Indicator

| LED State | Meaning |
|-----------|---------|
| **Bright Blue** | Switch is ON |
| **Dim Blue** | Switch is OFF |
| **Bright Blue Flashing** | Commissioning in progress |
| **Identify Pattern (2x)** | Identify command triggered from app - displays firmware config ID |
| **Binary Pattern** | Factory reset sequence - displays firmware config ID (see [Recovering Your QR Code](#recovering-your-qr-code)) |
| **Solid Green** | Factory reset cancelled - returning to previous state |
| **Solid Red** | Factory reset confirmed - resetting device |

**Binary patterns differ by build type:**
- **Thread build**: White (1) / Red (0)
- **WiFi build**: Purple (1) / Blue (0)

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
2. **Wait 1 second** (initial delay - you can release to cancel during this time)
3. **Binary pattern displays** (~19 seconds)
   - The LED will blink white (1) and red (0) showing the firmware configuration
   - This pattern helps you recover your QR code later (see [Recovering Your QR Code](#recovering-your-qr-code))
   - **Important:** Pattern is non-cancellable - it will always complete
4. **After pattern completes**, check the LED:
   - **Solid GREEN (3 seconds)** = Button was released, reset cancelled, LED returns to previous state
   - **Solid RED (3 seconds)** = Button still held, factory reset will proceed
5. If RED, device resets and restarts automatically
6. Device will be ready for commissioning again

**Total time: ~23 seconds** (1s delay + 19s pattern + 3s indicator)

### Cancelling a Factory Reset

You can cancel the factory reset at two points:

- **During initial 1-second delay**: Release the button to cancel immediately
- **After binary pattern completes**: Release the button before the pattern ends
  - LED will show **solid GREEN** for 3 seconds
  - LED returns to previous state (bright blue if ON, dim blue if OFF)
  - No reset performed

### When to Factory Reset

- Moving the device to a different home
- Changing Matter platforms (e.g., from Google to Apple)
- Troubleshooting connection issues
- Before giving the device to someone else

**What Gets Reset:**
- Matter fabric credentials
- WiFi or Thread network credentials
- Commissioning state

After factory reset, you'll need to commission the device again using the QR code.

---

## Recovering Your QR Code

If you've lost your commissioning QR code, you can recover it by reading the binary pattern displayed during the factory reset sequence.

### How to Read the Binary Pattern

1. **Start the factory reset sequence** (hold button for 1 second)
2. **Watch the LED pattern** for the next ~19 seconds:
   - The pattern repeats **5 times**
   - Each pattern shows **4 bits** in **MSB-first order** (bit 3, 2, 1, 0)
   - Brief pause between bits, longer pause between repetitions

3. **Note the colors** (depends on your build):
   - **Thread build:**
     - **WHITE LED** = Binary 1
     - **RED LED** = Binary 0
   - **WiFi build:**
     - **PURPLE LED** = Binary 1
     - **BLUE LED** = Binary 0

4. **Decode the pattern**:

   **Example for Thread build:** WHITE, RED, RED, WHITE (repeated 5 times)
   - Bit 3 (leftmost): WHITE = 1
   - Bit 2: RED = 0
   - Bit 1: RED = 0
   - Bit 0 (rightmost): WHITE = 1
   - Binary: 0b1001 (MSB first: bit 3, bit 2, bit 1, bit 0)
   - Decimal value: 9

   **Example for WiFi build:** PURPLE, BLUE, BLUE, PURPLE (same as above)
   - Bit 3 (leftmost): PURPLE = 1
   - Bit 2: BLUE = 0
   - Bit 1: BLUE = 0
   - Bit 0 (rightmost): PURPLE = 1
   - Binary: 0b1001
   - Decimal value: 9

5. **Find your QR code**:
   - Go to the project's GitHub releases page
   - Find the release tagged `release-0b1001` (using your decoded value)
   - Download the `pairing_qr.png` image from that release
   - Use this QR code to commission your device

### Binary Pattern Examples

**Thread build (White/Red):**

| Pattern (MSB first) | Binary | Decimal | Release Tag |
|---------------------|--------|---------|-------------|
| RED, RED, RED, WHITE | 0b0001 | 1 | `release-0b0001` |
| RED, RED, WHITE, RED | 0b0010 | 2 | `release-0b0010` |
| RED, RED, WHITE, WHITE | 0b0011 | 3 | `release-0b0011` |
| RED, WHITE, RED, RED | 0b0100 | 4 | `release-0b0100` |
| WHITE, RED, RED, WHITE | 0b1001 | 9 | `release-0b1001` |

**WiFi build (Purple/Blue):**

| Pattern (MSB first) | Binary | Decimal | Release Tag |
|---------------------|--------|---------|-------------|
| BLUE, BLUE, BLUE, PURPLE | 0b0001 | 1 | `release-0b0001` |
| BLUE, BLUE, PURPLE, BLUE | 0b0010 | 2 | `release-0b0010` |
| BLUE, BLUE, PURPLE, PURPLE | 0b0011 | 3 | `release-0b0011` |
| BLUE, PURPLE, BLUE, BLUE | 0b0100 | 4 | `release-0b0100` |
| PURPLE, BLUE, BLUE, PURPLE | 0b1001 | 9 | `release-0b1001` |

**Tip:** The pattern repeats 5 times - you have multiple chances to observe it. You can release the button after decoding to cancel the reset (LED will show GREEN).

---

## Troubleshooting

### Device won't connect during commissioning

**Thread builds:**
1. Verify your Thread Border Router is online and functioning
2. Check in your controller app (Apple Home → Home Settings → Thread Network or Google Home → Settings → Thread)
3. Move device closer to the Thread Border Router during commissioning
4. Ensure Bluetooth is enabled on your phone

**WiFi builds:**
1. Verify you're using a **2.4GHz WiFi network** (ESP32-C6 does not support 5GHz)
2. Ensure WiFi password is correct
3. Move device closer to your WiFi router during commissioning
4. Check that your WiFi network is not hidden or using special characters in SSID

**Both:**
1. Keep your phone close to the device during setup (Bluetooth range)
2. Ensure device is powered on and LED is lit
3. Try factory resetting the device and commissioning again
4. Restart your phone and try again

### "Unable to join network ESP_XXXXXX" (WiFi builds)

This is **expected and normal** for WiFi builds. Do NOT manually connect to this access point from your phone's WiFi settings.

Matter commissioning uses **Bluetooth LE** for setup, not the WiFi AP. The AP may appear during commissioning but will be managed automatically.

### "Accessory Not Found" or device not discoverable

**Causes:**
- Bluetooth is disabled on your phone
- Device not in commissioning mode
- Commissioning window expired (device has been running for more than 5 minutes)

**Solutions:**
- Enable Bluetooth on your phone
- Power cycle the device to reopen the commissioning window
- Ensure Location permissions are granted (required on Android)
- Try moving your phone closer to the device

### WiFi Connection Fails (WiFi builds only)

**Common causes:**
- Using 5GHz WiFi network (only 2.4GHz supported)
- Incorrect WiFi password
- WiFi network out of range
- Special characters in SSID or password causing issues
- WiFi network using enterprise security (WPA-Enterprise not supported)

**Solutions:**
- Verify your network is 2.4GHz (check router settings)
- Double-check your WiFi password
- Move device closer to WiFi router during commissioning
- Try a simpler WiFi network name without special characters if possible
- Use WPA2-Personal or WPA3-Personal security (not Enterprise)

### Thread Join Fails (Thread builds only)

**Common causes:**
- No Thread Border Router on network
- Thread Border Router is offline
- Device too far from Thread routers during commissioning

**Solutions:**
- Verify Thread Border Router is online:
  - **Apple Home**: Home app → Home Settings → Thread Network
  - **Google Home**: Settings → Thread
- Restart your Thread Border Router
- Move device closer to Thread Border Router or other Thread devices
- Ensure Thread Border Router and controller are on same WiFi network

### Device is unresponsive after commissioning

**Thread builds:**
1. Check if Thread Border Router is online
2. Verify Thread network is functioning (check other Thread devices)
3. Power cycle the device
4. Factory reset and re-commission if issues persist

**WiFi builds:**
1. Check if device is connected to WiFi (may have lost connection)
2. Verify WiFi router is functioning
3. Power cycle the device
4. Factory reset and re-commission if issues persist

### Button press doesn't toggle the switch

**Solution:**
1. The switch state is synced across your Matter fabric
2. If controlled from the app, the button should still work - try pressing again
3. Power cycle the device
4. Factory reset if the button is completely unresponsive

### LED is dim or not visible

**Solution:**
- Dim blue is the normal OFF state
- Make sure the device has power (USB-C connected)
- Try toggling to ON state for bright blue LED
- If LED is completely off, check power supply and USB cable

### Device keeps resetting or crashes

**Solutions:**
1. Check power supply - ensure USB cable and power adapter provide stable 5V
2. Try a different USB cable or power adapter
3. Factory reset may help clear corrupted configuration

---

## Technical Specifications

| Specification | Details |
|--------------|---------|
| **Device Type** | On/Off Plug-in Unit (Matter 0x010A) |
| **Connectivity** | WiFi 2.4GHz or Thread 1.3 (depends on build) |
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
- Vendor: 0x76656E Labs
- Product: M5NanoC6 Switch
- Vendor ID: 0xFFF1
- Product ID: 0x8000

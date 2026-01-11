# M5NanoC6 Switch - User Guide

## What is this device?

The M5NanoC6 Switch is a Matter-enabled smart switch that works with Apple Home, Google Home, Amazon Alexa, and other Matter-compatible platforms. It connects to your home network via **Thread** (a low-power mesh networking protocol) and can be controlled from your smartphone or voice assistant.

**Features:**
- Toggle ON/OFF with physical button
- LED indicator shows current state (bright blue = ON, dim blue = OFF)
- Thread network support (requires a Thread Border Router)
- Works with all Matter-compatible ecosystems

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
| **Blinking Red** (increasing speed) | Factory reset countdown |
| **Fast Red Blink** | Factory reset in progress |

### App Control

Once commissioned, you can:
- Turn the switch ON/OFF from your smart home app
- Include it in scenes and automations
- Control it with voice commands
- Monitor its status remotely

---

## Factory Reset

If you need to remove the device from your home or start over:

1. **Press and hold the button for 20 seconds**
2. The LED will start blinking red
3. The blink rate will increase as you continue holding
4. After 20 seconds, the LED will blink rapidly
5. Release the button
6. The device will reset and restart
7. It will be ready for commissioning again

**When to factory reset:**
- Moving the device to a different home
- Changing Matter platforms (e.g., from Google to Apple)
- Troubleshooting connection issues
- Before giving the device to someone else

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

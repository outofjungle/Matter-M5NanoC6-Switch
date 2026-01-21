#!/usr/bin/env python3
"""
Generate Matter pairing configuration for CHIPProjectConfig.h

This script generates the SPAKE2+ verifier and commissioning codes (QR and manual)
for a given discriminator and passcode combination.

Usage:
    python3 scripts/generate_pairing_config.py --discriminator 3840 --passcode 20202021

The output can be pasted into main/include/CHIPProjectConfig.h
"""

import argparse
import base64
import hashlib
import os
import re
import struct
import sys

try:
    import qrcode
    from PIL import Image, ImageDraw, ImageFont
    HAS_QRCODE = True
except ImportError:
    HAS_QRCODE = False

# Add CHIP SDK paths - try multiple locations
POSSIBLE_ESP_MATTER_PATHS = [
    os.environ.get('ESP_MATTER_PATH', ''),
    os.path.expanduser('~/esp/esp-matter'),
    os.path.expanduser('~/Workspace/ESP/esp-matter'),
]

SETUP_PAYLOAD_PATH = None
for esp_matter_path in POSSIBLE_ESP_MATTER_PATHS:
    if not esp_matter_path:
        continue
    path = os.path.join(esp_matter_path, 'connectedhomeip/connectedhomeip/src/setup_payload/python')
    if os.path.exists(path):
        SETUP_PAYLOAD_PATH = path
        sys.path.insert(0, path)
        break

try:
    from ecdsa.curves import NIST256p
except ImportError:
    print("Error: ecdsa library not found. Install with: pip install ecdsa")
    sys.exit(1)

# Invalid passcodes per Matter spec section 5.1.7.1
INVALID_PASSCODES = [
    00000000, 11111111, 22222222, 33333333, 44444444,
    55555555, 66666666, 77777777, 88888888, 99999999,
    12345678, 87654321,
]

# Default SPAKE2+ parameters (matching CHIP defaults)
DEFAULT_SALT = "U1BBS0UyUCBLZXkgU2FsdA=="  # "SPAKE2P Key Salt"
DEFAULT_ITERATION_COUNT = 1000

# Length of w0s and w1s elements
WS_LENGTH = NIST256p.baselen + 8


def generate_verifier(passcode: int, salt: bytes, iterations: int) -> str:
    """Generate SPAKE2+ verifier from passcode, salt, and iteration count."""
    ws = hashlib.pbkdf2_hmac('sha256', struct.pack('<I', passcode), salt, iterations, WS_LENGTH * 2)
    w0 = int.from_bytes(ws[:WS_LENGTH], byteorder='big') % NIST256p.order
    w1 = int.from_bytes(ws[WS_LENGTH:], byteorder='big') % NIST256p.order
    L = NIST256p.generator * w1

    verifier_bytes = w0.to_bytes(NIST256p.baselen, byteorder='big') + L.to_bytes('uncompressed')
    return base64.b64encode(verifier_bytes).decode('ascii')


def generate_qrcode_manual(discriminator: int, passcode: int, vendor_id: int = 0xFFF1, product_id: int = 0x8000, discovery: int = 2):
    """Generate QR code and manual pairing code."""
    try:
        from SetupPayload import SetupPayload, CommissioningFlow
        # Discovery capabilities bitmask:
        #   Bit 0 (1): SoftAP
        #   Bit 1 (2): BLE
        #   Bit 2 (4): OnNetwork
        # For Thread devices commissioning via BLE, use 2 (BLE only)
        payload = SetupPayload(discriminator, passcode, rendezvous=discovery,
                               flow=CommissioningFlow.Standard, vid=vendor_id, pid=product_id)
        return payload.generate_qrcode(), payload.generate_manualcode()
    except ImportError:
        # Fallback - use hardcoded default for common test values
        if discriminator == 0xF00 and passcode == 20202021:
            return "MT:Y.K9042C00KA0648G00", "34970112332"
        else:
            return "<Install dependencies: pip install bitarray construct stdnum click>", "<N/A>"


def read_current_config_id(filepath):
    """Read current FIRMWARE_CONFIG_ID from header file."""
    if not os.path.exists(filepath):
        return 0
    try:
        with open(filepath, 'r') as f:
            content = f.read()
        match = re.search(r'#define\s+FIRMWARE_CONFIG_ID\s+(\d+)', content)
        if match:
            return int(match.group(1))
    except Exception:
        pass
    return 0


def confirm_changes(current_id, new_id, discriminator, passcode):
    """Ask user to confirm all changes."""
    print("\n" + "=" * 60)
    print("Pairing Configuration Changes")
    print("=" * 60)
    print(f"  Discriminator: 0x{discriminator:03X} ({discriminator})")
    print(f"  Passcode:      {passcode}")
    print(f"  Config ID:     {current_id} -> {new_id} (binary: {new_id:04b})")
    print()
    print("Config ID is displayed on LED during factory reset:")
    print("  White = 1, Red = 0, LSB first, repeated 5 times")
    print()

    while True:
        response = input("Proceed with these changes? [y/N]: ").strip().lower()
        if response in ('y', 'yes'):
            return True
        elif response in ('n', 'no', ''):
            return False
        print("Please enter 'y' or 'n'")


def main():
    parser = argparse.ArgumentParser(
        description='Generate Matter pairing configuration for CHIPProjectConfig.h',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Generate config with custom passcode
  python3 %(prog)s -d 3840 -p 12341234

  # Generate with custom vendor/product IDs
  python3 %(prog)s -d 3840 -p 12341234 --vendor-id 0x1234 --product-id 0x5678

  # Write directly to config file
  python3 %(prog)s -d 3840 -p 12341234 --output main/include/CHIPPairingConfig.h
        """
    )
    parser.add_argument('-d', '--discriminator', type=lambda x: int(x, 0), required=True,
                        help='Discriminator (0-4095 or 0x000-0xFFF)')
    parser.add_argument('-p', '--passcode', type=int, required=True,
                        help='Passcode/PIN code (1-99999999)')
    parser.add_argument('--vendor-id', type=lambda x: int(x, 0), default=0xFFF1,
                        help='Vendor ID (default: 0xFFF1)')
    parser.add_argument('--product-id', type=lambda x: int(x, 0), default=0x8000,
                        help='Product ID (default: 0x8000)')
    parser.add_argument('--salt', type=str, default=DEFAULT_SALT,
                        help=f'SPAKE2+ salt in base64 (default: {DEFAULT_SALT})')
    parser.add_argument('--iterations', type=int, default=DEFAULT_ITERATION_COUNT,
                        help=f'SPAKE2+ iteration count (default: {DEFAULT_ITERATION_COUNT})')
    parser.add_argument('-o', '--output', type=str, default=None,
                        help='Output file path (writes C header instead of printing)')
    parser.add_argument('--qr-image', type=str, default=None,
                        help='Generate QR code image file (PNG)')
    parser.add_argument('--discovery', type=int, default=2,
                        help='Discovery capabilities bitmask: 1=SoftAP, 2=BLE, 4=OnNetwork (default: 2 for BLE)')
    parser.add_argument('--config-id', type=int, default=None,
                        help='Firmware config ID (0-15). Auto-increments if not specified.')
    parser.add_argument('--no-confirm', action='store_true',
                        help='Skip confirmation prompt for changes')

    args = parser.parse_args()

    # Validate discriminator
    if not 0 <= args.discriminator <= 0xFFF:
        print(f"Error: Discriminator must be 0-4095 (0x000-0xFFF), got {args.discriminator}")
        sys.exit(1)

    # Validate passcode
    if not 1 <= args.passcode <= 99999999:
        print(f"Error: Passcode must be 1-99999999, got {args.passcode}")
        sys.exit(1)

    if args.passcode in INVALID_PASSCODES:
        print(f"Error: Passcode {args.passcode} is in the list of invalid passcodes")
        sys.exit(1)

    # Read current config ID
    current_config_id = read_current_config_id(args.output) if args.output else 0

    # Determine new config ID
    if args.config_id is not None:
        if not 0 <= args.config_id <= 15:
            print(f"Error: Config ID must be 0-15, got {args.config_id}")
            sys.exit(1)
        new_config_id = args.config_id
    else:
        new_config_id = (current_config_id + 1) % 16

    # Confirm changes (always required unless --no-confirm)
    if args.output and not args.no_confirm:
        # Check if we can prompt for confirmation
        if not sys.stdin.isatty():
            print("=" * 60)
            print("Pairing Configuration Changes")
            print("=" * 60)
            print(f"  Discriminator: 0x{args.discriminator:03X} ({args.discriminator})")
            print(f"  Passcode:      {args.passcode}")
            print(f"  Config ID:     {current_config_id} -> {new_config_id} (binary: {new_config_id:04b})")
            print()
            print("ERROR: Cannot prompt for confirmation in non-interactive mode.")
            print()
            print("To generate pairing config, run directly (not via Docker):")
            print(f"  python3 scripts/generate_pairing_config.py -d {args.discriminator} -p {args.passcode} -o {args.output}")
            print()
            print("Or use --no-confirm to skip confirmation (use with caution):")
            print(f"  make generate-pairing PAIRING_EXTRA_ARGS='--no-confirm'")
            sys.exit(1)

        if not confirm_changes(current_config_id, new_config_id, args.discriminator, args.passcode):
            print("Aborted.")
            sys.exit(0)

    # Decode salt
    try:
        salt_bytes = base64.b64decode(args.salt)
        if not 16 <= len(salt_bytes) <= 32:
            print("Error: Salt must be 16-32 bytes when decoded")
            sys.exit(1)
    except Exception as e:
        print(f"Error: Invalid salt base64 encoding: {e}")
        sys.exit(1)

    # Generate verifier
    verifier = generate_verifier(args.passcode, salt_bytes, args.iterations)

    # Generate QR code and manual code
    qr_code, manual_code = generate_qrcode_manual(args.discriminator, args.passcode,
                                                   args.vendor_id, args.product_id,
                                                   args.discovery)

    # Split verifier for readability (C string continuation)
    if len(verifier) > 80:
        mid = len(verifier) // 2
        verifier_define = (
            f'#define CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER \\\n'
            f'    "{verifier[:mid]}" \\\n'
            f'    "{verifier[mid:]}"'
        )
    else:
        verifier_define = f'#define CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_VERIFIER "{verifier}"'

    # Generate header file content
    header_content = f'''/*
 * Auto-generated Matter Pairing Configuration
 * Generated by: scripts/generate_pairing_config.py
 *
 * QR Code:     {qr_code}
 * Manual Code: {manual_code}
 *
 * To regenerate: make generate-pairing
 */

#pragma once

/* Commissioning Parameters */
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR 0x{args.discriminator:03X}
#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE {args.passcode}

/* SPAKE2+ Parameters */
#define CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT {args.iterations}
#define CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT "{args.salt}"
{verifier_define}

/* Firmware Configuration ID (4-bit, 0-15)
 * Displayed as binary via LED during factory reset:
 * White = 1, Red = 0, LSB first, repeated 5 times
 */
#define FIRMWARE_CONFIG_ID {new_config_id}
'''

    if args.output:
        # Write to file
        with open(args.output, 'w') as f:
            f.write(header_content)
        print(f"Generated: {args.output}")
        print()
        print(f"QR Code:     {qr_code}")
        print(f"Manual Code: {manual_code}")
    else:
        # Print to stdout
        print("=" * 70)
        print("Matter Pairing Configuration")
        print("=" * 70)
        print()
        print(f"Vendor ID:      0x{args.vendor_id:04X}")
        print(f"Product ID:     0x{args.product_id:04X}")
        print(f"Discriminator:  {args.discriminator} (0x{args.discriminator:03X})")
        print(f"Passcode:       {args.passcode}")
        print()
        print(f"QR Code:        {qr_code}")
        print(f"Manual Code:    {manual_code}")
        print()
        print("=" * 70)
        print("Add to CHIPProjectConfig.h:")
        print("=" * 70)
        print()
        print(f"#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR 0x{args.discriminator:03X}")
        print(f"#define CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE {args.passcode}")
        print()
        print(f"#define CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_ITERATION_COUNT {args.iterations}")
        print(f'#define CHIP_DEVICE_CONFIG_USE_TEST_SPAKE2P_SALT "{args.salt}"')
        print(verifier_define)
        print()
        print("=" * 70)
        print("Add to sdkconfig.defaults (for vendor/product ID):")
        print("=" * 70)
        print()
        print(f"CONFIG_DEVICE_VENDOR_ID=0x{args.vendor_id:04X}")
        print(f"CONFIG_DEVICE_PRODUCT_ID=0x{args.product_id:04X}")
        print()

    # Generate QR code image if requested
    if args.qr_image:
        if not HAS_QRCODE:
            print("Error: qrcode library not installed. Run: pip install qrcode pillow")
            sys.exit(1)

        qr = qrcode.QRCode(
            version=1,
            error_correction=qrcode.constants.ERROR_CORRECT_M,
            box_size=10,
            border=4,
        )
        qr.add_data(qr_code)
        qr.make(fit=True)

        qr_img = qr.make_image(fill_color="black", back_color="white")

        # Convert to PIL Image if necessary
        if not isinstance(qr_img, Image.Image):
            qr_img = qr_img.convert('RGB')

        # Format manual code as XXXX-XXX-XXXX
        manual_code_str = str(manual_code)
        if len(manual_code_str) == 11:
            formatted_code = f"{manual_code_str[0:4]}-{manual_code_str[4:7]}-{manual_code_str[7:11]}"
        else:
            formatted_code = manual_code_str

        # Add text below QR code
        qr_width, qr_height = qr_img.size
        text_height = 60  # Height for text area

        # Create new image with extra space for text
        img = Image.new('RGB', (qr_width, qr_height + text_height), 'white')
        img.paste(qr_img, (0, 0))

        # Add manual code text
        draw = ImageDraw.Draw(img)

        # Try to use a nice font, fall back to default if not available
        try:
            font = ImageFont.truetype("/System/Library/Fonts/Helvetica.ttc", 36)
        except:
            try:
                font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 36)
            except:
                font = ImageFont.load_default()

        # Center the text
        bbox = draw.textbbox((0, 0), formatted_code, font=font)
        text_width = bbox[2] - bbox[0]
        text_x = (qr_width - text_width) // 2
        text_y = qr_height + 10

        draw.text((text_x, text_y), formatted_code, fill='black', font=font)

        img.save(args.qr_image)
        print(f"QR Image:    {args.qr_image}")
        print(f"Manual Code: {formatted_code}")


if __name__ == '__main__':
    main()

/*
 * M5NanoC6 Matter Switch - CHIP Project Configuration
 *
 * Device identification and commissioning parameters for Matter.
 *
 * Pairing configuration is in CHIPPairingConfig.h (auto-generated).
 * Run 'make generate-pairing' to generate new random pairing codes.
 *
 * Invalid passcodes (cannot be used):
 *   00000000, 11111111, 22222222, 33333333, 44444444,
 *   55555555, 66666666, 77777777, 88888888, 99999999,
 *   12345678, 87654321
 */

#pragma once

/*
 * Device Information
 * These appear in the Matter fabric and home apps.
 */
#define CHIP_DEVICE_CONFIG_DEVICE_VENDOR_NAME "0x76656E Labs"
#define CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_NAME "M5NanoC6 Switch"

/*
 * Pairing Configuration (auto-generated)
 * Run 'make generate-pairing' to regenerate with new random values.
 */
#include "CHIPPairingConfig.h"

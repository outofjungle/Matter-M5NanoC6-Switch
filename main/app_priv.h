/*
   M5NanoC6 Matter Switch - Private Header

   Hardware Configuration:
   - Button: GPIO 9
   - WS2812 LED Data: GPIO 20
   - WS2812 LED Power Enable: GPIO 19
*/

#pragma once

#include <esp_err.h>
#include <esp_matter.h>

// M5NanoC6 GPIO Configuration
#define M5NANOC6_BUTTON_GPIO        9
#define M5NANOC6_LED_DATA_GPIO      20
#define M5NANOC6_LED_POWER_GPIO     19
#define M5NANOC6_RMT_CHANNEL        0

// LED Color Configuration (GRB order for WS2812)
// Format: LED_COLOR_<STATE>_<CHANNEL> where channel is G, R, or B
#define LED_COLOR_ON_G              0
#define LED_COLOR_ON_R              0
#define LED_COLOR_ON_B              128     // Bright blue

#define LED_COLOR_OFF_G             0
#define LED_COLOR_OFF_R             0
#define LED_COLOR_OFF_B             20      // Dim blue

#define LED_COLOR_IDENTIFY_G        128     // White flash
#define LED_COLOR_IDENTIFY_R        128
#define LED_COLOR_IDENTIFY_B        128

// LED Color Configuration - Factory Reset (red)
#define LED_COLOR_RESET_G           0       // GRB order
#define LED_COLOR_RESET_R_MIN       50      // Red starting intensity
#define LED_COLOR_RESET_R_MAX       255     // Red final intensity
#define LED_COLOR_RESET_B           0

// LED Timing Configuration
#define LED_IDENTIFY_BLINK_MS       500
#define LED_REFRESH_TIMEOUT_MS      100
#define LED_RESET_UPDATE_MS         100     // Reset countdown LED update rate

// Reset blink rate configuration (blink speeds up as progress increases)
#define LED_RESET_BLINK_START_MS    1000    // Initial blink period at 0% progress
#define LED_RESET_BLINK_END_MS      200     // Final blink period at 100% progress

// Firmware Config ID Display Configuration
#define FIRMWARE_CONFIG_ID_BITS             4       // Number of bits to display
#define FIRMWARE_CONFIG_ID_BIT_DELAY_MS     500     // Delay showing each bit
#define FIRMWARE_CONFIG_ID_PATTERN_DELAY_MS 1500    // Delay between pattern repetitions
#define FIRMWARE_CONFIG_ID_REPEAT_COUNT     5       // Number of times to repeat pattern
#define FIRMWARE_CONFIG_ID_START_DELAY_MS   1000    // Delay before binary display starts

// Identify pattern configuration (repeats config ID binary pattern)
#define IDENTIFY_CONFIG_ID_REPEAT_COUNT     2       // Repeat pattern twice for identify

// LED Colors for binary code display
// Binary 1 = White (reuse identify color)
#define LED_COLOR_BIT_1_G   LED_COLOR_IDENTIFY_G
#define LED_COLOR_BIT_1_R   LED_COLOR_IDENTIFY_R
#define LED_COLOR_BIT_1_B   LED_COLOR_IDENTIFY_B

// Binary 0 = Red (reuse reset color)
#define LED_COLOR_BIT_0_G   LED_COLOR_RESET_G
#define LED_COLOR_BIT_0_R   LED_COLOR_RESET_R_MAX
#define LED_COLOR_BIT_0_B   LED_COLOR_RESET_B

// LED Colors for reset result indicators
// Green = reset cancelled (button released)
#define LED_COLOR_CANCEL_R      0
#define LED_COLOR_CANCEL_G      128
#define LED_COLOR_CANCEL_B      0

// Red = reset confirmed (button held) - reuse existing
#define LED_COLOR_CONFIRM_R     LED_COLOR_RESET_R_MAX
#define LED_COLOR_CONFIRM_G     LED_COLOR_RESET_G
#define LED_COLOR_CONFIRM_B     LED_COLOR_RESET_B

// Duration to show result indicator
#define FIRMWARE_CONFIG_ID_RESULT_MS    3000

typedef void *app_driver_handle_t;

/** Initialize the WS2812 LED indicator
 *
 * Enables GPIO 19 power supply and initializes WS2812 on GPIO 20.
 *
 * @return Handle on success, NULL on failure.
 */
app_driver_handle_t app_driver_led_init(void);

/** Initialize the button driver
 *
 * Initializes the button on GPIO 9.
 *
 * @return Handle on success, NULL on failure.
 */
app_driver_handle_t app_driver_button_init(void);

/** Set LED indicator state
 *
 * Updates the WS2812 LED to reflect the on/off state.
 * ON = bright blue, OFF = dim blue
 *
 * @param[in] handle LED driver handle.
 * @param[in] power true = on, false = off.
 *
 * @return ESP_OK on success.
 */
esp_err_t app_driver_led_set_power(app_driver_handle_t handle, bool power);

/** Handle attribute updates from Matter stack
 *
 * Called when OnOff cluster attribute changes.
 *
 * @param[in] driver_handle LED driver handle.
 * @param[in] endpoint_id Endpoint ID.
 * @param[in] cluster_id Cluster ID.
 * @param[in] attribute_id Attribute ID.
 * @param[in] val Attribute value.
 *
 * @return ESP_OK on success.
 */
esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, const esp_matter_attr_val_t *val);

/** Start LED identify pattern
 *
 * Displays firmware config ID as binary pattern to identify the device.
 *
 * @return ESP_OK on success.
 */
esp_err_t app_driver_led_identify_start(void);

/** Stop LED identify pattern
 *
 * Stops the pattern and restores normal LED state.
 *
 * @param[in] current_power Current on/off state to restore.
 *
 * @return ESP_OK on success.
 */
esp_err_t app_driver_led_identify_stop(bool current_power);

/** Get LED strip handle for direct access
 *
 * Used by app_reset for LED control during factory reset countdown.
 * IMPORTANT: Caller must use app_driver_led_lock/unlock for thread safety.
 *
 * @return LED strip pointer, or NULL if not initialized.
 */
struct led_strip_s *app_driver_get_led_strip(void);

/** Lock LED strip for exclusive access
 *
 * Must be called before directly accessing the LED strip via app_driver_get_led_strip().
 * Uses a 50ms timeout to avoid deadlock.
 *
 * @return true if lock acquired, false on timeout.
 */
bool app_driver_led_lock(void);

/** Unlock LED strip after exclusive access
 *
 * Must be called after LED operations are complete.
 */
void app_driver_led_unlock(void);

/** Display firmware config ID as binary pattern on LED
 *
 * Displays 4-bit config ID as white (1) and red (0) LEDs, MSB first.
 * Blocks until pattern completes.
 *
 * @param[in] repeat_count Number of times to repeat the pattern.
 */
void app_driver_display_config_id_pattern(int repeat_count);

/** Get current on/off power state
 *
 * Reads the current OnOff attribute value from Matter data model.
 *
 * @return true if on, false if off.
 */
#ifdef __cplusplus
extern "C"
#endif
bool app_get_current_power_state(void);

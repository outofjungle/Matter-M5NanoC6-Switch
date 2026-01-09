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

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include "esp_openthread_types.h"
#endif

// M5NanoC6 GPIO Configuration
#define M5NANOC6_BUTTON_GPIO        9
#define M5NANOC6_LED_DATA_GPIO      20
#define M5NANOC6_LED_POWER_GPIO     19
#define M5NANOC6_RMT_CHANNEL        0

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
 * ON = Green, OFF = Red (dim)
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
                                      uint32_t attribute_id, esp_matter_attr_val_t *val);

/** Start LED identify blink pattern
 *
 * Blinks the LED to identify the device during commissioning.
 *
 * @return ESP_OK on success.
 */
esp_err_t app_driver_led_identify_start(void);

/** Stop LED identify blink pattern
 *
 * Stops the blink and restores normal LED state.
 *
 * @param[in] current_power Current on/off state to restore.
 *
 * @return ESP_OK on success.
 */
esp_err_t app_driver_led_identify_stop(bool current_power);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#define ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG()                                           \
    {                                                                                   \
        .radio_mode = RADIO_MODE_NATIVE,                                                \
    }

#define ESP_OPENTHREAD_DEFAULT_HOST_CONFIG()                                            \
    {                                                                                   \
        .host_connection_mode = HOST_CONNECTION_MODE_NONE,                              \
    }

#define ESP_OPENTHREAD_DEFAULT_PORT_CONFIG()                                            \
    {                                                                                   \
        .storage_partition_name = "nvs", .netif_queue_size = 10, .task_queue_size = 10, \
    }
#endif

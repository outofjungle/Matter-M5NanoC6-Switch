/*
   M5NanoC6 Matter Switch - Factory Reset Handler Header

   Factory reset by holding button for 20 seconds while device is running.
*/

#pragma once

#include <esp_err.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register factory reset button callbacks
 *
 * Registers long press (20s hold) to trigger factory reset with
 * LED countdown indication.
 *
 * @param handle Button handle from app_driver_button_init()
 * @return ESP_OK on success
 */
esp_err_t app_reset_button_register(void *handle);

#ifdef __cplusplus
}
#endif

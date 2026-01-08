/*
   M5NanoC6 Matter Switch - Factory Reset Handler Header
*/

#pragma once

#include <esp_err.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register factory reset button callbacks
 *
 * Registers long press (hold) to trigger factory reset flag,
 * and press up to execute factory reset.
 *
 * @param handle Button handle from app_driver_button_init()
 * @return ESP_OK on success
 */
esp_err_t app_reset_button_register(void *handle);

#ifdef __cplusplus
}
#endif

/*
   M5NanoC6 Matter Switch - Factory Reset Handler

   Implements 20 second button hold for factory reset with LED countdown.
   Runtime only - hold button for 20 seconds while device is running.
*/

#include <atomic>
#include <esp_log.h>
#include <esp_matter.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include <iot_button.h>
#include <led_strip.h>

#include "app_priv.h"

static const char *TAG = "app_reset";

// Factory reset timing
#define FACTORY_RESET_HOLD_TIME_MS  20000  // 20 seconds

// Runtime reset state
static TimerHandle_t s_reset_timer = NULL;
static std::atomic<uint32_t> s_reset_start_time{0};
static std::atomic<bool> s_reset_in_progress{false};

// Set LED color for reset countdown (red with increasing intensity)
static void set_reset_led(uint32_t elapsed_ms)
{
    led_strip_t *strip = app_driver_get_led_strip();
    if (!strip) return;

    // Calculate progress (0-100%)
    uint32_t progress = (elapsed_ms * 100) / FACTORY_RESET_HOLD_TIME_MS;
    if (progress > 100) progress = 100;

    // Blink rate increases with progress
    uint32_t blink_period = 1000 - (progress * 8);  // 1000ms -> 200ms
    bool led_on = ((elapsed_ms / (blink_period / 2)) % 2) == 0;

    if (led_on) {
        // Red intensity increases with progress (50 -> 255)
        uint8_t intensity = LED_COLOR_RESET_R_MIN + (progress * 2);
        if (intensity > LED_COLOR_RESET_R_MAX) intensity = LED_COLOR_RESET_R_MAX;
        strip->set_pixel(strip, 0, LED_COLOR_RESET_G, intensity, LED_COLOR_RESET_B);
    } else {
        strip->set_pixel(strip, 0, 0, 0, 0);  // Off
    }
    strip->refresh(strip, LED_REFRESH_TIMEOUT_MS);
}

// Flash LED to indicate reset is happening
static void flash_reset_led(void)
{
    led_strip_t *strip = app_driver_get_led_strip();
    if (!strip) return;

    for (int i = 0; i < 5; i++) {
        strip->set_pixel(strip, 0, LED_COLOR_RESET_G, LED_COLOR_RESET_R_MAX, LED_COLOR_RESET_B);
        strip->refresh(strip, LED_REFRESH_TIMEOUT_MS);
        vTaskDelay(pdMS_TO_TICKS(LED_RESET_UPDATE_MS));
        strip->set_pixel(strip, 0, 0, 0, 0);
        strip->refresh(strip, LED_REFRESH_TIMEOUT_MS);
        vTaskDelay(pdMS_TO_TICKS(LED_RESET_UPDATE_MS));
    }
}

// Timer callback for reset countdown
static void reset_timer_cb(TimerHandle_t timer)
{
    if (!s_reset_in_progress) return;

    uint32_t elapsed_ms = (xTaskGetTickCount() * portTICK_PERIOD_MS) - s_reset_start_time;

    // Update LED
    set_reset_led(elapsed_ms);

    // Check if countdown complete
    if (elapsed_ms >= FACTORY_RESET_HOLD_TIME_MS) {
        xTimerStop(s_reset_timer, 0);
        s_reset_in_progress = false;
        ESP_LOGW(TAG, "Factory reset triggered!");
        flash_reset_led();
        esp_matter::factory_reset();
    }
}

static void button_long_press_start_cb(void *arg, void *data)
{
    if (s_reset_in_progress) return;

    ESP_LOGW(TAG, "Hold button for %u seconds to factory reset",
             (unsigned)(FACTORY_RESET_HOLD_TIME_MS / 1000));

    s_reset_in_progress = true;
    s_reset_start_time = xTaskGetTickCount() * portTICK_PERIOD_MS;

    // Start timer for LED updates (timer pre-created in app_reset_button_register)
    if (s_reset_timer) {
        xTimerStart(s_reset_timer, 0);
    }
}

static void button_released_cb(void *arg, void *data)
{
    if (!s_reset_in_progress) return;

    // Cancel reset
    s_reset_in_progress = false;
    if (s_reset_timer) {
        xTimerStop(s_reset_timer, 0);
    }

    uint32_t elapsed_ms = (xTaskGetTickCount() * portTICK_PERIOD_MS) - s_reset_start_time;
    ESP_LOGI(TAG, "Factory reset cancelled after %u ms", (unsigned)elapsed_ms);

    // Restore LED state (will be updated by attribute callback)
    led_strip_t *strip = app_driver_get_led_strip();
    if (strip) {
        strip->set_pixel(strip, 0, LED_COLOR_OFF_G, LED_COLOR_OFF_R, LED_COLOR_OFF_B);
        strip->refresh(strip, LED_REFRESH_TIMEOUT_MS);
    }
}

extern "C" esp_err_t app_reset_button_register(void *handle)
{
    if (!handle) {
        ESP_LOGE(TAG, "Handle cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }

    button_handle_t button_handle = (button_handle_t)handle;
    esp_err_t err;

    // Pre-create reset timer to avoid allocation during operation
    s_reset_timer = xTimerCreate("reset", pdMS_TO_TICKS(LED_RESET_UPDATE_MS), pdTRUE, NULL, reset_timer_cb);
    if (!s_reset_timer) {
        ESP_LOGE(TAG, "Failed to create reset timer");
        return ESP_ERR_NO_MEM;
    }

    // Long press start triggers countdown
    err = iot_button_register_cb(button_handle, BUTTON_LONG_PRESS_START,
                                  button_long_press_start_cb, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register long press callback: %d", err);
        return err;
    }

    // Release cancels if not complete
    err = iot_button_register_cb(button_handle, BUTTON_PRESS_UP,
                                  button_released_cb, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register press up callback: %d", err);
        return err;
    }

    ESP_LOGI(TAG, "Factory reset handler registered (hold %us)",
             (unsigned)(FACTORY_RESET_HOLD_TIME_MS / 1000));
    return ESP_OK;
}

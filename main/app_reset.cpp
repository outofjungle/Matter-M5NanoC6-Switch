/*
   M5NanoC6 Matter Switch - Factory Reset Handler

   Implements 20 second button hold for factory reset with LED countdown.
   Runtime only - hold button for 20 seconds while device is running.
*/

#include <atomic>
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_matter.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include <iot_button.h>
#include <led_strip.h>

#include "app_priv.h"
#include "include/CHIPPairingConfig.h"

static const char *TAG = "app_reset";

// Reset state machine
enum class ResetState : uint8_t {
    IDLE,
    COUNTDOWN,      // Displaying binary code and performing reset
};

static TimerHandle_t s_reset_timer = NULL;
static std::atomic<ResetState> s_reset_state{ResetState::IDLE};

// Display a single bit via LED color (MSB first order)
static void display_bit(bool bit_value)
{
    if (!app_driver_led_lock()) return;
    led_strip_t *strip = app_driver_get_led_strip();
    if (strip) {
        if (bit_value) {
            // Binary 1 = White (RGB order)
            strip->set_pixel(strip, 0, LED_COLOR_BIT_1_R, LED_COLOR_BIT_1_G, LED_COLOR_BIT_1_B);
        } else {
            // Binary 0 = Red (RGB order)
            strip->set_pixel(strip, 0, LED_COLOR_BIT_0_R, LED_COLOR_BIT_0_G, LED_COLOR_BIT_0_B);
        }
        strip->refresh(strip, LED_REFRESH_TIMEOUT_MS);
    }
    app_driver_led_unlock();
}

// Turn LED off between bits
static void led_off(void)
{
    if (!app_driver_led_lock()) return;
    led_strip_t *strip = app_driver_get_led_strip();
    if (strip) {
        strip->set_pixel(strip, 0, 0, 0, 0);
        strip->refresh(strip, LED_REFRESH_TIMEOUT_MS);
    }
    app_driver_led_unlock();
}

// Check if button is currently pressed (GPIO low = pressed, active low with pull-up)
static bool is_button_pressed(void)
{
    return gpio_get_level(static_cast<gpio_num_t>(M5NANOC6_BUTTON_GPIO)) == 0;
}

// Show result indicator (green = cancelled, red = confirmed)
static void show_result(bool will_reset)
{
    if (!app_driver_led_lock()) return;
    led_strip_t *strip = app_driver_get_led_strip();
    if (strip) {
        if (will_reset) {
            // Red = confirming reset
            strip->set_pixel(strip, 0, LED_COLOR_CONFIRM_R, LED_COLOR_CONFIRM_G, LED_COLOR_CONFIRM_B);
        } else {
            // Green = cancelled
            strip->set_pixel(strip, 0, LED_COLOR_CANCEL_R, LED_COLOR_CANCEL_G, LED_COLOR_CANCEL_B);
        }
        strip->refresh(strip, LED_REFRESH_TIMEOUT_MS);
    }
    app_driver_led_unlock();
}

// Delay that can be cancelled by button release
// Returns true if delay completed, false if cancelled
static bool cancellable_delay(uint32_t delay_ms, uint32_t check_interval_ms = 50)
{
    uint32_t elapsed = 0;
    while (elapsed < delay_ms) {
        // Check if button was released (state changed from COUNTDOWN)
        if (s_reset_state.load() != ResetState::COUNTDOWN) {
            return false;  // Cancelled
        }
        uint32_t wait = (delay_ms - elapsed < check_interval_ms)
                       ? (delay_ms - elapsed) : check_interval_ms;
        vTaskDelay(pdMS_TO_TICKS(wait));
        elapsed += wait;
    }
    return true;  // Completed
}

// Display firmware config ID as 4-bit binary code (MSB first)
// Non-cancellable - always completes the full pattern
static void display_firmware_config_id(void)
{
    uint8_t config_id = FIRMWARE_CONFIG_ID & 0x0F;

    ESP_LOGI(TAG, "Displaying firmware config ID: %d (0b%d%d%d%d, MSB first)",
             config_id,
             (config_id >> 3) & 1, (config_id >> 2) & 1,
             (config_id >> 1) & 1, config_id & 1);

    for (int repeat = 0; repeat < FIRMWARE_CONFIG_ID_REPEAT_COUNT; repeat++) {
        // Display 4 bits, MSB first (bit 3, 2, 1, 0)
        for (int bit = FIRMWARE_CONFIG_ID_BITS - 1; bit >= 0; bit--) {
            bool bit_value = (config_id >> bit) & 1;
            display_bit(bit_value);

            vTaskDelay(pdMS_TO_TICKS(FIRMWARE_CONFIG_ID_BIT_DELAY_MS));

            // Brief off between bits (except after last bit of pattern)
            if (bit > 0) {
                led_off();
                vTaskDelay(pdMS_TO_TICKS(50));
            }
        }

        // Delay between patterns (except after last)
        if (repeat < FIRMWARE_CONFIG_ID_REPEAT_COUNT - 1) {
            led_off();
            vTaskDelay(pdMS_TO_TICKS(FIRMWARE_CONFIG_ID_PATTERN_DELAY_MS));
        }
    }

    ESP_LOGI(TAG, "Firmware config ID display complete");
}

// Timer callback - no longer used, kept for compatibility
static void reset_timer_cb(TimerHandle_t timer)
{
    // No-op - reset is now handled directly in button callback
}

static void button_long_press_start_cb(void *arg, void *data)
{
    // Only start if idle (not already in countdown)
    ResetState expected = ResetState::IDLE;
    if (!s_reset_state.compare_exchange_strong(expected, ResetState::COUNTDOWN)) {
        return;
    }

    // Save current power state before starting reset sequence
    bool saved_power_state = app_get_current_power_state();

    ESP_LOGW(TAG, "Factory reset sequence starting in 3 seconds...");

    // Initial delay (3 seconds) - user can still release to cancel
    if (!cancellable_delay(FIRMWARE_CONFIG_ID_START_DELAY_MS)) {
        ESP_LOGI(TAG, "Factory reset cancelled during initial delay");
        app_driver_led_set_power(NULL, saved_power_state);
        s_reset_state = ResetState::IDLE;
        return;
    }

    ESP_LOGW(TAG, "Displaying config ID...");

    // Display binary code sequence (non-cancellable - user can see pairing info)
    display_firmware_config_id();

    // Check if button is still held by reading GPIO directly
    // (callback-based state won't work since we're blocking in the same task)
    bool button_still_held = is_button_pressed();

    if (button_still_held) {
        ESP_LOGW(TAG, "Button held - reset will proceed in 3 seconds");
        show_result(true);  // Red
        vTaskDelay(pdMS_TO_TICKS(FIRMWARE_CONFIG_ID_RESULT_MS));

        ESP_LOGW(TAG, "Performing factory reset");
        s_reset_state = ResetState::IDLE;
        esp_matter::factory_reset();
    } else {
        ESP_LOGI(TAG, "Button released - reset cancelled");
        show_result(false);  // Green
        vTaskDelay(pdMS_TO_TICKS(FIRMWARE_CONFIG_ID_RESULT_MS));

        // Restore LED to previous power state
        app_driver_led_set_power(NULL, saved_power_state);
        s_reset_state = ResetState::IDLE;
    }
}

static void button_released_cb(void *arg, void *data)
{
    // Signal cancellation by changing state from COUNTDOWN to IDLE.
    // The main callback (button_long_press_start_cb) handles LED restoration.
    ResetState expected = ResetState::COUNTDOWN;
    if (!s_reset_state.compare_exchange_strong(expected, ResetState::IDLE)) {
        return;
    }

    ESP_LOGI(TAG, "Factory reset cancelled (button released)");
}

extern "C" esp_err_t app_reset_button_register(void *handle)
{
    if (!handle) {
        ESP_LOGE(TAG, "Handle cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }

    auto button_handle = static_cast<button_handle_t>(handle);
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
        xTimerDelete(s_reset_timer, 0);
        s_reset_timer = NULL;
        return err;
    }

    // Release cancels if not complete
    err = iot_button_register_cb(button_handle, BUTTON_PRESS_UP,
                                  button_released_cb, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register press up callback: %d", err);
        xTimerDelete(s_reset_timer, 0);
        s_reset_timer = NULL;
        return err;
    }

    ESP_LOGI(TAG, "Factory reset handler registered (long press displays config ID then resets)");
    return ESP_OK;
}
